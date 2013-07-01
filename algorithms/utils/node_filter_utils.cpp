#include <boost/foreach.hpp>

#include "node_filter_utils.h"

#include "nodes_filter.h"
#include "AlgoTypedefs.h"

using namespace RLC;

NodeSet * isochrone ( const RLC::AbstractGraph * g, const int center, const int max_time )
{
    typedef AspectMaxCostPruning<AspectMinCost<DRegLC> > Dij;
    NodeSet * ns = new NodeSet( g->transport->num_vertices() );
    
    Dij::ParamType p(
        DRegLCParams( g, 0, 1 ),
        AspectMaxCostPruningParams( max_time )
    );
    
    Dij dij( p );
    BOOST_FOREACH( int state, g->dfa_start_states() ) {
        dij.add_source_node( RLC::Vertice(center, state), 0, 0 );
    }
    while( !dij.finished() ) {
        RLC::Label lab = dij.treat_next();
        ns->addNode( lab.node.first );
    }
    
    return ns;
}

VisualResult show_isochrone ( const RLC::AbstractGraph * g, const int center, const int max_time )
{
    NodeSet * ns = isochrone( g, center, max_time );
    return ns->visualization();
}



BBNodeFilter* rectangle_containing ( const Transport::Graph* trans, const int node1, const int node2, const float margin )
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

BBNodeFilter* rectangle_containing ( const Transport::Graph* trans, const std::vector<int> nodes, const float margin )
{
    float max_lat = std::numeric_limits< float >::min();
    float min_lat = std::numeric_limits< float >::max();
    float max_lon = std::numeric_limits< float >::min();
    float min_lon = std::numeric_limits< float >::max();
    
    BOOST_FOREACH(int node, nodes) {
        float lat = trans->latitude( node );
        float lon = trans->longitude( node );
        
        if( lat > max_lat ) {
            max_lat = lat;
        } 
        if( lat < min_lat ) {
            min_lat = lat;
        }
        if( lon > max_lon ) {
            max_lon = lon;
        } 
        if( lon < min_lon ) {
            min_lon = lon;
        }
    }
    return new BBNodeFilter(trans, max_lon + margin, min_lon - margin, max_lat + margin, min_lat - margin);
}


BBNodeFilter * bordeaux_bb( const Transport::Graph * trans )
{
    return new BBNodeFilter(trans, -0.455148, -0.860322, 45.0317, 44.7266);
}

BBNodeFilter * toulouse_bb( const Transport::Graph * trans )
{
    return new BBNodeFilter(trans, 1.68514, 1.2078, 43.7794, 43.3935);
}


