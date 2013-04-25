#ifndef ASPECT_TARGET_AREA_LANDMARK_H
#define ASPECT_TARGET_AREA_LANDMARK_H

#include "DRegLC.h"
#include "AspectTarget.h"
#include "Landmark.h"
#include "../utils/Area.h"

namespace RLC {
    
struct AspectTargetAreaLandmarkParams {
    AspectTargetAreaLandmarkParams( Area * area ) : target(target), area(area) {}
    const int target;
    const Area * area;
};


template<typename Base = DRegLC>
class AspectTargetAreaLandmark : public Base
{
    const Area * area = NULL;
    const Landmark * lm = NULL;
    int ** evaluated_costs;
    int target;
        
public:    
    typedef LISTPARAM<AspectTargetAreaLandmarkParams, typename Base::ParamType> ParamType;
    
    AspectTargetAreaLandmark( ParamType parameters ) : Base( parameters.next ) 
    {
        target = parameters.value.target;
        area = parameters.value.area;
        lm = area->lm;
        
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
        int h;
        if(Base::graph->forward)
            if( lm->hplus[v.first] < 0 )
                h = std::numeric_limits< int >::max() / 2;
            else
                h = (lm->hplus[v.first] - area->radius) * Base::cost_factor;
        else
            if( lm->hminus[v.first] < 0 )
                h = std::numeric_limits< int >::max() / 2;
            else
                h = (lm->hminus[v.first] - area->radius) * Base::cost_factor;
        if(h < 0)
            h = 0;
        evaluated_costs[v.second][v.first] = cost + h;
    }
    
    virtual inline int cost_eval( const Vertice & v, const int cost ) const override { 
        int h;
        if(Base::graph->forward)
            if( lm->hplus[v.first] < 0 )
                h = std::numeric_limits< int >::max() / 2;
            else
                h = (lm->hplus[v.first] - area->radius) * Base::cost_factor;
        else
            if( lm->hminus[v.first] < 0 )
                h = std::numeric_limits< int >::max() / 2;
            else
                h = (lm->hminus[v.first] - area->radius) * Base::cost_factor;
        if( h < 0 )
            h = 0;
        return  cost + h;
    }

};


}


#endif
