#ifndef ASPECT_MIN_COST_H
#define ASPECT_MIN_COST_H

#include "DRegLC.h"

namespace RLC {

template<typename Base>
class AspectMinCost : public Base {
public:    
    typedef typename Base::ParamType ParamType;
    AspectMinCost( ParamType parameters ) : Base(parameters) {}
    virtual ~AspectMinCost() {}

    virtual std::pair <bool, int> duration( const RLC::Edge& edge, const float start_sec, const int day ) const {
        return Base::graph->min_duration( edge );
    }
    
};

}

#endif