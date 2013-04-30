#ifndef ASPECT_TARGET_AREA_LANDMARK_H
#define ASPECT_TARGET_AREA_LANDMARK_H

#include "DRegLC.h"
#include "AspectTarget.h"
#include "Landmark.h"
#include "../utils/Area.h"

namespace RLC {
    
struct AspectTargetAreaLandmarkParams {
    AspectTargetAreaLandmarkParams( const Area * area, const Landmark * lm ) : area(area), lm(lm) {}
    const Area * area;
    const Landmark * lm;
};


template<typename Base = DRegLC>
class AspectTargetAreaLandmark : public Base
{
    const Area * area = NULL;
    const Landmark * lm = NULL;
    int ** evaluated_costs;
        
public:    
    typedef LISTPARAM<AspectTargetAreaLandmarkParams, typename Base::ParamType> ParamType;
    
    AspectTargetAreaLandmark( ParamType parameters ) : Base( parameters.next ) 
    {
        area = parameters.value.area;
        lm = parameters.value.lm;
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
        int h = lm->dist_lb( v.first, *area, Base::graph->forward) * Base::cost_factor;
        evaluated_costs[v.second][v.first] = cost + h;
    }
    
    virtual inline int cost_eval( const Vertice & v, const int cost ) const override { 
        int h = lm->dist_lb( v.first, *area, Base::graph->forward) * Base::cost_factor;
        return  cost + h;
    }

};


}


#endif
