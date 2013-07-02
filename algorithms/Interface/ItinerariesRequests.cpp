

#include "ItinerariesRequests.h"
#include <AspectStorePreds.h>
#include <AspectTarget.h>



Path point_to_point( const Transport::Graph * trans, const int source, const int dest, const int departure_time, RLC::DFA dfa )
{
    int day = 10;
    typedef RLC::AspectStorePreds<RLC::AspectTarget<RLC::DRegLC>> Algo;
    
    RLC::Graph g(trans, dfa);
    
    Algo::ParamType p( RLC::DRegLCParams(&g, day), RLC::AspectTargetParams(dest));
    
    Algo dij( p );
    dij.add_source_node( RLC::Vertice(source, dfa.start_state), departure_time, 0 );

    dij.run();
    
    Path res = dij.get_path_to(dest);
    
    return res;
}
    
    
    