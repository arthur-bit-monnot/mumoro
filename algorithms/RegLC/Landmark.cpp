#include "Landmark.h"

#include "reglc_graph.h"
#include "graph_wrapper.h"
#include "DRegLC.h"
#include <graph_wrapper.h>

namespace RLC {

Landmark * create_car_landmark ( const Transport::Graph* trans, const int node )
{
    int day = 10;
    Landmark * lm =  new Landmark( trans->get_id(), node, trans->num_vertices() );
        
    RLC::Graph g( trans, RLC::car_dfa() );
    RLC::BackwardGraph bg( &g );
    {
        DRegLCParams p( &g, day );
        DRegLC algo( p );
        algo.add_source_node(Vertice(node, 0), 0, 0 );
        
        while( !algo.finished() ) {
            RLC::Label lab = algo.treat_next();
            if( !lm->backward_reachable( lab.node.first ) )
                lm->set_hminus(lab.node.first, lab.cost);
        }
    }
    {
        DRegLCParams p( &bg, day );
        DRegLC algo( p );
        algo.add_source_node(Vertice(node, 0), 0, 0 );
        
        while( !algo.finished() ) {
            RLC::Label lab = algo.treat_next();
            if( !lm->forward_reachable( lab.node.first ) )
                lm->set_hplus(lab.node.first, lab.cost);
        }
    }
    
    /*
    for(int i=0 ; i<trans->num_vertices() ; ++i) {
        if( trans->car_accessible( i ) ) {
            std::cerr <<node<<" "<<i<<" "<< lm->hplus[i] << " " << lm->hminus[i] <<endl;
            if( !lm->backward_reachable( i ) || !lm->forward_reachable( i ) ) {
                
                std::cerr<< "Coverage is not full\n";
            }
        }
    }
    */
    return lm;
}


} // end namespace RLC