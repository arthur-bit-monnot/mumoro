#include <boost/foreach.hpp>

#include "debug/cwd_sys.h"
#include "debug/cwd_debug.h"

#include "MultipleParticipants/muparo.h"
#include "MultipleParticipants/run_configurations.h"
#include "utils/node_filter_utils.h"
#include "utils/Area.h"

#include "RegLC/AspectTarget.h"
#include "RegLC/AlgoTypedefs.h"

#include "MultipleParticipants/PotentialMeetingPoints.h"

using namespace RLC;

int main() 
{
    Debug( dc::notice.on() );             // Turn on the NOTICE Debug Channel.
    Debug( libcw_do.on() );               // Turn on the default Debug Object.
  
//     Transport::Graph g("/home/arthur/LAAS/mumoro/976c9329c82da079f78be26dddcf1174.dump");
//     Transport::Graph g("/home/arthur/LAAS/Data/Graphs/toulouse-mixed.dump");
    const Transport::Graph g("/home/arthur/LAAS/Data/Graphs/sud-ouest.dump");
    
    
    RLC::Graph rlc(&g, RLC::car_dfa());
//     RLC::BackwardGraph back_rlc( &rlc );
    
    Area * toulouse = toulouse_area(&g);
    Area * bordeaux = bordeaux_area(&g);
    
    toulouse->init();
    bordeaux->init();
    
    typedef RLC::AspectCount<Algo::TargetArea> MyAlgo;
    
    MyAlgo::ParamType p(
        DRegLCParams(&rlc, 10),
        AspectTargetAreaParams( toulouse )
    );
    
    MyAlgo algo( p );
    algo.insert_node(RLC::Vertice(190562, 0), 0, 0);
    algo.run();
    
    cout << "Num vertices : " << algo.count <<endl;
    
}