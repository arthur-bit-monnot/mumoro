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
     * Stores an evaluation of the cost of nodes. This is the sum of:
     *  - cost to reach the node from the start (exact)
     *  - cost to reach the target from the node (lower bound provided by h)
     */
    int ** evaluated_costs;
    
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
        
        evaluated_costs = new int*[Base::dfa_num_vert];
        for(int i=0 ; i<Base::dfa_num_vert ; ++i) {
            evaluated_costs[i] = new int[Base::trans_num_vert];
        }

        // overrides the default heap and make it use the cost evaluation provided by h
        if( Base::heap != NULL )
            delete Base::heap;
        Base::heap = new DRegHeap(  ) ; //TODO: customize heap
    }
    
    virtual ~AspectTargetLandmark() {
        for(int i=0 ; i<Base::dfa_num_vert ; ++i) {
            delete evaluated_costs[i];
        }
        delete evaluated_costs;
    }

    virtual void set_cost(const RLC::Vertice v, const int cost) override { 
        Base::set_cost( v, cost );
        evaluated_costs[v.second][v.first] = cost + h->dist_lb( v.first, target, Base::graph->forward );
    }

};


}


#endif
