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
     * Area we're willing to reach
     */
    const Area * area = NULL;
        
public:    
    typedef LISTPARAM<AspectTargetAreaLandmarkParams<H>, typename Base::ParamType> ParamType;
    
    AspectTargetAreaLandmark( ParamType parameters ) : Base( parameters.next ) 
    {
        area = parameters.value.area;
        h = parameters.value.h;
    }
    virtual ~AspectTargetAreaLandmark() {}
    
    virtual Label label(RLC::Vertice vert, int time, int cost, int source = -1) const override {
        Label l = Base::label(vert, time, cost, source);
        l.h = h->dist_lb( vert.first, *area, Base::graph->forward ) * Base::cost_factor;
        
        BOOST_ASSERT( l.valid() );
        return l;
    }

};


}


#endif
