#include "nodes_filter.h"

VisualResult NodeFilter::visualization() const
{
    VisualResult vres(g);
    
    Graph_t::vertex_iterator curr, end;
    boost::tie(curr, end) = boost::vertices( g->g );
    
    while(curr != end) {
        if( isIn( *curr ) ) {
            vres.a_nodes.push_back(*curr);
        }
        curr++;
    }
    
    return vres;
}


BBNodeFilter::BBNodeFilter ( const Transport::Graph * g, float max_lon, float min_lon, float max_lat, float min_lat ) :
NodeFilter( g ),
max_lon(max_lon), min_lon(min_lon), max_lat(max_lat), min_lat(min_lat)
{
}

bool BBNodeFilter::isIn ( int node ) const
{
    bool in = g->g[node].lon < max_lon 
        && g->g[node].lon > min_lon
        && g->g[node].lat < max_lat
        && g->g[node].lat > min_lat;
    return in;
}

BBNodeFilter * cap_jj_nf( const Transport::Graph * g)
{
    return new BBNodeFilter(g, 1.44940832773, 1.44468763987, 43.6059589047, 43.6040632995);
}

NodeSet::NodeSet ( const Transport::Graph * g ) :
NodeFilter( g ),
bitset(boost::num_vertices(g->g))
{
}

void NodeSet::addNode ( const int node )
{
    bitset[node] = 1;
}

bool NodeSet::isIn ( const int node ) const
{
    return bitset[node];
}

