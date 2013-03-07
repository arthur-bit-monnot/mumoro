#include "nodes_filter.h"



BBNodeFilter::BBNodeFilter ( Graph_t & g, float max_lon, float min_lon, float max_lat, float min_lat ) :
g(g), max_lon(max_lon), min_lon(min_lon), max_lat(max_lat), min_lat(min_lat)
{
}

bool BBNodeFilter::isIn ( int node ) const
{
    bool in = g[node].lon < max_lon 
        && g[node].lon > min_lon
        && g[node].lat < max_lat
        && g[node].lat > min_lat;
    if(in)
        std::cout << "Yes " <<g[node].lon << " " << g[node].lat <<std::endl;
    else
        std::cout << "No  " <<g[node].lon << " " << g[node].lat <<std::endl;
    return in;
}

BBNodeFilter cap_jj_nf(Graph_t & g)
{
    return BBNodeFilter(g, 1.44940832773, 1.44468763987, 43.6059589047, 43.6040632995);
}