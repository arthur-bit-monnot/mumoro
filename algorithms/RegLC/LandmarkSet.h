#ifndef LANDMARK_SET_H
#define LANDMARK_SET_H

#include <boost/foreach.hpp>
#include "Landmark.h"

using std::cout;
using std::endl;

namespace RLC {


class LandmarkSet
{
    const uint num_landmarks = 3;
    int * potentials;
    std::vector<int> area_potentials;
    const Transport::Graph * g;
    
    uint potential_index(const uint vertex, const uint landmark) const {
        return vertex * num_landmarks * 2 + landmark * 2;
    }
    
    uint size_per_node() const {
        return num_landmarks * 2;
    }
    
public:
    /**
     * Boolean to choose which evaluation method to use.
     */
    bool use_maxmin = true;
    
    LandmarkSet(std::vector<const Landmark*> landmarks, const Transport::Graph * g) : g(g) {
        uint num_vertices = landmarks[0]->hplus.size();
        potentials = new int[ num_vertices * num_landmarks * 2 ];
        
        for(uint l = 0 ; l<landmarks.size() ; ++l) {
            for(uint v = 0 ; v < num_vertices ; ++v) {
                potentials[potential_index(v, l)] = landmarks[l]->hplus[v];
                potentials[potential_index(v, l)+1] = landmarks[l]->hminus[v];
            }
        }
    }
    
    int dist_lb( const int source, const int target, const bool is_forward ) const {
        int best = 0;
        int * p_pot_src = potentials + potential_index(source, 0);
        int * p_pot_target = potentials + potential_index(target, 0);
        
        if(is_forward) {
            for(uint l=0 ; l<num_landmarks ; ++l) {
                if( *p_pot_src - *p_pot_target > best )
                    best = *p_pot_src - *p_pot_target;
                ++p_pot_src; ++p_pot_target;
                
                if( *p_pot_target - *p_pot_src > best )
                    best = *p_pot_target - *p_pot_src;
                ++p_pot_src; ++p_pot_target;
            }
        } else {
            for(uint l=0 ; l<num_landmarks ; ++l) {
                if( *p_pot_target - *p_pot_src > best )
                    best = *p_pot_target - *p_pot_src;
                ++p_pot_src; ++p_pot_target;
                
                if( *p_pot_src - *p_pot_target > best )
                    best = *p_pot_src - *p_pot_target;
                ++p_pot_src; ++p_pot_target;
            }
        }
        
        return best;
    }
    
    void set_potentials( const Area & area ) {
        uint it = area.id * size_per_node();
        for(uint offset = 0 ; offset < num_landmarks*2 ; offset += 2) {
            area_potentials[it+offset] = 0;
            area_potentials[it+offset+1] = std::numeric_limits<int>::max();
        }
        for(int i = 0 ; i < area.size() ; ++i) {
            int v = area.get(i);
            if( g->car_accessible( v ) ) {
                int * p_pot = potentials + potential_index(v, 0);
                for(uint offset = 0 ; offset < num_landmarks*2 ; offset += 2, p_pot += 2) {
                    if(area_potentials[it+offset] < *p_pot ) 
                        area_potentials[it+offset] = *p_pot;
                    if(area_potentials[it+offset+1] > *(p_pot+1) ) 
                        area_potentials[it+offset+1] = *(p_pot+1);
                }
            }
        }
    }
    
    // TODO: choice of the distance evaluation method should be done at compilation time
    int dist_lb( const int source, const Area & area, const bool is_forward ) { 
        if( use_maxmin )
            return dist_lb_maxmin( source, area, is_forward );
        else
            return dist_lb_rad( source, area, is_forward );
    }
    
    /**
     * Uses maximal (resp. minimal) distance of all nodes in the area for h+(area) (resp. h-(area))
     */
    int dist_lb_maxmin( const int source, const Area & area, const bool is_forward ) {   
        if( area_potentials.size() <= area.id * size_per_node() ) {
            area_potentials.resize((area.id + 1) * size_per_node(), -1);
        }
        if(area_potentials[ area.id * size_per_node() ] < 0)
            set_potentials(area);
        
        int best = 0;
        
        int * p_pot_src = potentials + potential_index(source, 0);
        uint it_area =  potential_index(area.id, 0);
        
        if(is_forward) {
            for(uint l=0 ; l<num_landmarks ; ++l) {
                BOOST_ASSERT( it_area < area_potentials.size() );
                
                if( *p_pot_src - area_potentials[it_area] > best )
                    best = *p_pot_src - area_potentials[it_area];
                ++p_pot_src; ++it_area;
                
                if( area_potentials[it_area] - *p_pot_src > best )
                    best = area_potentials[it_area] - *p_pot_src;
                ++p_pot_src; ++it_area;
            }
        } else {
            for(uint l=0 ; l<num_landmarks ; ++l) {
                BOOST_ASSERT( it_area < area_potentials.size() );
                
                if( area_potentials[it_area] - *p_pot_src > best )
                    best = area_potentials[it_area] - *p_pot_src;
                ++p_pot_src; ++it_area;
                
                if( *p_pot_src - area_potentials[it_area] > best )
                    best = *p_pot_src - area_potentials[it_area];
                ++p_pot_src; ++it_area;
            }
        }
        
        return best;
    }
    
    
    /**
     * Uses the distance to area's center minus the area's radius as distance estimation
     */
    int dist_lb_rad( const int source, const Area & area, const bool is_forward ) const {   
        
        int target = area.center;
        int radius = area.radius;
        int best = radius;
        
        int * p_pot_src = potentials + potential_index(source, 0);
        int * p_pot_target = potentials + potential_index(target, 0);
        
        if(is_forward) {
            for(uint l=0 ; l<num_landmarks ; ++l) {
                if( *p_pot_src - *p_pot_target > best )
                    best = *p_pot_src - *p_pot_target;
                ++p_pot_src; ++p_pot_target;
                
                if( *p_pot_target - *p_pot_src > best )
                    best = *p_pot_target - *p_pot_src;
                ++p_pot_src; ++p_pot_target;
            }
        } else {
            for(uint l=0 ; l<num_landmarks ; ++l) {
                if( *p_pot_target - *p_pot_src > best )
                    best = *p_pot_target - *p_pot_src;
                ++p_pot_src; ++p_pot_target;
                
                if( *p_pot_src - *p_pot_target > best )
                    best = *p_pot_src - *p_pot_target;
                ++p_pot_src; ++p_pot_target;
            }
        }
        
        best -= radius;
        
        return best;
    }
    
};


class LandmarkSetNaive
{
    std::list<const Landmark*> landmarks;
    
public:
    LandmarkSetNaive(std::list<const Landmark*> landmarks) : landmarks(landmarks) {
        
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