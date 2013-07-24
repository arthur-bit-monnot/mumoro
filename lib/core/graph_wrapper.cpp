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

#include "graph_wrapper.h"
#include <iostream>
#include <fstream>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/adj_list_serialize.hpp>
#include <boost/foreach.hpp>
#include <boost/assert.hpp>

using namespace std;

namespace boost
{
    namespace serialization
    {
        template <class Archive>
        void save(Archive &ar, const Time &t, const unsigned int version)
        {
            ar << boost::get<0>(t);
            ar << boost::get<1>(t);
            std::string s = boost::get<2>(t).to_string();
            ar << s;
        }

        template <class Archive>
        void load(Archive &ar, Time &t, const unsigned int version)
        {
            ar >> boost::get<0>(t);
            ar >> boost::get<1>(t);
            std::string s;
            ar >> s;
            boost::get<2>(t) = Services(s);
        }

        template <class Archive>
        void serialize(Archive &ar, Time &t, const unsigned int version)
        {
                boost::serialization::split_free(ar, t, version);
        }

        template <class Archive>
        void save(Archive &ar, const Frequency &t, const unsigned int version)
        {
            ar << boost::get<0>(t);
            ar << boost::get<1>(t);
            ar << boost::get<2>(t);
            std::string s = boost::get<3>(t).to_string();
            ar << s;
        }

        template <class Archive>
        void load(Archive &ar, Frequency &t, const unsigned int version)
        {
            ar >> boost::get<0>(t);
            ar >> boost::get<1>(t);
            ar >> boost::get<2>(t);
            std::string s;
            ar >> s;
            boost::get<3>(t) = Services(s);
        }

        template <class Archive>
        void serialize(Archive &ar, Frequency &t, const unsigned int version)
        {
                boost::serialization::split_free(ar, t, version);
        }

        template < class Archive , typename Block , typename Allocator >
        inline void save( Archive & ar , boost::dynamic_bitset< Block , Allocator > const & t , const unsigned int /* version */ )
        {
                // Serialize bitset size
                std::size_t size = t.size();
                ar << size;

                // Convert bitset into a vector
                std::vector< Block > v( t.num_blocks() );
                to_block_range( t, v.begin() );

                // Serialize vector
                ar & v;
        }

        template < class Archive , typename Block , typename Allocator >
        inline void load( Archive & ar, boost::dynamic_bitset< Block , Allocator > & t, const unsigned int /* version */ )
        {
                std::size_t size;
                ar & size;
                t.resize( size );

                // Load vector
                std::vector< Block > v;
                ar & v;

                // Convert vector into a bitset
                boost::from_block_range( v.begin() , v.end() , t );
        }

        template <class Archive, typename Block, typename Allocator>
        inline void serialize( Archive & ar, boost::dynamic_bitset<Block, Allocator> & t, const unsigned int version )
        {
                boost::serialization::split_free( ar, t, version );
        }

    }   //  namespace serialization
}   //  namespace boost



Edge::Edge(const bool road_edge, const int index, const EdgeMode type) : road_edge(road_edge), index(index), type(type)
{
}

namespace Transport {

Graph::Graph(const std::string & filename, bool from_bin)
{
    if(from_bin)
        load_from_bin(filename);
    else
        load_from_txt(filename);
    
}

Graph::Graph(int nb_nodes) : g(nb_nodes), car_accessibility(nb_nodes)
{
}


void Graph::add_road_edge ( const int source, const int target, const EdgeMode type, const int duration )
{
    
    Edge e(true, num_road_edges, type);
    num_road_edges++;
    
    boost::add_edge(source, target, e, g);
    this->road_durations.push_back(duration);
    BOOST_ASSERT(this->road_durations[e.index] == duration);
}

void Graph::set_coord(int node, float lon, float lat)
{
    g[node].lon = lon;
    g[node].lat = lat;
}

bool Graph::add_public_transport_edge ( int source, int target, DurationType dur_type, float start, float arrival, int duration, const string& services, const EdgeMode type )
{
    edge_t e;
    bool b;
    tie(e, b) = boost::edge(source, target, g);
    if(!b)
    {
        Edge edge(false, num_pt_edges, type);
        num_pt_edges++;
        bool c;
        tie(e, c) = boost::add_edge(source, target, edge, g);
        this->pt_durations.push_back(DurationPT(dur_type));
        
        BOOST_ASSERT( this->pt_durations[edge.index].dur_type ==  dur_type );
    }
    int index = g[e].index;
    
    if(dur_type == TimetableDur)
        pt_durations[index].append_timetable(start, arrival, services);
    else if(dur_type == FrequencyDur)
        pt_durations[index].append_frequency(start, arrival, duration, services);
    else if(dur_type == ConstDur)
        pt_durations[index].const_duration = duration;
    
    return !b;
}

bool Graph::add_public_transport_edge ( int source, int target, const int duration, const EdgeMode type )
{
    return add_public_transport_edge(source, target, ConstDur, 0, 0, duration, "", type);
}





void Graph::preprocess()
{
    sort();
    init_edge_indexes();
    compute_min_durations();
}
   
void Graph::sort()
{
    for(uint i=0 ; i<pt_durations.size() ; ++i) {
        pt_durations[i].sort();
    }
}

void Graph::compute_min_durations()
{
    for(uint i=0 ; i<pt_durations.size() ; ++i) {
        pt_durations[i].set_min();
    }
}

void Graph::init_edge_indexes()
{
    edges_vec.resize(num_pt_edges + num_road_edges);

    BOOST_FOREACH(edge_t e, boost::edges(g)) {
        edges_vec[ edgeIndex(e) ] = e;
    }
}

void Graph::load_from_bin(const std::string & filename)
{
    std::cout << "Loading graph from file " << filename << std::endl;
    std::ifstream ifile(filename.c_str());
    boost::archive::binary_iarchive iArchive(ifile);
    iArchive >> id;
    iArchive >> num_road_edges;
    iArchive >> num_pt_edges;
    iArchive >> road_durations;
    iArchive >> pt_durations;
    iArchive >> car_accessibility;
    iArchive >> g; //graph;   
    std::cout << "   " << boost::num_vertices(g) << " nodes" << std::endl;
    std::cout << "   " << boost::num_edges(g) << " edges" << std::endl;
    init_edge_indexes();
}

void Graph::save_to_bin(const std::string & filename) const
{
    std::ofstream ofile(filename.c_str());
    boost::archive::binary_oarchive oArchive(ofile);
    oArchive << id;
    oArchive << num_road_edges;
    oArchive << num_pt_edges;
    oArchive << road_durations;
    oArchive << pt_durations;
    oArchive << car_accessibility;
    oArchive << g; //graph; 
}

void Graph::load_from_txt(const std::string & filename)
{
    std::cout << "Loading graph from file " << filename << std::endl;
    std::ifstream ifile(filename.c_str());
    boost::archive::text_iarchive iArchive(ifile);
    iArchive >> id;
    iArchive >> num_road_edges;
    iArchive >> num_pt_edges;
    iArchive >> road_durations;
    iArchive >> pt_durations;
    iArchive >> car_accessibility;
    iArchive >> g; //graph;   
    std::cout << "   " << boost::num_vertices(g) << " nodes" << std::endl;
    std::cout << "   " << boost::num_edges(g) << " edges" << std::endl;
    init_edge_indexes();
}

void Graph::save_to_txt(const std::string & filename) const
{
    std::ofstream ofile(filename.c_str());
    boost::archive::text_oarchive oArchive(ofile);
    oArchive << id;
    oArchive << num_road_edges;
    oArchive << num_pt_edges;
    oArchive << road_durations;
    oArchive << pt_durations;
    oArchive << car_accessibility;
    oArchive << g; //graph; 
}

EdgeList Graph::listEdges(const EdgeMode type) const
{
    EdgeList edgeList;
    
    BOOST_FOREACH(edge_t e, boost::edges(g)) {
        if(type == WhateverEdge || g[e].type == type)
            edgeList.push_back(edgeIndex(e));
    }
    return edgeList;
}


} // end namespace Transport