#ifndef ASPECT_TARGET_AREA_LANDMARK_H
#define ASPECT_TARGET_AREA_LANDMARK_H

#include "DRegLC.h"
#include "AspectTarget.h"
#include "Landmark.h"
#include "../utils/Area.h"

namespace RLC {
    
template<typename H = Landmark>
struct AspectTargetAreaLandmarkParams {
    AspectTargetAreaLandmarkParams( const Area * area, H * h ) : area(area), h(h) {}
    const Area * area;
    H * h;
};


template<typename Base = DRegLC, typename H = Landmark>
class AspectTargetAreaLandmark : public Base
{
    /**
     * Provide a heuristic giving lower bound of distance to target from any point in the graph.
     * Needs to be admissible.
     */
    H * h = NULL;
    
    /**
     * Stores an evaluation of the cost of nodes. This is the sum of:
     *  - cost to reach the node from the start (exact)
     *  - cost to reach the target from the node (lower bound provided by h)
     */
    int ** evaluated_costs;
    
    /** 
     * Area we're willing to reach
     */
    const Area * area = NULL;
        
public:    
    typedef LISTPARAM<AspectTargetAreaLandmarkParams<H>, typename Base::ParamType> ParamType;
    
    AspectTargetAreaLandmark( ParamType parameters ) : Base( parameters.next ) 
    {
        area = parameters.value.area;
        h = parameters.value.h;
        evaluated_costs = new int*[Base::dfa_num_vert];
        for(int i=0 ; i<Base::dfa_num_vert ; ++i) {
            evaluated_costs[i] = new int[Base::trans_num_vert];
        }
        
        if( Base::heap != NULL )
            delete Base::heap;
        Base::heap = new DRegHeap( DRegCompare( evaluated_costs ) ) ;
        
        
    }
    virtual ~AspectTargetAreaLandmark() {
        for(int i=0 ; i<Base::dfa_num_vert ; ++i) {
            delete evaluated_costs[i];
        }
        delete evaluated_costs;
    }

    virtual void set_cost(const RLC::Vertice v, const int cost) override { 
        Base::set_cost( v, cost );
        evaluated_costs[v.second][v.first] = cost + h->dist_lb( v.first, *area, Base::graph->forward) * Base::cost_factor;
    }

};


}


#endif
