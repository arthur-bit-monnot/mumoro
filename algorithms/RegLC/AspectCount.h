#ifndef ASPECT_COUNT_H
#define ASPECT_COUNT_H

#include "DRegLC.h"

namespace RLC {

template<typename Base>
class AspectCount : public Base {
public:    
    typedef typename Base::ParamType ParamType;
    AspectCount( ParamType parameters ) : Base(parameters) { count = 0; }
    virtual ~AspectCount() {}
    
    int count;
    
    virtual RLC::Vertice treat_next() {
        count++;
        return Base::treat_next();
    }
    
};

}

#endif