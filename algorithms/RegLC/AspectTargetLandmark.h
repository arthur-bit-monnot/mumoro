#ifndef ASPECT_TARGET_LANDMARK_H
#define ASPECT_TARGET_LANDMARK_H

#include "DRegLC.h"
#include "AspectTarget.h"
#include "Landmark.h"

namespace RLC {
    
struct AspectTargetLandmarkParams {
    AspectTargetLandmarkParams( const int target, const Landmark * lm ) : target(target), lm(lm) {}
    const int target;
    const Landmark * lm;
};


template<typename Base = DRegLC>
class AspectTargetLandmark : public AspectTarget<Base> 
{
    const Landmark * lm = NULL;
    int ** evaluated_costs;
    int target;
        
public:    
    typedef LISTPARAM<AspectTargetLandmarkParams, typename Base::ParamType> ParamType;
    
    AspectTargetLandmark( ParamType parameters ) : 
    AspectTarget<Base>(typename AspectTarget<Base>::ParamType(
        typename Base::ParamType( parameters.next ),
        AspectTargetParams( parameters.value.target )  )) 
    {
        target = parameters.value.target;
        lm = parameters.value.lm;
        
        evaluated_costs = new int*[Base::dfa_num_vert];
        for(int i=0 ; i<Base::dfa_num_vert ; ++i) {
            evaluated_costs[i] = new int[Base::trans_num_vert];
        }

        if( Base::heap != NULL )
            delete Base::heap;
        Base::heap = new DRegHeap( DRegCompare( evaluated_costs ) ) ;
    }
    
    virtual ~AspectTargetLandmark() {
        for(int i=0 ; i<Base::dfa_num_vert ; ++i) {
            delete evaluated_costs[i];
        }
        delete evaluated_costs;
    }

    virtual void set_cost(const RLC::Vertice v, const int cost) override { 
        Base::set_cost( v, cost );
        evaluated_costs[v.second][v.first] = cost + lm->dist_lb( v.first, target, Base::graph->forward );
    }

};


}


#endif
