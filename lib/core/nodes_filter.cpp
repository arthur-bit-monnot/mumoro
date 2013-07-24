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

#include "nodes_filter.h"

BBNodeFilter::BBNodeFilter ( const Transport::Graph * g, float max_lon, float min_lon, float max_lat, float min_lat ) :
g(g), max_lon(max_lon), min_lon(min_lon), max_lat(max_lat), min_lat(min_lat)
{
}

bool BBNodeFilter::isIn ( int node ) const
{
    bool in = g->longitude(node) < max_lon 
        && g->longitude(node) > min_lon
        && g->latitude(node) < max_lat
        && g->latitude(node) > min_lat;
    return in;
}

BBNodeFilter * cap_jj_nf( const Transport::Graph * g)
{
    return new BBNodeFilter(g, 1.44940832773, 1.44468763987, 43.6059589047, 43.6040632995);
}

VisualResult BBNodeFilter::visualization() const
{
    VisualResult vres;
    
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


NodeSet::NodeSet ( const int size ) :
bitset( size )
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

VisualResult NodeSet::visualization() const
{
    VisualResult vres;
    
    for(uint i=0 ; i<bitset.size() ; ++i) {
        if( isIn( i ) ) {
            vres.a_nodes.push_back(i);
        }
    }
    
    return vres;
}

void NodeSet::add ( const boost::dynamic_bitset< long unsigned int > & to_merge )
{
    bitset |= to_merge;
}



