#ifndef MUPARO_TYPEDEFS_H
#define MUPARO_TYPEDEFS_H

#include "AlgoTypedefs.h"
#include "muparo.h"
#include "MPR_AspectTarget.h"
#include "MPR_AspectPropagationRule.h"
#include "MPR_AspectCount.h"

using namespace MuPaRo;

namespace AlgoMPR {
    
    typedef AspectTarget<Muparo<Algo::Basic> > PtToPt;
    typedef AspectPropagationRule<AspectTarget<Muparo<Algo::Basic> > > SharedPath;
    typedef AspectPropagationRule<AspectPropagationRule<AspectTarget<Muparo<Algo::Basic> > > > CarSharing;
    typedef AspectCount<AspectPropagationRule<AspectPropagationRule<AspectTarget<Muparo<RLC::AspectCount<Algo::Basic> > > > > > CarSharingTest;
}



#endif