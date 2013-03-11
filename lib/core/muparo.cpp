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
    
Muparo::Muparo(Transport::Graph * trans, int start1, int start2) :
transport(trans),
vres(transport)
{
    num_layers = 3;
    int day = 10;
    
    dfas.push_back(RLC::all_dfa());
    dfas.push_back(RLC::bike_pt_dfa());
    dfas.push_back(RLC::bike_pt_dfa());
    
    for(int i=0; i<num_layers ; ++i)
    {
        graphs.push_back( new RLC::Graph(this->transport, dfas[i] ));
        dij.push_back( new RLC::Dijkstra(graphs[i], -1, -1, -1, day) );
    }
    
    ConnectionRule cr;
    cr.conditions.push_back(0);
    cr.conditions.push_back(1);
    cr.insertions.push_back(2);
    rules.push_back(cr);
    
    start_nodes.push_back( StartNode( StateFreeNode(0, start1), 50000) );
    start_nodes.push_back( StartNode( StateFreeNode(1, start2), 50000) );
    goal_nodes.push_back( StateFreeNode(2, 172807) );
    
    flags = new Flag* [num_layers];
    for(int i=0; i<num_layers ; ++i) {
        flags[i] = new Flag[boost::num_vertices(transport->g)];
        memset(flags[i], 0, sizeof(Flag) * boost::num_vertices(transport->g));
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
        insert(sn.first, sn.second);
    }
    
    while(!finished()) {
        int layer = select_layer();
        
        RLC::Vertice vert = dij[layer]->treat_next();
        StateFreeNode node(layer, vert.first);
        
        if( graphs[layer]->is_accepting( vert ) && !is_set( node ) ) {
            set( CompleteNode(layer, vert) );
            apply_rules( node.second );
        }
    }
    
    double runtime = get_run_time_sec() - start_time;
    cout << "Muparo finished in "<< runtime << " seconds"<<endl;
    
    build_result();
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
    
    BOOST_FOREACH(StateFreeNode n, goal_nodes) {
        if(!is_set(n)) {
            goals_reached = false;
            break;
        }
    }
    
    return heap_empties || goals_reached;
}


int Muparo::select_layer()
{
    int best_cost = 999999999;
    int best_layer = -1;
    
    for(int i=0 ; i<num_layers ; ++i) {
        if( !dij[i]->heap.empty() && ( dij[i]->arrival( dij[i]->heap.top().v ) < best_cost ) ) {
            best_cost = dij[i]->arrival( dij[i]->heap.top().v );
            best_layer = i;
        }
    }
    
    return best_layer;
}


void Muparo::apply_rules ( int node )
{
    BOOST_FOREACH( ConnectionRule rule, rules )
    {
        bool applicable = true;
        BOOST_FOREACH( int layer, rule.conditions ) {
            if( !is_set( StateFreeNode(layer, node) ) ) {
                applicable = false;
                break;
            }
        }
        
        if( applicable ) {
            int arr = -99999999;
            BOOST_FOREACH( int layer, rule.conditions ) {
                if( arrival( StateFreeNode(layer, node) ) > arr )
                    arr = arrival( StateFreeNode(layer, node) );
            }
            BOOST_FOREACH( int layer, rule.insertions ) {
                insert( StateFreeNode(layer, node), arr );
            }
        }   
    }
}

void Muparo::insert ( StateFreeNode n, int arrival )
{
    int layer = n.first;
    int node = n.second;
    BOOST_FOREACH(int dfa_start, graphs[layer]->dfa_start_states()) 
    {
        RLC::Vertice rlc_node;
        rlc_node.first = node;
        rlc_node.second = dfa_start;
        
        if(dij[layer]->white(rlc_node))
        {
            dij[layer]->set_arrival(rlc_node, arrival);
            dij[layer]->put_dij_node(rlc_node);
            dij[layer]->set_gray(rlc_node);
        }
        else if( arrival < dij[layer]->arrival(rlc_node) )
        {
            BOOST_ASSERT(!dij[layer]->black(rlc_node));
            
            dij[layer]->set_arrival(rlc_node, arrival);
            dij[layer]->clear_pred(rlc_node);
            dij[layer]->heap.update(dij[layer]->handle(rlc_node));
        }
    }
}

void Muparo::build_result()
{
    RLC::Vertice rlc_dest;
    rlc_dest.first = (*goal_nodes.begin()).second;
    rlc_dest.second = flags[2][rlc_dest.first].dfa_state;
    
    BOOST_ASSERT(dij[2]->black(rlc_dest));
    BOOST_ASSERT(graphs[2]->is_accepting(rlc_dest));

    RLC::Vertice curr;
    vres.a_nodes.push_back(rlc_dest.first);
    
    for(curr = rlc_dest ; dij[2]->has_pred(curr) ; curr = graphs[2]->source(dij[2]->get_pred(curr)))
        vres.edges.push_back(transport->edgeIndex( dij[2]->get_pred(curr).first ));
    
    vres.b_nodes.push_back(curr.first);
    
    int m = curr.first;
    curr.first = m;
    curr.second = flags[0][m].dfa_state;
    
    for(; dij[0]->has_pred(curr) ; curr = graphs[0]->source(dij[0]->get_pred(curr))) 
        vres.edges.push_back(transport->edgeIndex( dij[0]->get_pred(curr).first ));
    
    vres.c_nodes.push_back(curr.first);
    
    curr.first = m;
    curr.second = flags[1][m].dfa_state;
    
    for(; dij[1]->has_pred(curr) ; curr = graphs[1]->source(dij[1]->get_pred(curr))) 
        vres.edges.push_back(transport->edgeIndex( dij[1]->get_pred(curr).first ));
    
    vres.c_nodes.push_back(curr.first);
}


}