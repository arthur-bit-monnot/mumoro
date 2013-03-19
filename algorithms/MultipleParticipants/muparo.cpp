#include <sys/times.h>

#include "muparo.h"

double get_run_time_sec() {

  struct tms usage;
  static int clock_ticks = sysconf(_SC_CLK_TCK);
  times(&usage);
  double df=((double)usage.tms_utime+(double)usage.tms_stime)/clock_ticks;

  return df;
}

namespace MuPaRo
{

bool PropagationRule::applicable(int node) const
{

    BOOST_FOREACH( int layer, conditions ) {
        if( !mup->is_set( StateFreeNode(layer, node) ) ) {
            return false;
        }
    }
    return true;
}

void PropagationRule::apply(int node) 
{
    if( applicable(node) ) {
        int arr = -99999999;
        int cost = 0;
        BOOST_FOREACH( int layer, conditions ) {
            cost = combine_costs(cost, mup->get_cost( StateFreeNode(layer, node) ));
            if( mup->arrival( StateFreeNode(layer, node) ) > arr ) {
                arr = mup->arrival( StateFreeNode(layer, node) );
            }
        }

        if( mup->insert( StateFreeNode(insertion, node), arr, cost ) ) {
            mup->clear_pred_layers( StateFreeNode(insertion, node) );
            BOOST_FOREACH( int cond_layer, conditions ) 
                mup->add_pred_layer( StateFreeNode(insertion, node), cond_layer);
        }
    }   
}
    
Muparo * point_to_point(Transport::Graph * trans, int source, int dest)
{
    Muparo * mup = new Muparo(trans, 1);
    int day = 10;
    
    mup->dfas.push_back(RLC::bike_pt_dfa());
    
    for(int i=0; i<mup->num_layers ; ++i)
    {
        mup->graphs.push_back( new RLC::Graph(mup->transport, mup->dfas[i] ));
        mup->dij.push_back( new RLC::Dijkstra(mup->graphs[i], -1, -1, -1, day) );
    }
    
    mup->start_nodes.push_back( StartNode( StateFreeNode(0, source), 50000) );
    mup->goal_nodes.push_back( StateFreeNode(0, dest) );
    
    return mup;
}

Muparo * bi_point_to_point(Transport::Graph * trans, int source, int dest)
{
    MuparoParameters mup_params;
    mup_params.bidirectional = true;
    mup_params.bidir_layers = make_pair<int, int>(0, 1);
    
    Muparo * mup = new Muparo(trans, 2, mup_params);
    int day = 10;
    
    mup->dfas.push_back(RLC::foot_dfa());
    mup->dfas.push_back(RLC::foot_dfa());
    
    RLC::DijkstraParameters param;
    param.save_touched_nodes = true;
    RLC::Graph *g = new RLC::Graph(mup->transport, mup->dfas[0] );
    mup->graphs.push_back( g );
    mup->graphs.push_back( new RLC::BackwardGraph(g));
    mup->dij.push_back( new RLC::Dijkstra(mup->graphs[0], -1, -1, -1, day, param) );
    mup->dij.push_back( new RLC::Dijkstra(mup->graphs[1], -1, -1, -1, day, param) );
    
    mup->start_nodes.push_back( StartNode( StateFreeNode(0, source), 50000) );
    mup->start_nodes.push_back( StartNode( StateFreeNode(1, dest), 0) );
    
    return mup;
}

Muparo * covoiturage(Transport::Graph * trans, int source1, int source2, int dest1, int dest2, RLC::DFA dfa_pass, RLC::DFA dfa_car, int limit)
{
    MuparoParameters mup_params;
    mup_params.bidirectional = true;
    mup_params.bidir_layers = make_pair<int, int>(2, 5);
    
    Muparo * mup = new Muparo(trans, 6, mup_params);
    int day = 10;
    
    mup->dfas.push_back(dfa_pass);
    mup->dfas.push_back(dfa_car);
    mup->dfas.push_back(dfa_car);
    mup->dfas.push_back(dfa_pass);
    mup->dfas.push_back(dfa_car);
    mup->dfas.push_back(dfa_car);
    
    RLC::DijkstraParameters param_car;
    
    RLC::DijkstraParameters param_car_bi;
    param_car_bi.save_touched_nodes = true;
    
    RLC::DijkstraParameters param_passenger;
    param_passenger.cost_limit = limit > 0;
    param_passenger.cost_limit_value = limit;
    
    
    RLC::Graph *g1 = new RLC::Graph(mup->transport, mup->dfas[0] );
    RLC::Graph *g2 = new RLC::Graph(mup->transport, mup->dfas[1] );
    RLC::Graph *g3 = new RLC::Graph(mup->transport, mup->dfas[2] );
    mup->graphs.push_back( g1 );
    mup->graphs.push_back( g2 );
    mup->graphs.push_back( g3 );
    mup->graphs.push_back( new RLC::BackwardGraph(g1));
    mup->graphs.push_back( new RLC::BackwardGraph(g2));
    mup->graphs.push_back( new RLC::BackwardGraph(g3));
    mup->dij.push_back( new RLC::Dijkstra(mup->graphs[0], -1, -1, -1, day, param_passenger) );
    mup->dij.push_back( new RLC::Dijkstra(mup->graphs[1], -1, -1, -1, day, param_car) );
    mup->dij.push_back( new RLC::Dijkstra(mup->graphs[2], -1, -1, -1, day, param_car_bi) );
    mup->dij.push_back( new RLC::Dijkstra(mup->graphs[3], -1, -1, -1, day, param_passenger) );
    mup->dij.push_back( new RLC::Dijkstra(mup->graphs[4], -1, -1, -1, day, param_car) );
    mup->dij.push_back( new RLC::Dijkstra(mup->graphs[5], -1, -1, -1, day, param_car_bi) );
    
    mup->start_nodes.push_back( StartNode( StateFreeNode(0, source1), 50000) );
    mup->start_nodes.push_back( StartNode( StateFreeNode(1, source2), 0) );
    mup->start_nodes.push_back( StartNode( StateFreeNode(3, dest1), 0) );
    mup->start_nodes.push_back( StartNode( StateFreeNode(4, dest2), 0) );
    
    PropagationRule cr(mup);
    cr.conditions.push_back(0);
    cr.conditions.push_back(1);
    cr.insertion = 2;
    PropagationRule cr2(mup);
    cr2.conditions.push_back(3);
    cr2.conditions.push_back(4);
    cr2.insertion = 5;
    mup->propagation_rules.push_back(cr);
    mup->propagation_rules.push_back(cr2);
    
    return mup;
}
    
Muparo::Muparo(Transport::Graph * trans, int num_layers, MuparoParameters params) :
params(params),
num_layers(num_layers),
transport(trans),
connection_found(false),
vres(transport)
{
    flags = new Flag* [num_layers];
    for(int i=0; i<num_layers ; ++i) {
        flags[i] = new Flag[boost::num_vertices(transport->g)];
    }
}
    
Muparo::Muparo(Transport::Graph * trans, int start1, int start2, MuparoParameters params) :
params(params),
transport(trans),
connection_found(false),
vres(transport)
{
    num_layers = 3;
    int day = 10;
    
    dfas.push_back(RLC::foot_dfa());
    dfas.push_back(RLC::bike_pt_dfa());
    dfas.push_back(RLC::bike_pt_dfa());
    
    for(int i=0; i<num_layers ; ++i)
    {
        graphs.push_back( new RLC::Graph(this->transport, dfas[i] ));
        dij.push_back( new RLC::Dijkstra(graphs[i], -1, -1, -1, day) );
    }
    
    PropagationRule cr(this);
    cr.conditions.push_back(0);
    cr.conditions.push_back(1);
    cr.insertion = 2;
    propagation_rules.push_back(cr);
    
    start_nodes.push_back( StartNode( StateFreeNode(0, start1), 50000) );
    start_nodes.push_back( StartNode( StateFreeNode(1, start2), 50000) );
    goal_nodes.push_back( StateFreeNode(2, 400) );
    
    flags = new Flag* [num_layers];
    for(int i=0; i<num_layers ; ++i) {
        flags[i] = new Flag[boost::num_vertices(transport->g)];
    }
}

Muparo::~Muparo()
{
    for(int i=0; i<num_layers ; ++i)
    {
        delete dij[i];
        delete graphs[i];
        delete[] flags[i];
    }
    delete[] flags;
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
        
        if(params.bidirectional && (layer == params.bidir_layers.first || layer == params.bidir_layers.second)) {
            check_connections( layer );
        }
        
        if( graphs[layer]->is_accepting( vert ) && !is_set( node ) ) {
            set( CompleteNode(layer, vert) );
            apply_rules( node.second );
        }
    }
    
    double runtime = get_run_time_sec() - start_time;
    cout << "Muparo finished in "<< runtime << " seconds"<<endl;
    
    build_result();
    
    return true;
}

bool Muparo::finished()
{
    bool heap_empties = true;
    bool goals_reached = true;
    
    BOOST_FOREACH(RLC::Dijkstra *d, dij) {
        if(! d->heap.empty() ) {
            heap_empties = false;
            break;
        }
    }
    
    if(!params.bidirectional) {
        BOOST_FOREACH(StateFreeNode n, goal_nodes) {
            if(!is_set(n)) {
                goals_reached = false;
                break;
            }
        }
    } else {
        if(connection_found) {
            if(best_cost <= min_cost(params.bidir_layers.first) + min_cost(params.bidir_layers.second))
                goals_reached = true;
        } else {
            goals_reached = false;
        }
    }
    
    return heap_empties || goals_reached;
}


int Muparo::select_layer()
{
    int best_cost = 999999999;
    int best_layer = -1;
    
    for(int i=0 ; i<num_layers ; ++i) {
        if( !dij[i]->heap.empty() && ( dij[i]->cost( dij[i]->heap.top().v ) < best_cost ) ) {
            best_cost = dij[i]->cost( dij[i]->heap.top().v );
            best_layer = i;
        }
    }
    
    return best_layer;
}


void Muparo::apply_rules ( int node )
{
    BOOST_FOREACH( PropagationRule rule, propagation_rules )
        rule.apply( node );
    
}

bool Muparo::insert ( StateFreeNode n, int arrival, int cost )
{
    bool inserted = false;
    int layer = n.first;
    int node = n.second;
    BOOST_FOREACH(int dfa_start, graphs[layer]->dfa_start_states())  
    {
        RLC::Vertice rlc_node;
        rlc_node.first = node;
        rlc_node.second = dfa_start;
        
        if( dij[layer]->insert_node( rlc_node, arrival, cost, RLC::Predecessor() ) )
            inserted = true;
    }
    
    return inserted;
}

void Muparo::build_result()
{
    std::list< CompleteNode > queue;
    
    if(params.bidirectional) {
        queue.push_back(CompleteNode(params.bidir_layers.first, best_connection));
        queue.push_back(CompleteNode(params.bidir_layers.second, best_connection));
        vres.c_nodes.push_back(best_connection.first);
    }
    
    BOOST_FOREACH(StateFreeNode n, goal_nodes) {
        queue.push_back( CompleteNode(n.first, RLC::Vertice(n.second, flags[n.first][n.second].dfa_state )));
        vres.a_nodes.push_back(n.second);
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
        else if( flags[l][vert.first].pred_layers.empty() )
        {
            vres.b_nodes.push_back(vert.first);
        }
        else
        {
            BOOST_FOREACH(int layer, flags[l][vert.first].pred_layers) {
                queue.push_back( CompleteNode(layer, RLC::Vertice(vert.first, flags[layer][vert.first].dfa_state )));
                vres.c_nodes.push_back(vert.first);
            }
        }
    }
}

void Muparo::check_connections ( const int modified_layer )
{
    BOOST_ASSERT(params.bidirectional);
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
                best_connection = v;
            }
        }
    }
}

int Muparo::min_cost ( const int layer ) const
{
    int min = 99999999;
    if( ! dij[layer]->heap.empty() ) {
        min = dij[layer]->cost( dij[layer]->heap.top().v );
    }
    
    BOOST_FOREACH(PropagationRule rule, propagation_rules) {
        if( !(layer == rule.insertion) )
            continue;
        
        int tmp_cost = 0;
        BOOST_FOREACH(int l_cond, rule.conditions) {
            tmp_cost = rule.combine_costs( tmp_cost, min_cost(l_cond) );
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