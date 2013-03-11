
#include "debug/cwd_sys.h"
#include "debug/cwd_debug.h"


#include "reglc_graph.h"
#include "path_algo.h"
#include "muparo.h"

int main() 
{
    Debug( dc::notice.on() );             // Turn on the NOTICE Debug Channel.
    Debug( libcw_do.on() );               // Turn on the default Debug Object.
  
//     Transport::Graph g("/home/arthur/LAAS/mumoro/976c9329c82da079f78be26dddcf1174.dump");
//     Transport::Graph g("/home/arthur/LAAS/Data/Graphs/toulouse.dump");
    Transport::Graph g("/home/arthur/LAAS/Data/Graphs/midi-pyrennees.dump");
    
    /*
    RLC::Graph rlc(&g, RLC::foot_subway_dfa());
    
    RLC::BackwardGraph back_rlc( &rlc );
    
//     RLC::Dijkstra dij(&rlc, 382, 733, 50400, 233);
    float start_secs = 12000.0f;
    float start_day = 20;
    RLC::Dijkstra dij(&rlc, 609, 87, start_secs, start_day);
    
    // Forward search
    if( dij.run() ) 
    {
        EdgeList edges = dij.get_transport_path();
        EdgeList::iterator it;
        for(it = edges.begin(); it != edges.end() ; it++) {
            Edge e = g.mapEdge(*it);
            std::cout << "(" << g.sourceNode(*it) <<" "<<*it<<" "<<g.targetNode(*it) <<") "<<edgeTypeToString(e.type)<<" \n";
        }
        
        std::cout << "\nBackward search\n\n";
        RLC::Dijkstra back_dij(&back_rlc, 87, 609, dij.path_arrival, start_day);
        back_dij.run();
    } 
    else 
    {
        std::cout << "No path found\n";
    }
    return 0;
    */
    
//      134532 to node 319179o
     
     //MeetPoint mp(312348, 295483, 5000, 10, &g);  mp.run();
     // A = 109256  B = 109246   C = 259542
     
//      SharedPath sp(109256, 109246, 259542, 40080, 27, &g);
//      sp.run();

    MuPaRo::Muparo mpr(&g, 12000, 20000);
    mpr.run();
}