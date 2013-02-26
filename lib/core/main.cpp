
#include "debug/cwd_sys.h"
#include "debug/cwd_debug.h"


#include "reglc_graph.h"

int main() 
{
    Debug( dc::notice.on() );             // Turn on the NOTICE Debug Channel.
    Debug( libcw_do.on() );               // Turn on the default Debug Object.
  
    Transport::Graph g("/home/arthur/LAAS/mumoro/797546be36d0b96affbcf4400f6a62aa.dump");
//     Transport::Graph g("/home/arthur/LAAS/Data/Graphs/toulouse.dump");
//     Transport::Graph g("/home/arthur/LAAS/Data/Graphs/midi-pyrennees.dump");
    std::cerr << "ICICICICI\n";
    RLC::Graph rlc(&g, RLC::pt_foot_dfa());// RLC::foot_subway_dfa());
    std::cerr << "Here\n";
    
//     RLC::Dijkstra dij(&rlc, 382, 733, 50400, 233);
    RLC::Dijkstra dij(&rlc, 710, 321, 50520.0f, 17);
    
    if( dij.run() ) 
    {
        EdgeList edges = dij.get_transport_path();
        EdgeList::iterator it;
        for(it = edges.begin(); it != edges.end() ; it++) {
            Edge e = g.mapEdge(*it);
            std::cout << "(" << g.sourceNode(*it) <<" "<<*it<<" "<<g.targetNode(*it) <<") "<<edgeTypeToString(e.type)<<" \n";
        }
    } 
    else 
    {
        std::cout << "No path found\n";
    }
    return 0;
}