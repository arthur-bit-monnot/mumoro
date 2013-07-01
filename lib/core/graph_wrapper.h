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
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/dynamic_bitset.hpp>

#include <bitset>

#ifndef GRAPH_WRAPPER_H
#define GRAPH_WRAPPER_H

using std::vector;

typedef enum { FootEdge = 0, BikeEdge = 1, CarEdge = 2, SubwayEdge = 3, 
               BusEdge = 4, TramEdge = 5, TransferEdge = 6, UnknownEdgeType = 7, 
               WhateverEdge = 8 } EdgeMode;

/**
 * Type of the duration object attached to an edge 
 */
typedef enum { 
    ConstDur = 1, // traversing the edge takes a constant duration
    TimetableDur = 2, // duration is defined as timetable (ex: bus leaves at 5:00 and arrives at 5:06
    FrequencyDur = 3 // duration is defined as frequency (ex: between 5:00 and 6:00, one bus every two minutes)
} DurationType;


/**
 * Attaches a boolean to every day
 */
typedef std::bitset<128> Services;

/**
 * Defines a timetable line:
 *  - float: departure
 *  - float: arrival
 *  - Services: days of availability
 */
typedef boost::tuple<float, float, Services> Time;

/**
 * Defines a frequency with the four following properties :
 * - begining of the period during which it is usable
 * - end of the period during which it is usable
 * - const duration of the transfer
 * - days on which it is usable
 */
typedef boost::tuple<int, int, int, Services> Frequency;


typedef enum { NextDay = 1, PrevDay = 2 } AllowedLookup;

class DurationPT
{
private:
    std::vector<Time> timetable;
    std::vector<Frequency> frequencies;
    
public:
    int const_duration;
    DurationType dur_type;
    
    DurationPT(const DurationType dur_type = ConstDur);
    DurationPT(float const_duration);
    void append_timetable(float start, float arrival, const std::string & services);
    void append_frequency(int start, int end, int duration, const std::string & services);
    void sort();
    void set_min();
    std::pair<bool, int> operator()(float start_time, int day, bool backward = false) const;
    
    /**
     * Returns the minimum cost that might appear on this edge.
     * This is tipically useful for backward search when no arrival time is known
     */
    std::pair<bool, int> min_duration() const;
    

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
    Edge() : index(-1) {}
    Edge(const bool road_edge, const int index, const EdgeMode type);
    bool road_edge;
    int index;
    EdgeMode type;
    
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar & road_edge & index & type;
    }   
};

/**
 * Implementation of the graph.
 * 
 * Using vectors (and not lists) is important for cache performance reasons since there is less 
 * memory overhead and items are stacked.
 */
typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, Node, Edge > Graph_t;

typedef boost::graph_traits<Graph_t>::edge_descriptor edge_t;
typedef std::list<int> EdgeList;
typedef std::list<int> NodeList;

namespace Transport {

struct Graph
{
    friend class GraphFactory;
public:
    Graph_t g;

private:
    /**
     * Initializes the graph with a given number of nodes
     */
    Graph(int nb_nodes);
    
    /**
     * Loads the graph from the archive
     */
    Graph(const std::string & filename);
    
    /**
     * Insert an edge from *source* to *target* in the graph
     */
    void add_road_edge(const int source, const int target, const EdgeMode type, const int duration);
    
    /**
     * Adds public transport information to the given edge
     */
    bool add_public_transport_edge(int source, int target, DurationType dur_type, float start, float arrival, 
                                      int duration, const std::string & services, const EdgeMode type);
    bool add_public_transport_edge(int source, int target, const int duration, const EdgeMode type);
    
    /**
     * Sets longitude and latitude to the node
     */
    void set_coord(int node, float lon, float lat);
    void set_car_accessible( const int node ) { car_accessibility.set( node ); }
    
    void set_id(const std::string id) { this->id = id; }
    
    /**
     * Saves the whole graph to a file
     */
    void save(const std::string & filename) const;
    
    /**
     * Loads graph from a file
     */
    void load(const std::string & filename);

    
    /**
     * Performs operations that need to be done once all information is in the graph :
     * 
     * - sorts timetables
     * - builds an edge index
     * - computes min duration for each edge
     */
    void preprocess();

public:
    std::string get_id() const { return id; }
    
    /**
     * List all edges with type `type`
     */
    EdgeList listEdges(const EdgeMode type = WhateverEdge) const;
    
    /**
     * Returns the number of vertices in the graph
     */
    inline int num_vertices() const { return boost::num_vertices( g ); }
    
    /**
     * Return the Edge instance associated with the edge index passed
     */
    inline Edge map(const int edge_id) const { return g[this->edge_descriptor(edge_id)]; }
    inline Edge map( const edge_t edge ) const { return g[edge]; }
    inline int edgeIndex(const edge_t edge) const { 
        if(g[edge].road_edge) 
            return g[edge].index; 
        else 
            return num_road_edges + g[edge].index; }
    
    inline std::pair<bool, int> duration_forward(const edge_t edge, const float start_sec, const int day) const {
        if(g[edge].road_edge) {
            return std::pair<bool, int>(true, road_durations[g[edge].index]);
        } else {
            return pt_durations[g[edge].index](start_sec, day, false);
        }
    }
    
    inline std::pair<bool, int> duration_backward(const edge_t edge, const float start_sec, const int day) const {
        if(g[edge].road_edge) {
            return std::pair<bool, int>(true, road_durations[g[edge].index]);
        } else {
            return pt_durations[g[edge].index](start_sec, day, true);
        }
    }
    
    inline std::pair<bool, int> min_duration(const edge_t edge) const {
        if(g[edge].road_edge) {
            return std::pair<bool, int>(true, road_durations[g[edge].index]);
        } else {
            return pt_durations[g[edge].index].min_duration();
        }
    }
    
    /**
     * Return the Node instance associated with the node index passed
     */
    inline Node mapNode(const int node_id) const { return g[node_id]; }
    
    /**
     * Returns the origin (node index) of an edge
     */
    inline int source(const int edge_id) const { return boost::source(this->edge_descriptor(edge_id), g); }
    inline int source(const edge_t edge) const { return boost::source(edge, g); }
    
    /**
     * Returns the target (node index) of an edge
     */
    inline int target(const int edge_id) const { return boost::target(this->edge_descriptor(edge_id), g); }
    inline int target(const edge_t edge) const { return boost::target(edge, g); }
    
    /**
     * Returns the longitude of a node
     */
    inline float longitude(const int node) const { return g[node].lon; }
    
    /**
     * Returns the latitude of a node
     */
    inline float latitude(const int node) const { return g[node].lat; }
    
    inline bool car_accessible(const int node) const { return car_accessibility.test(node); }
  
private:
    std::string id;
    int num_road_edges = 0;
    int num_pt_edges = 0;
    
    vector<int> road_durations;
    vector<DurationPT> pt_durations;
    
    std::vector<edge_t> edges_vec;
    boost::dynamic_bitset<> car_accessibility;
    inline edge_t edge_descriptor(const int edge_id) const { return edges_vec[edge_id]; }
    
    void compute_min_durations();
    void sort();
    void init_edge_indexes();
};

} // end namespace Transport

struct VisualResult
{
    EdgeList edges;
    NodeList a_nodes;
    NodeList b_nodes;
    NodeList c_nodes;
};

#endif
