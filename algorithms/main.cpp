#include <boost/foreach.hpp>

#include "debug/cwd_sys.h"
#include "debug/cwd_debug.h"

#include "MultipleParticipants/muparo.h"
#include "MultipleParticipants/run_configurations.h"
#include "utils/node_filter_utils.h"
#include "utils/Area.h"

#include "RegLC/AspectTarget.h"
#include "RegLC/AlgoTypedefs.h"
#include "RegLC/AspectTargetLandmark.h"
#include "RegLC/AspectTargetAreaLandmark.h"
#include "RegLC/LandmarkSet.h"

#include "DataStructures/GraphFactory.h"

#include "MultiObjectives/Martins.h"

using namespace RLC;

int main() 
{
//     std::string file( "/home/arthur/LAAS/Data/Graphs/sud-ouest.dump" );
    std::string file( "/home/arthur/LAAS/Data/Graphs/toulouse.dump" );
    Transport::GraphFactory gf( file, true );
    
    const Transport::Graph * g = gf.get();
    
    RLC::Graph rlc(g, RLC::foot_dfa());
    RLC::BackwardGraph back_rlc( &rlc );
    
//     Area * toulouse = toulouse_area(g);
//     Area * bordeaux = bordeaux_area(g);
    
    int src1 = 10;
    int src2 = 644;
    int dest = 235;//20; //693;
    
    int time = 50000;
    int day = 10;
    
    {
        typedef AspectPropagationRule<Muparo<DRegLC>> AlgoMulti;
        
        AlgoMulti::ParamType p(
            MuparoParams( g, 3),
            AspectPropagationRuleParams( SumPlusWaitCost, MaxArrival, 2, 0, 1 )
        );
        
        AlgoMulti algo( p );
        
        for(int i=0; i<2 ; ++i)
        {
            algo.graphs.push_back( new RLC::Graph(algo.transport, RLC::car_dfa() ));
            DRegLC::ParamType p( RLC::DRegLCParams( algo.graphs[i], day) );
            algo.dij.push_back( new DRegLC( p ) );
        }
        
        algo.graphs.push_back( new RLC::Graph(algo.transport, pt_foot_dfa() ));
        Martins * m = new Martins(algo.graphs[2], dest, day);
        algo.dij.push_back( m );
        
        algo.insert( StateFreeNode(0, src1), time, 0);
        algo.insert( StateFreeNode(1, src2), time, 0);
        algo.run();
        
        if( m->success ) {
            cout << "Stats : "<< m->undominated_iter <<" " << m->total_iter <<endl;
        }
    }
    
}