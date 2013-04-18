#ifndef NODES_FILTER_H
#define NODES_FILTER_H

#include <boost/dynamic_bitset.hpp>

#include "graph_wrapper.h"

class NodeFilter
{
public:
    NodeFilter() {}
    virtual ~NodeFilter() {}
    
    virtual bool isIn( const int node ) const = 0;
    virtual VisualResult visualization() const = 0;
};

class BBNodeFilter : public NodeFilter
{
public:
    BBNodeFilter(const Transport::Graph * g, float max_lon, float min_lon, float max_lat, float min_lat);
    bool isIn( const int node ) const;
    virtual VisualResult visualization() const;
private:
    const Transport::Graph * g;
    const float max_lon;
    const float min_lon;
    const float max_lat;
    const float min_lat;
};

class NodeSet : public NodeFilter
{
public:
    NodeSet ( const int size );
    bool isIn ( const int node ) const;
    
    void addNode ( const int node );
    
    void add ( const boost::dynamic_bitset<> & to_merge );

    virtual VisualResult visualization() const;

    boost::dynamic_bitset<> bitset;
};

class AcceptAllFilter : public NodeFilter
{
public:
    AcceptAllFilter () {}
    virtual ~AcceptAllFilter () {}
    
    virtual bool isIn ( const int node ) const { return true; }
    virtual VisualResult visualization () const { return VisualResult(); }
};

BBNodeFilter * cap_jj_nf(const Transport::Graph * g);




#endif