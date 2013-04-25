#ifndef ASPECT_TARGET_AREA_H
#define ASPECT_TARGET_AREA_H

#include "AspectMaxCostPruning.h"
#include "../utils/Area.h"

namespace RLC {

struct AspectTargetAreaParams {
    AspectTargetAreaParams( Area * area ) : area(area) {}
    Area * area;
};
    
    
template<typename Base>
class AspectTargetArea : public AspectMaxCostPruning<Base> 
{
private:
    bool max_cost_set = false;
    Area * area;
    
public:
    
    typedef LISTPARAM<AspectTargetAreaParams, typename Base::ParamType> ParamType;
    
    AspectTargetArea(ParamType p) : 
    AspectMaxCostPruning<Base>( typename AspectMaxCostPruning<Base>::ParamType(
        p.next,
        AspectMaxCostPruningParams( std::numeric_limits< int >::max() )) )
    {
        area = p.value.area;
    }
    
    virtual bool insert_node( const Vertice & node, const int arrival, const int cost ) {
        if( !max_cost_set && area->isIn( node.first ) ) {
            AspectMaxCostPruning<Base>::max_cost = cost + area->diameter;
            max_cost_set = true;
        }
        
        return AspectMaxCostPruning<Base>::insert_node( node, arrival, cost );
    }
};



}







#endif