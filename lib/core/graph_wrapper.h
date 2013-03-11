/*    This file is part of Mumoro.

    Mumoro is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Mumoro is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Mumoro.  If not, see <http://www.gnu.org/licenses/>.

    © Université de Toulouse 1 2010
    Author: Tristram Gräbener*/

#include <boost/graph/adjacency_list.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

#include <boost/tuple/tuple.hpp>
#include <bitset>

#ifndef GRAPH_WRAPPER_H
#define GRAPH_WRAPPER_H

typedef enum { Foot, Bike, Car, PublicTransport } Mode;
typedef enum { FootEdge = 0, BikeEdge = 1, CarEdge = 2, SubwayEdge = 3, 
               BusEdge = 4, TramEdge = 5, TransferEdge = 6, UnknownEdgeType = 7, 
               WhateverEdge = 8 } EdgeMode;
typedef enum { ConstDur = 1, TimetableDur = 2, FrequencyDur = 3 } DurationType;
typedef std::bitset<128> Services;
typedef boost::tuple<float, float, Services> Time;

/**
 * Defines a frequency with the four following properties :
 * - begining of the period during which it is usable
 * - end of the period during which it is usable
 * - const duration of the transfer
 * - days on which it is usable
 */
typedef boost::tuple<int, int, int, Services> Frequency;

const char* edgeTypeToString(EdgeMode type);


namespace boost { namespace serialization {
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
}
} 

typedef enum { NextDay = 1, PrevDay = 2 } AllowedLookup;

class Duration
{
public:
    int const_duration;
    DurationType dur_type;
    std::vector<Time> timetable;
    std::vector<Frequency> frequencies;
public:
    Duration();
    Duration(float const_duration);
    void append_timetable(float start, float arrival, const std::string & services);
    void append_frequency(int start, int end, int duration, const std::string & services);
    void sort();
    std::pair<bool, int> operator()(float start_time, int day, bool backward = false) const;

    template<class Archive>
        void serialize(Archive& ar, const unsigned int version)
        {
            ar & dur_type & const_duration & timetable & frequencies;
        }

private: 
    std::pair<bool, int> freq_duration_forward(float start_time, int day, int allowed_lookup = NextDay | PrevDay ) const;
    std::pair<bool, int> freq_duration_backward(float start_time, int day, int allowed_lookup = NextDay | PrevDay ) const;
    std::pair<bool, int> tt_duration_forward(float start_time, int day, int allowed_lookup = NextDay | PrevDay ) const;
    std::pair<bool, int> tt_duration_backward(float start_time, int day, int allowed_lookup = NextDay | PrevDay ) const;
};

struct Node
{
    float lon;
    float lat;
    template<class Archive>
        void serialize(Archive& ar, const unsigned int version)
        {
            ar & lon & lat;
        }

};

struct Edge
{
    Edge();
    int edge_index;
    EdgeMode type;
    Duration duration;
            template<class Archive>
            void serialize(Archive& ar, const unsigned int version)
            {
                ar & edge_index & type & duration;
            }

};

typedef boost::adjacency_list<boost::listS, boost::vecS, boost::bidirectionalS, Node, Edge > Graph_t;
typedef boost::graph_traits<Graph_t>::edge_descriptor edge_t;
typedef std::list<int> EdgeList;
typedef std::list<int> NodeList;

void print_edge(edge_t e, Graph_t g);

namespace Transport {

struct Graph
{
    Graph_t g;
    Graph(int nb_nodes);
    Graph(const std::string & filename);
    void add_edge(int source, int target, const Edge & e);
    void set_coord(int node, float lon, float lat);
    bool public_transport_edge(int source, int target, DurationType dur_type, float start, float arrival, 
                                      int duration, const std::string & services, const EdgeMode type = UnknownEdgeType);
    bool dijkstra(int source, int target);
    void save(const std::string & filename) const;
    void load(const std::string & filename);
    void sort();
    
    /**
     * List all edges with type `type`
     */
    EdgeList listEdges(const EdgeMode type = WhateverEdge);
    
    /**
     * Return the Edge instance associated with the edge index passed
     */
    inline Edge mapEdge(const int edge_id) const { return g[this->edge_descriptor(edge_id)]; }
    
    inline int edgeIndex(const edge_t edge) const { return g[edge].edge_index; }
    
    /**
     * Return the Node instance associated with the node index passed
     */
    inline Node mapNode(const int node_id) const { return g[node_id]; }
    
    /**
     * Returns the origin (node index) of an edge
     */
    inline int sourceNode(const int edge_id) const { return source(this->edge_descriptor(edge_id), g); }
    
    /**
     * Returns the target (node index) of an edge
     */
    inline int targetNode(const int edge_id) const { return target(this->edge_descriptor(edge_id), g); }
  
private:
    std::vector<edge_t> edges_vec;
    inline edge_t edge_descriptor(const int edge_id) const { return edges_vec[edge_id]; }
    void initEdgeIndexes();
};

const int invalid_node = -1;

} // end namespace Transport

struct VisualResult
{
    VisualResult(Transport::Graph * g) : g(g) {}
    Transport::Graph *g;
    EdgeList edges;
    NodeList a_nodes;
    NodeList b_nodes;
    NodeList c_nodes;
};

#endif
