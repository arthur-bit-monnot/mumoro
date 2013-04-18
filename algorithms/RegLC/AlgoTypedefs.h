#ifndef ALGO_TYPEDEFS_H
#define ALGO_TYPEDEFS_H

#include "DRegLC.h"
#include "AspectTarget.h"
#include "AspectCount.h"
#include "AspectMaxCostPruning.h"
#include "AspectMinCost.h"
#include "AspectNoRun.h"
#include "AspectNodePruning.h"
#include "AspectTargetArea.h"

namespace Algo {
    
    typedef RLC::DRegLC Basic;
    typedef RLC::AspectTarget<RLC::DRegLC> PtToPt;
    typedef RLC::AspectNodePruning<RLC::DRegLC> Filtered;
    typedef RLC::AspectTargetArea<RLC::DRegLC> TargetArea;
    
}


#endif