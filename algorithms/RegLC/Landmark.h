#ifndef LANDMARK_H
#define LANDMARK_H

#include <string>
#include <vector>
#include <boost/concept_check.hpp>
#include <limits>
#include <graph_wrapper.h>

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
     * Distances of this landmark to all other vertices
     */
    std::vector<int> hminus;
    
public:
    Landmark( const std::string graph_id, const int node, const int size ) : graph_id(graph_id), node(node) {
        hplus.resize( size, -1);
        hminus.resize( size, -1);
    }
    
    inline bool forward_reachable( const int node ) { return hplus[node] >= 0; }
    inline bool backward_reachable( const int node ) const { return hminus[node] >= 0; }
    
    void set_hplus( const int node, const int dist ) {  hplus[node] = dist; }
    void set_hminus( const int node, const int dist ) {  hminus[node] = dist; }
    
    int dist_lb( const int source, const int target, const bool is_forward ) const {
        if( is_forward ) {
            const int dplus = hplus[source] - hplus[target];
            const int dminus = hminus[target] - hminus[source];
            return dplus > dminus ? dplus : dminus;
        } else {
            const int dplus = hminus[source] - hminus[target];
            const int dminus = hplus[target] - hplus[source];
            return dplus > dminus ? dplus : dminus;
        }
    }
    
    
};


Landmark * create_landmark( const Transport::Graph * trans, const int node );


} //end namespace RLC

#endif