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

#include "debug/cwd_sys.h"

#include "graph_wrapper.h"
#include <iostream>
#include <fstream>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/adj_list_serialize.hpp>
#include <boost/foreach.hpp>
#include <boost/assert.hpp>

using namespace std;

const char* edgeTypeToString(EdgeMode type) {
    if(type == FootEdge)
        return "Foot";
    else if(type == BikeEdge)
        return "Bike";
    else if(type == CarEdge)
        return "Car";
    else if(type == BusEdge)
        return "Bus";
    else if(type == TramEdge)
        return "Tram";
    else if(type == SubwayEdge)
        return "Subway";
    else if(type == TransferEdge)
        return "--Transfer--";
    else
        return "------ Unknown -------";
}

Edge::Edge(const bool road_edge, const int index, const EdgeMode type) : road_edge(road_edge), index(index), type(type)
{
}



namespace Transport {

Graph::Graph(const std::string & filename)
{
    load(filename);
}

Graph::Graph(int nb_nodes) : g(nb_nodes)
{
}


void Graph::add_road_edge ( const int source, const int target, const EdgeMode type, const int duration )
{
    Edge e(true, num_road_edges, type);
    num_road_edges++;
    
    boost::add_edge(source, target, e, g);
    this->road_durations.push_back(duration);
    BOOST_ASSERT(this->road_durations[e.index] = duration);
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
    int count = 0;
    BOOST_FOREACH(edge_t e, boost::edges(g)) {
        count++;
        edges_vec[ edgeIndex(e) ] = e;
    }
    BOOST_ASSERT(count == (num_pt_edges + num_road_edges));
}


void Graph::load(const std::string & filename)
{
    std::cout << "Loading graph from file " << filename << std::endl;
    std::ifstream ifile(filename.c_str());
    boost::archive::binary_iarchive iArchive(ifile);
    iArchive >> id;
    iArchive >> num_road_edges;
    iArchive >> num_pt_edges;
    iArchive >> road_durations;
    iArchive >> pt_durations;
    iArchive >> g; //graph;   
    std::cout << "   " << boost::num_vertices(g) << " nodes" << std::endl;
    std::cout << "   " << boost::num_edges(g) << " edges" << std::endl;
    init_edge_indexes();
}

void Graph::save(const std::string & filename) const
{
    std::ofstream ofile(filename.c_str());
    boost::archive::binary_oarchive oArchive(ofile);
    oArchive << id;
    oArchive << num_road_edges;
    oArchive << num_pt_edges;
    oArchive << road_durations;
    oArchive << pt_durations;
    oArchive << g; //graph; 
}

EdgeList Graph::listEdges(const EdgeMode type) const
{
    EdgeList edgeList;
    
    BOOST_FOREACH(edge_t e, boost::edges(g)) {
        if(type == WhateverEdge || g[e].type == type)
            edgeList.push_back(g[e].index);
    }
    return edgeList;
}


} // end namespace Transport