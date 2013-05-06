#ifndef LANDMARK_SET_H
#define LANDMARK_SET_H

#include <boost/foreach.hpp>
#include "Landmark.h"

namespace RLC {

class LandmarkSet
{
    std::list<const Landmark*> landmarks;
    
public:
    LandmarkSet(std::list<const Landmark*> landmarks) : landmarks(landmarks) {
        
    }
    
    int dist_lb( const int source, const int target, const bool is_forward ) const {
        int best = 0;
        BOOST_FOREACH( const Landmark * l, landmarks ) {
            int curr = l->dist_lb( source, target, is_forward );
            if( curr > best )
                best = curr;
        }
        
        return best;
    }
    
    int dist_lb( const int source, const Area & area, const bool is_forward ) const {   
        int best = 0;
        BOOST_FOREACH( const Landmark * l, landmarks ) {
            int curr = l->dist_lb( source, area, is_forward );
            if( curr > best )
                best = curr;
        }
        
        return best;
    }
    
};



} // end namespace RLC







#endif