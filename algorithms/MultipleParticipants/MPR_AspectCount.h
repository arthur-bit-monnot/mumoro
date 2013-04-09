#ifndef MPR_ASPECT_COUNT_H
#define MPR_ASPECT_COUNT_H

#include "muparo.h"


namespace MuPaRo {


template<typename Base>
class AspectCount : public Base {
public:    
    typedef typename Base::ParamType ParamType;
    AspectCount( ParamType p ) : Base(p) { count = 0; }

    int count;
    
    virtual CompleteNode proceed_one_step() override {
        ++count;
        return Base::proceed_one_step();
    }
};

} //end namespace MuPaRo
        
        
#endif