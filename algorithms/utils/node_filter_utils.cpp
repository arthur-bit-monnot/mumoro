#include <boost/foreach.hpp>

#include "node_filter_utils.h"

#include "nodes_filter.h"
#include "AlgoTypedefs.h"

using namespace RLC;

NodeSet * isochrone ( RLC::AbstractGraph * g, const int center, const int max_time )
{
    typedef AspectMaxCostPruning<AspectMinCost<DRegLC> > Dij;
    NodeSet * ns = new NodeSet( g->transport );
    
    Dij::ParamType p(
        DRegLCParams( g, 0, 1 ),
        AspectMaxCostPruningParams( max_time )
    );
    
    Dij dij( p );
    BOOST_FOREACH( int state, g->dfa_start_states() ) {
        dij.insert_node( RLC::Vertice(center, state), 0, 0 );
    }
    while( !dij.finished() ) {
        RLC::Vertice vert = dij.treat_next();
        ns->addNode( vert.first );
    }
    
    return ns;
}

VisualResult show_isochrone ( RLC::AbstractGraph * g, const int center, const int max_time )
{
    NodeSet * ns = isochrone( g, center, max_time );
    return ns->visualization();
}



BBNodeFilter* rectangle_containing ( Transport::Graph* trans, const int node1, const int node2, const float margin )
{
    
    float lat1 = trans->latitude( node1 );
    float lon1 = trans->longitude( node1 );
    float lat2 = trans->latitude( node2 );
    float lon2 = trans->longitude( node2 );
    
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

