
#include "debug/cwd_sys.h"
#include "debug/cwd_debug.h"


#include "reglc_graph.h"

int main() 
{
    Debug( dc::notice.on() );             // Turn on the NOTICE Debug Channel.
    Debug( libcw_do.on() );               // Turn on the default Debug Object.
  
    Graph g("/home/arthur/LAAS/mumoro/3d12fc983d92949462bdc2c3c6a65670.dump");
    RLC::RegLCGraph rlc(g, RLC::foot_subway_dfa());
    
//     RLC::Dijkstra dij(&rlc, 382, 733, 50400, 233);
    RLC::Dijkstra dij(&rlc, 527, 58, 61500, 233);
    
    dij.run();
    EdgeList edges = dij.get_transport_path();
    EdgeList::iterator it;
    for(it = edges.begin(); it != edges.end() ; it++) {
        Edge e = g.mapEdge(*it);
        std::cout << "(" << g.sourceNode(*it) <<" "<<*it<<" "<<g.targetNode(*it) <<") "<<edgeTypeToString(e.type)<<" \n";
    }
    
    return 0;
}