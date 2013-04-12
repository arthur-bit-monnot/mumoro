#include <boost/foreach.hpp>

#include "debug/cwd_sys.h"
#include "debug/cwd_debug.h"


#include "path_algo.h"
#include "MultipleParticipants/muparo.h"
#include "MultipleParticipants/run_configurations.h"
#include "utils/node_filter_utils.h"

#include "RegLC/AspectTarget.h"

#include "MultipleParticipants/PotentialMeetingPoints.h"

using namespace RLC;

int main() 
{
    Debug( dc::notice.on() );             // Turn on the NOTICE Debug Channel.
    Debug( libcw_do.on() );               // Turn on the default Debug Object.
  
//     Transport::Graph g("/home/arthur/LAAS/mumoro/976c9329c82da079f78be26dddcf1174.dump");
//     Transport::Graph g("/home/arthur/LAAS/Data/Graphs/toulouse-mixed.dump");
    Transport::Graph g("/home/arthur/LAAS/Data/Graphs/midi-pyrennees.dump");
    
    
     RLC::Graph rlc(&g, RLC::foot_subway_dfa());
//     RLC::BackwardGraph back_rlc( &rlc );
    
//     MuPaRo::Muparo mpr(&g, 50, 600);
//     mpr.run();
//     MuPaRo::Muparo * mup = MuPaRo::bi_point_to_point(&g, 223, 3);
    
//     MuPaRo::Muparo * mup = MuPaRo::covoiturage(&g, 713, 425, 306, 298,  RLC::foot_dfa(), RLC::foot_dfa());
//     mup->run();
//     delete mup;
    
//     NodeFilter * nf = isochrone(&g, RLC::foot_dfa(), 713, 120);
//     VisualResult vres = nf->visualization();
//     BOOST_FOREACH( int n, vres.a_nodes ) {
//         std::cout << "node : "<<n<<endl;
//     }
     
//      RLC::DRegLC * dreg = RLC::dreg_with_target(&rlc, 425, 306);
//      AlgoMPR::PtToPt *mup =  MuPaRo::point_to_point( &g, 254, 900 );
//      AlgoMPR::SharedPath * mup = MuPaRo::shared_path( &g, 254, 425, 900 );
     AlgoMPR::CarSharing * mup = car_sharing( &g, 713, 425, 306, 298,  foot_dfa(), foot_dfa() );
//      
     mup->run();
//      
//      
//      
     mup->build_result();
     mup->get_result();
     delete mup;
//      cout << "jdklfjdskfjqdslfjdqsf\n\n";
//      show_car_sharing( &g, 713, 425, 306, 298,  foot_dfa(), foot_dfa() );
     
//      show_meeting_points(&g, 425);
}