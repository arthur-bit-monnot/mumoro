#ifndef NODES_FILTER_H
#define NODES_FILTER_H

#include "graph_wrapper.h"

class NodeFilter
{
public:
//     virtual ~NodeFilter() {}
    virtual bool isIn(int node) const = 0;
};

class BBNodeFilter : public NodeFilter
{
public:
    BBNodeFilter(Graph_t & g, float max_lon, float min_lon, float max_lat, float min_lat);
    bool isIn(int node) const;
private:
    Graph_t & g;
    const float max_lon;
    const float min_lon;
    const float max_lat;
    const float min_lat;
};

BBNodeFilter cap_jj_nf(Graph_t & g);




#endif