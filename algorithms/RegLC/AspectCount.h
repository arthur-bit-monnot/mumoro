#ifndef ASPECT_COUNT_H
#define ASPECT_COUNT_H

#include "DRegLC.h"

namespace RLC {

template<typename Base>
class AspectCount : public Base {
public:    
    typedef typename Base::ParamType ParamType;
    AspectCount( ParamType parameters ) : Base(parameters) { Base::count = 0; }
    virtual ~AspectCount() {}
    
    virtual RLC::Label treat_next() override {
        Base::count++;
        return Base::treat_next();
    }
    
};

}

#endif