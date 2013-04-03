#include <boost/foreach.hpp>

#include "node_filter_utils.h"

#include "nodes_filter.h"
#include "reglc_dijkstra.h"

NodeSet* isochrone ( Transport::Graph * trans, RLC::DFA dfa, const int center, const int max_time )
{
    NodeSet * ns = new NodeSet(trans);
    
    RLC::Graph forward(trans, dfa);
    RLC::BackwardGraph backward(&forward);
    
    RLC::DijkstraParameters * params =  new RLC::DijkstraParameters();
    params->save_touched_nodes = true;
    params->cost_limit = true;
    params->cost_limit_value = max_time;
    params->use_cost_lower_bounds = true;
    
    RLC::Dijkstra dij(&backward, center, -1, -1, -1, params);
    dij.run();
    
    BOOST_FOREACH( const RLC::Vertice vert, dij.touched_nodes ) {
        ns->addNode( vert.first );
    }
    
    return ns;
}


BBNodeFilter* rectangle_containing ( Transport::Graph* trans, const int node1, const int node2, const float margin )
{
    float lat1 = trans->g[node1].lat;
    float lon1 = trans->g[node1].lon;
    float lat2 = trans->g[node2].lat;
    float lon2 = trans->g[node2].lon;
    
    float max_lat, min_lat, max_lon, min_lon;
    if(lat1 > lat2) {
        max_lat = lat1 + margin;
        min_lat = lat2 - margin;
    } else {
        max_lat = lat2 + margin;
        min_lat = lat1 - margin;
    }
    
    if(lon1 > lon2) {
        max_lon = lon1 + margin;
        min_lon = lon2 - margin;
    } else {
        max_lon = lon2 + margin;
        min_lon = lon1 - margin;
    }
    
    return new BBNodeFilter(trans, max_lon, min_lon, max_lat, min_lat);
}

