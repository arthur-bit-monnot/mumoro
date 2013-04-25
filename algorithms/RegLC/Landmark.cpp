#include "Landmark.h"

#include "reglc_graph.h"
#include "graph_wrapper.h"
#include "DRegLC.h"

namespace RLC {

Landmark * create_landmark ( const Transport::Graph* trans, const int node )
{
    int day = 10;
    Landmark * lm =  new Landmark( trans->get_id(), node, trans->num_vertices() );
        
    RLC::Graph g( trans, RLC::car_dfa() );
    RLC::BackwardGraph bg( &g );
    {
        DRegLCParams p( &g, day );
        DRegLC algo( p );
        algo.insert_node(Vertice(node, 0), 0, 0 );
        algo.run();
        
        for(int i=0 ; i<g.num_transport_vertices() ; ++i ) {
            if( algo.black(Vertice(i, 0) ) ) {
                int cost = algo.cost(Vertice(i, 0));
                lm->set_hminus(i, cost);
            }
        }
    }
    {
        DRegLCParams p( &bg, day );
        DRegLC algo( p );
        algo.insert_node(Vertice(node, 0), 0, 0 );
        algo.run();
        
        for(int i=0 ; i<g.num_transport_vertices() ; ++i ) {
            if( algo.black(Vertice(i, 0) ) ) {
                int cost = algo.cost(Vertice(i, 0));
                lm->set_hplus(i, cost);
            }
        }
    }
    
    return lm;
}


} // end namespace RLC