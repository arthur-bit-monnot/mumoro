/* Copyright : Université Toulouse 1 (2010)

Contributors : 
Tristram Gräbener
Odysseas Gabrielides
Arthur Bit-Monnot (arthur.bit-monnot@laas.fr)

This software is a computer program whose purpose is to [describe
functionalities and technical features of your software].

This software is governed by the CeCILL-B license under French law and
abiding by the rules of distribution of free software.  You can  use, 
modify and/ or redistribute the software under the terms of the CeCILL-B
license as circulated by CEA, CNRS and INRIA at the following URL
"http://www.cecill.info". 

As a counterpart to the access to the source code and  rights to copy,
modify and redistribute granted by the license, users are provided only
with a limited warranty  and the software's author,  the holder of the
economic rights,  and the successive licensors  have only  limited
liability. 

In this respect, the user's attention is drawn to the risks associated
with loading,  using,  modifying and/or developing or reproducing the
software by the user in light of its specific status of free software,
that may mean  that it is complicated to manipulate,  and  that  also
therefore means  that it is reserved for developers  and  experienced
professionals having in-depth computer knowledge. Users are therefore
encouraged to load and test the software's suitability as regards their
requirements in conditions enabling the security of their systems and/or 
data to be ensured and,  more generally, to use and operate it in the 
same conditions as regards security. 

The fact that you are presently reading this means that you have had
knowledge of the CeCILL-B license and that you accept its terms. */

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