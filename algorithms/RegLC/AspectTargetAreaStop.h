#ifndef ASPECT_TARGET_AREA_STOP_H
#define ASPECT_TARGET_AREA_STOP_H

#include "../utils/Area.h"

namespace RLC {

struct AspectTargetAreaStopParams {
    AspectTargetAreaStopParams( Area * area ) : area(area) {}
    Area * area;
};
    
/**
 * This Aspect add a stop condition when all car accessible nodes in the area have been set
 */
template<typename Base>
class AspectTargetAreaStop : public Base
{
private:
    int car_nodes_set = 0;
    Area * area;
    
public:
    
    typedef LISTPARAM<AspectTargetAreaStopParams, typename Base::ParamType> ParamType;
    
    AspectTargetAreaStop(ParamType p) : Base( p.next )
    {
        area = p.value.area;
    }

    virtual Label treat_next() override {
        RLC::Label l = Base::treat_next();
        if( area->isIn( l.node.first ) && Base::graph->transport->car_accessible( l.node.first ) ) {
            ++car_nodes_set;                
        }
        return l;
    }
    
    virtual bool finished() const override {
        return Base::finished() || (car_nodes_set >= area->num_car_accessible);
    }
};



}


#endif