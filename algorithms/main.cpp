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

#include "MultipleParticipants/PotentialMeetingPoints.h"
#include "DataStructures/GraphFactory.h"

using namespace RLC;

int main() 
{
    Debug( dc::notice.on() );             // Turn on the NOTICE Debug Channel.
    Debug( libcw_do.on() );               // Turn on the default Debug Object.
    std::string file( "/home/arthur/LAAS/Data/Graphs/sud-ouest.dump" );
    Transport::GraphFactory gf( file );
    
    const Transport::Graph * g = gf.get();
    
    RLC::Graph rlc(g, RLC::car_dfa());
    RLC::BackwardGraph back_rlc( &rlc );
    
    Area * toulouse = toulouse_area(g);
    Area * bordeaux = bordeaux_area(g);
    Area * small = small_area(g);

    int source = 218238; //580025;
    int target = 329194;
    int landmark = source;
    int landmark2 = 302296; //target;
    
    {
        typedef RLC::AspectCount<RLC::AspectTargetLandmark<RLC::DRegLC, RLC::LandmarkSet>> MyAlgo;
        
        std::list<const Landmark *> lms;
        lms.push_back( create_car_landmark( g, landmark ) );
        lms.push_back( create_car_landmark( g, landmark2 ) );
        lms.push_back( create_car_landmark( g, target ) );
        LandmarkSet lmset(lms);
        
        MyAlgo::ParamType p(
            DRegLCParams(&rlc, 10),
            AspectTargetLandmarkParams<RLC::LandmarkSet>( target, &lmset )
        );
        
        MyAlgo algo( p );
        algo.insert_node(RLC::Vertice(source, 0), 0, 0);
        algo.run();
        
        cout << "Num vertices LandmarkSet: " << algo.count <<" cost : "<< algo.cost(Vertice(target, 0))<<endl;
    }
    
    {
        typedef RLC::AspectCount<RLC::AspectTargetLandmark<RLC::DRegLC>> MyAlgo;
        
        
        Landmark * lm = create_car_landmark( g, landmark ) ;
        
        MyAlgo::ParamType p(
            DRegLCParams(&rlc, 10),
            AspectTargetLandmarkParams<RLC::Landmark>( target, lm )
        );
        
        MyAlgo algo( p );
        algo.insert_node(RLC::Vertice(source, 0), 0, 0);
        algo.run();
        
        cout << "Num vertices Landmark: " << algo.count <<" cost : "<< algo.cost(Vertice(target, 0))<<endl;
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