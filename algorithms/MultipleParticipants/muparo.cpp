#include <boost/foreach.hpp> 

#include "muparo.h"
#include "utils.h"

namespace MuPaRo
{

bool PropagationRule::applicable(int node) const
{
    if(filter != NULL) {
        if( !filter->isIn( node ) )
            return false;
    }
    
    BOOST_FOREACH( int layer, conditions ) {
        if( !mup->is_node_set( StateFreeNode(layer, node) ) ) {
            return false;
        }
    }
    return true;
}

void PropagationRule::apply(int node) 
{
    if( applicable(node) ) {
        int arr = arrival_in_insertion_layer( node );
        int cost = cost_in_insertion_layer( node );

        if( mup->insert( StateFreeNode(insertion, node), arr, cost ) ) {
            mup->clear_pred_layers( StateFreeNode(insertion, node) );
            BOOST_FOREACH( int cond_layer, conditions ) 
                mup->add_pred_layer( StateFreeNode(insertion, node), cond_layer);
        }
    }   
}

int PropagationRule::arrival_in_insertion_layer ( const int node ) const
{
    if(arr_comb == MaxArrival) {
        int arr = -99999999;
        BOOST_FOREACH( int layer, conditions ) {
            if( mup->arrival( StateFreeNode(layer, node) ) > arr )
                arr = mup->arrival( StateFreeNode(layer, node) );
        }
        return arr;
    } else if(arr_comb == FirstLayerArrival) {
        return mup->arrival( StateFreeNode(conditions[0], node) );
    } else if(arr_comb == SecondLayerArrival) {
        return mup->arrival( StateFreeNode(conditions[1], node) );
    }
    
    throw new Invalid_Operation(); // This should never be reached;
}

int PropagationRule::cost_in_insertion_layer ( const int node ) const
{
    int cost = 0;
    if(cost_comb == SumCost || cost_comb == MaxCost) {
        BOOST_FOREACH( int layer, conditions ) {
            cost = combine_costs(cost, mup->get_cost( StateFreeNode(layer, node) ));
        }
    } else if(cost_comb == SumPlusWaitCost) {
        const int last_arr = arrival_in_insertion_layer( node );
        int max_wait = 0;
        
        BOOST_FOREACH( int layer, conditions ) {
            cost += mup->get_cost( StateFreeNode(layer, node) );
            if(last_arr - mup->arrival( StateFreeNode(layer, node) ) > max_wait)
                max_wait = last_arr - mup->arrival( StateFreeNode(layer, node) );
        }
        
        cost += max_wait;
    } else {
        throw Invalid_Operation();
    }
    return cost;
}


bool ConnectionRule::applicable ( const int node ) const
{
    BOOST_FOREACH( int layer, conditions ) {
        if( !mup->is_node_set( StateFreeNode(layer, node) ) ) {
            return false;
        }
    }
    return true;
}

int ConnectionRule::get_cost( const int node, bool real_cost ) const {
        int c2 = mup->get_cost( StateFreeNode(2, node) );
        
        int c4 = mup->get_cost( StateFreeNode(4, node) );
        
        int c3;
        if(!real_cost)
            c3 = mup->get_cost( StateFreeNode(3, node) );
        else {
            RLC::BackwardGraph * g = dynamic_cast<RLC::BackwardGraph*>(mup->graphs[3]);
            RLC::Dijkstra dij(g->forward_graph, node, l3_dest, mup->arrival( StateFreeNode(2, node) ), 10);
            dij.run();
            if(dij.path_found) {
                c3 = dij.path_cost;                
                BOOST_ASSERT( mup->get_cost( StateFreeNode(3, node) ) <= c3 );
            } else {
                c3 = 99999999;
            }
            
        }
        
        if(c3 > c4)
            return c2 + c3;
        else
            return c2 + c4;
    }



void ConnectionRule::apply ( const int node )
{
    if( applicable( node ) ) {
        
        // Make sure this path can be interesting before going any further
        if(mup->connection_found && get_cost( node, false) >= mup->best_cost)
            return;
        
        int total_cost = get_cost( node, true );

        if(!mup->connection_found || total_cost < mup->best_cost) {
            mup->connection_found = true;
            mup->best_cost = total_cost;
            mup->best_connection_node = node;
        }
    }
}
    

    
Muparo::Muparo(Transport::Graph * trans, int num_layers, MuparoParameters params) :
params(params),
num_layers(num_layers),
transport(trans),
connection_found(false),
vres(transport)
{
    is_set = new boost::dynamic_bitset<>*[num_layers];
    flags = new Flag* [num_layers];
    for(int i=0; i<num_layers ; ++i) {
        is_set[i] = new boost::dynamic_bitset<>(boost::num_vertices(transport->g));
        flags[i] = (Flag*) malloc( sizeof( Flag ) * boost::num_vertices(transport->g) );
    }
}

Muparo::~Muparo()
{
    for(int i=0; i<num_layers ; ++i)
    {
        delete dij[i];
        delete graphs[i];
        delete is_set[i];
        delete[] flags[i];
    }
    delete is_set;
    delete[] flags;
    
    for(uint i=0 ; i<propagation_rules.size() ; ++i) {
        delete propagation_rules[i];
    }
}


bool Muparo::run()
{
    double start_time = get_run_time_sec();
    BOOST_FOREACH(StartNode sn, start_nodes) {
        insert(sn.first, sn.second, 0);
    }
    
    while(!finished()) {
        const int layer = select_layer();
        
        RLC::Vertice vert = dij[layer]->treat_next();
        StateFreeNode node(layer, vert.first);
        
        if(params.search_type == Bidirectional && (layer == params.bidir_layers.first || layer == params.bidir_layers.second)) {
            check_connections( layer );
        }
        
        if( graphs[layer]->is_accepting( vert ) && !is_node_set( node ) ) {
            set( CompleteNode(layer, vert) );
            apply_rules( node.second );
        }
    }
    
    double runtime = get_run_time_sec() - start_time;
    cout << "Muparo finished in "<< runtime << " seconds"<<endl;
    
    build_result();
    
    return true;
}

bool Muparo::finished() const
{
    bool heap_empties = true;
    bool goals_reached = true;
    
    BOOST_FOREACH(RLC::Dijkstra *d, dij) {
        if(! d->heap.empty() ) {
            heap_empties = false;
            break;
        }
    }
    
    if(params.search_type == DestNodes) {
        goals_reached = true;
        BOOST_FOREACH(StateFreeNode n, goal_nodes) {
            if(!is_node_set(n)) {
                goals_reached = false;
                break;
            }
        }
    } else if(params.search_type == Bidirectional) {
        goals_reached = false;
        if(connection_found) {
            if(best_cost <= min_cost(params.bidir_layers.first) + min_cost(params.bidir_layers.second))
                goals_reached = true;
        }
    } else if(params.search_type == Connection) {
        goals_reached = false;
        if(connection_found) {
            if( best_cost <= min_cost(2) 
                && best_cost <= min_cost(3)
                && best_cost <= min_cost(4) )
                goals_reached = true;
        }
    }
    
    return heap_empties || goals_reached;
}


int Muparo::select_layer() const
{
    int best_cost = 999999999;
    int best_layer = -1;
    
    for(int i=0 ; i<num_layers ; ++i) {
        if( !dij[i]->heap.empty() && ( dij[i]->cost( dij[i]->heap.top() ) < best_cost ) ) {
            best_cost = dij[i]->cost( dij[i]->heap.top() );
            best_layer = i;
        }
    }
    
    return best_layer;
}


void Muparo::apply_rules ( const int node )
{
    BOOST_FOREACH( PropagationRule * rule, propagation_rules )
        rule->apply( node );
    
    BOOST_FOREACH( ConnectionRule rule, connection_rules )
        rule.apply( node );
}

bool Muparo::insert ( const StateFreeNode & n, const int arrival, const int cost )
{
    bool inserted = false;
    int layer = n.first;
    int node = n.second;
    BOOST_FOREACH(int dfa_start, graphs[layer]->dfa_start_states())  
    {
        RLC::Vertice rlc_node;
        rlc_node.first = node;
        rlc_node.second = dfa_start;
        
        if( dij[layer]->insert_node( rlc_node, arrival, cost ) )
            inserted = true;
    }
    
    return inserted;
}

void Muparo::build_result()
{
    std::list< CompleteNode > queue;
    
    if(params.search_type == Bidirectional) {
        queue.push_back(CompleteNode(params.bidir_layers.first, best_bidir_connection));
        queue.push_back(CompleteNode(params.bidir_layers.second, best_bidir_connection));
        vres.c_nodes.push_back(best_bidir_connection.first);
    } else if(params.search_type == DestNodes) {
        BOOST_FOREACH(StateFreeNode n, goal_nodes) {
            queue.push_back( CompleteNode(n.first, RLC::Vertice(n.second, flags[n.first][n.second].dfa_state )));
//             vres.a_nodes.push_back(n.second);
        }
    } else { // ConnectionRule, not really used right now
        RLC::Vertice v;
        v.first = best_connection_node;
        v.second = flags[2][best_connection_node].dfa_state;
        queue.push_back(CompleteNode(2, v));
        v.first = best_connection_node;
        v.second = flags[4][best_connection_node].dfa_state;
        queue.push_back(CompleteNode(4, v));
        vres.c_nodes.push_back(best_connection_node);
        
        int node = v.first;
        RLC::BackwardGraph * g = dynamic_cast<RLC::BackwardGraph*>(graphs[3]);
        RLC::Dijkstra dij(g->forward_graph, node, connection_rules[0].l3_dest, arrival( StateFreeNode(2, node) ), 10);
        dij.run();
        BOOST_FOREACH( int edge, dij.get_transport_path() ) {
            vres.edges.push_back( edge );
        }
    }
    
    while( !queue.empty() ) {
        CompleteNode curr = queue.back();
        queue.pop_back();
        
        int l = curr.first;
        RLC::Vertice vert = curr.second;
        
        if( dij[l]->has_pred(vert) ) {
            vres.edges.push_back(transport->edgeIndex( dij[l]->get_pred(vert).first ));
            queue.push_back( CompleteNode(l, graphs[l]->source(dij[l]->get_pred(vert) )));
        }
        else if( flags[l][vert.first].pred_layers == 0 ) // no pred layers  
        {
        }
        else
        {
            for(uint layer=0 ; layer < sizeof(Flag::pred_layers)*8 ; ++layer) {
                if(flags[l][vert.first].pred_layers & (1 << layer)) {
                    queue.push_back( CompleteNode(layer, RLC::Vertice(vert.first, flags[layer][vert.first].dfa_state )));
                    vres.c_nodes.push_back(vert.first);
                }
            }
        }
    }
}

VisualResult Muparo::get_result() const
{
    return vres;
}


int Muparo::visited_nodes() const
{
    int total = 0;
    BOOST_FOREACH( RLC::Dijkstra * d, dij ) {
        total += d->visited_nodes;
    }
    return total;
}

void Muparo::check_connections ( const int modified_layer )
{
    BOOST_ASSERT(params.search_type == Bidirectional);
    int l;
    if(modified_layer == params.bidir_layers.first)
        l = params.bidir_layers.second;
    else
        l = params.bidir_layers.first;
    
    while(!dij[modified_layer]->touched_nodes.empty()) {
        const RLC::Vertice v = dij[modified_layer]->touched_nodes.front();
        dij[modified_layer]->touched_nodes.pop_front();
        
        if(!dij[l]->white(v)) {
            int cost = dij[l]->cost(v) + dij[modified_layer]->cost(v);
            if(!connection_found || cost < best_cost) {
                connection_found = true;
                best_cost = cost;
                best_bidir_connection = v;
            }
        }
    }
}

int Muparo::min_cost ( const int layer ) const
{
    int min = 99999999;
    if( ! dij[layer]->heap.empty() ) {
        min = dij[layer]->cost( dij[layer]->heap.top() );
    }
    
    BOOST_FOREACH(PropagationRule * rule, propagation_rules) {
        if( !(layer == rule->insertion) )
            continue;
        
        int tmp_cost = 0;
        BOOST_FOREACH(int l_cond, rule->conditions) {
            //TODO : false, should be the min of all layers
            tmp_cost = rule->combine_costs( tmp_cost, min_cost(l_cond) );
        }
        
        if(tmp_cost < min)
            min = tmp_cost;
    }
    
    return min;
}

void free(Muparo * mup)
{
    delete mup;
}

}