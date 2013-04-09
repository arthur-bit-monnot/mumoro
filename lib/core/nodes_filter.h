#ifndef NODES_FILTER_H
#define NODES_FILTER_H

#include <boost/dynamic_bitset.hpp>

#include "graph_wrapper.h"

class NodeFilter
{
public:
    NodeFilter(const Transport::Graph * g) : g(g) {}
    virtual ~NodeFilter() {}
    virtual bool isIn( const int node ) const = 0;
    const Transport::Graph * g;
    
    VisualResult visualization() const;
};

class BBNodeFilter : public NodeFilter
{
public:
    BBNodeFilter(const Transport::Graph * g, float max_lon, float min_lon, float max_lat, float min_lat);
    bool isIn( const int node ) const;
private:
    const float max_lon;
    const float min_lon;
    const float max_lat;
    const float min_lat;
};

class NodeSet : public NodeFilter
{
public:
    NodeSet(const Transport::Graph * g);
    bool isIn(const int node) const;
    
    void addNode( const int node );

private:
    boost::dynamic_bitset<> bitset;
};

BBNodeFilter * cap_jj_nf(const Transport::Graph * g);




#endif