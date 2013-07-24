/** Copyright : Arthur Bit-Monnot (2013)  arthur.bit-monnot@laas.fr

This software is a computer program whose purpose is to [describe
functionalities and technical features of your software].

This software is governed by the CeCILL-B license under French law and
abiding by the rules of distribution of free software.  You can  use, 
modify and/ or redistribute the software under the terms of the CeCILL-B
license as circulated by CEA, CNRS and INRIA at the following URL
"http://www.cecill.info". 

As a counterpart to the access to the source code and  rights to copy,
modify and redistribute granted by the license, users are provided only
with a limited warranty  and the software's author,  the holder of the
economic rights,  and the successive licensors  have only  limited
liability. 

In this respect, the user's attention is drawn to the risks associated
with loading,  using,  modifying and/or developing or reproducing the
software by the user in light of its specific status of free software,
that may mean  that it is complicated to manipulate,  and  that  also
therefore means  that it is reserved for developers  and  experienced
professionals having in-depth computer knowledge. Users are therefore
encouraged to load and test the software's suitability as regards their
requirements in conditions enabling the security of their systems and/or 
data to be ensured and,  more generally, to use and operate it in the 
same conditions as regards security. 

The fact that you are presently reading this means that you have had
knowledge of the CeCILL-B license and that you accept its terms. 
*/

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