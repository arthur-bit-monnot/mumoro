#ifndef ASPECT_NO_RUN_H
#define ASPECT_NO_RUN_H

#include "DRegLC.h"

namespace RLC {

template<typename Base>
class AspectNoRun : public Base {
public:    
    typedef typename Base::ParamType ParamType;
    AspectNoRun( ParamType parameters ) : Base(parameters) { }
    virtual ~AspectNoRun() {}
    
    virtual bool finished() const override {
        return true;
    }
};

}

#endif