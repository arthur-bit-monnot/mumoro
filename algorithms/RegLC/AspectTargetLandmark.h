#ifndef ASPECT_TARGET_LANDMARK_H
#define ASPECT_TARGET_LANDMARK_H

#include "DRegLC.h"
#include "AspectTarget.h"
#include "Landmark.h"

namespace RLC {
    
template<typename H = Landmark>
struct AspectTargetLandmarkParams {
    AspectTargetLandmarkParams( const int target, const H * h ) : target(target), h(h) {}
    const int target;
    const H * h;
};


/**
 * Aspect implementing an A* search towards a single target.
 * 
 * It needs a heuristic H (default is landmark) to provide a lower bound of the distance 
 * between two vertices in the graph.
 */
template<typename Base = DRegLC, typename H = Landmark>
class AspectTargetLandmark : public AspectTarget<Base> 
{
    /**
     * Provide a heuristic giving lower bound of distance to target from any point in the graph.
     * Needs to be admissible.
     */
    const H * h = NULL;
    
    /** 
     * Node we're willing to reach
     */
    int target;
        
public:    
    typedef LISTPARAM<AspectTargetLandmarkParams<H>, typename Base::ParamType> ParamType;
    
    AspectTargetLandmark( ParamType parameters ) : 
    AspectTarget<Base>(typename AspectTarget<Base>::ParamType(
        typename Base::ParamType( parameters.next ),
        AspectTargetParams( parameters.value.target )  )) 
    {
        target = parameters.value.target;
        h = parameters.value.h;
    }
    
    virtual ~AspectTargetLandmark() {}
    
    virtual Label label(RLC::Vertice vert, int time, int cost, int source = -1) const override {
        Label l = Base::label(vert, time, cost, source);
        l.h = h->dist_lb( vert.first, target, Base::graph->forward ) * Base::cost_factor;
        
        BOOST_ASSERT( l.valid() );
        return l;
    }

};


}


#endif
