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

#include "MultipleParticipants/PotentialMeetingPoints.h"

using namespace RLC;

int main() 
{
    Debug( dc::notice.on() );             // Turn on the NOTICE Debug Channel.
    Debug( libcw_do.on() );               // Turn on the default Debug Object.
  
//     Transport::Graph g("/home/arthur/LAAS/mumoro/976c9329c82da079f78be26dddcf1174.dump");
//     Transport::Graph g("/home/arthur/LAAS/Data/Graphs/toulouse-mixed.dump");
    const Transport::Graph g("/home/arthur/LAAS/Data/Graphs/sud-ouest.dump");
//     const Transport::Graph g("/home/arthur/LAAS/mumoro/c01210f9d0f38553b4f5fd508c79be1a.dump");
    
    
    RLC::Graph rlc(&g, RLC::car_dfa());
    RLC::BackwardGraph back_rlc( &rlc );
    
    Area * toulouse = toulouse_area(&g);
    Area * bordeaux = bordeaux_area(&g);
    Area * small = small_area(&g);

    int source = 218238; //580025;
    int target = 329194;
    int landmark = 302296; //target;
    
    {
        typedef RLC::AspectCount<RLC::AspectTargetLandmark<RLC::DRegLC>> MyAlgo;
        
        Landmark * lm = create_landmark( &g, landmark );
        
        MyAlgo::ParamType p(
            DRegLCParams(&rlc, 10),
            AspectTargetLandmarkParams( target, lm )
        );
        
        MyAlgo algo( p );
        algo.insert_node(RLC::Vertice(source, 0), 0, 0);
        algo.run();
        
        cout << "Num vertices lm: " << algo.count <<" cost : "<< algo.cost(Vertice(target, 0))<<endl;
    }
    
    {
        typedef RLC::AspectCount<RLC::AspectTarget<RLC::DRegLC>> MyAlgo;
        MyAlgo::ParamType p(
            DRegLCParams(&rlc, 10),
            RLC::AspectTargetParams( target )
        );
        
        MyAlgo algo( p );
        algo.insert_node(RLC::Vertice(source, 0), 0, 0);
        algo.run();
        
        cout << "Num vertices : " << algo.count <<" cost : "<< algo.cost(Vertice(target, 0))<<endl;
    }

    /*
    {
        typedef RLC::AspectCount<RLC::AspectTargetArea<AspectTargetAreaLandmark<RLC::DRegLC>>> MyAlgo;

        MyAlgo::ParamType p(
            DRegLCParams(&back_rlc, 10),
            AspectTargetAreaLandmarkParams( small ),
            AspectTargetAreaParams( small )            
        );
        
        MyAlgo algo( p );
        algo.insert_node(RLC::Vertice(source, 0), 0, 0);
        algo.run();
        
        cout << "Num vertices lm: " << algo.count <<" cost : "<< algo.cost(Vertice(target, 0))<<endl;
    }
    
    {
        typedef RLC::AspectCount<RLC::AspectTargetArea<RLC::DRegLC>> MyAlgo;
        MyAlgo::ParamType p(
            DRegLCParams(&rlc, 10),
            RLC::AspectTargetAreaParams( small )
        );
        
        MyAlgo algo( p );
        algo.insert_node(RLC::Vertice(source, 0), 0, 0);
        algo.run();
        
        cout << "Num vertices : " << algo.count <<" cost : "<< algo.cost(Vertice(target, 0))<<endl;
    }
    
    */
    
    
}