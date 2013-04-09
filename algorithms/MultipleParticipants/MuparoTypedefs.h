#ifndef MUPARO_TYPEDEFS_H
#define MUPARO_TYPEDEFS_H

#include "AlgoTypedefs.h"
#include "muparo.h"
#include "MPR_AspectTarget.h"
#include "MPR_AspectPropagationRule.h"

using namespace MuPaRo;

namespace AlgoMPR {
    
    typedef AspectTarget<Muparo<Algo::Basic> > PtToPt;
    typedef AspectPropagationRule<AspectTarget<Muparo<Algo::Basic> > > SharedPath;
    
}



#endif