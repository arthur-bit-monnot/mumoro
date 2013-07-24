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

#ifndef LANDMARK_H
#define LANDMARK_H

#include <string>
#include <vector>
#include <boost/concept_check.hpp>
#include <limits>
#include <graph_wrapper.h>
#include "Area.h"

#define INF (std::numeric_limits<int>::max() / 3)

namespace RLC {


class Landmark
{public:
    /**
     * Id of the graph this landmark is applied to
     */
    std::string graph_id;
    
    /**
     * Node in which the landmark is situated
     */
    int node;
    
    /**
     * Distances from all vertices to this landmark
     */
    std::vector<int> hplus;
    
    /**
     * Distances from this landmark to all other vertices
     */
    std::vector<int> hminus;
    
public:
    Landmark( const std::string graph_id, const int node, const int size ) : graph_id(graph_id), node(node) {
        hplus.resize( size, -1 );
        hminus.resize( size, -1 );
    }
    
    /**
     * Is there a path from the node to this landmark
     */
    inline bool forward_reachable( const int node ) const { return hplus[node] >= 0; }
    
    /**
     * Is there a path from this landmark to the node
     */
    inline bool backward_reachable( const int node ) const { return hminus[node] >= 0; }
    
    void set_hplus( const int node, const int dist ) {  hplus[node] = dist; }
    void set_hminus( const int node, const int dist ) {  hminus[node] = dist; }
    
    
    /**
     * Returns a lower bound of the distance between source and target
     */
    int dist_lb( const int source, const int target, const bool is_forward ) const {
        if( is_forward ) {
            const int dplus = potential_plus_forward( source, target );
            const int dminus = potential_minus_forward( source, target );
            return dplus > dminus ? dplus : dminus;
        } else {
            const int dplus = potential_plus_backward( source, target );
            const int dminus = potential_minus_backward( source, target );
            return dplus > dminus ? dplus : dminus;
        }
    }

private:
    /**
     * Following are the potential functions as defined in "Computing the Shortest PAth: A* Meets
     * Graph Theory".
     * 
     * Those are defined for both forward and backward search.
     */
    
    inline int potential_plus_forward( const int source, const int target ) const {
        BOOST_ASSERT( forward_reachable( source ) && forward_reachable( target ) );
        return hplus[source] - hplus[target];
    }

    inline int potential_minus_forward( const int source, const int target ) const {
        BOOST_ASSERT( backward_reachable( source ) && backward_reachable( target ) );
        return hminus[target] - hminus[source];
    }
    
    inline int potential_plus_backward( const int source, const int target ) const {
        BOOST_ASSERT( forward_reachable( source ) && forward_reachable( target ) );
        return hplus[target] - hplus[source];
    }
    
    inline int potential_minus_backward( const int source, const int target ) const {
        BOOST_ASSERT( backward_reachable( source ) && backward_reachable( target ) );
        return hminus[source] - hminus[target];
    }
    
};


/**
 * Creates a Landmark on 'node' for the graph 'trans'.
 * 
 * The computation uses a car DFA (the only considered are the car ones)
 */
Landmark * create_car_landmark( const Transport::Graph * trans, const int node );


} //end namespace RLC

#endif