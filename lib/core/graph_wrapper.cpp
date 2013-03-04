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

void print_edge(edge_t e, Graph_t g)
{
    cout << "("<< source(e, g) <<", "<< target(e, g) <<", "<<g[e].type<<", "<<g[e].duration(0,0).second<<")   ";
}

Edge::Edge() : edge_index(-1), type(UnknownEdgeType)
{
}

Duration::Duration(float d) : const_duration(d), dur_type(ConstDur) { }

Duration::Duration() : const_duration(-1) {}

void Duration::append_timetable(float start, float arrival, const std::string & services)
{
    BOOST_ASSERT(start < arrival);
    BOOST_ASSERT(dur_type == TimetableDur);
    timetable.push_back(Time(start, arrival, Services(services)));
}

void Duration::append_frequency(int start, int end, int duration, const std::string & services)
{
    BOOST_ASSERT(start < end);
    BOOST_ASSERT(dur_type == FrequencyDur);
    frequencies.push_back(Frequency(start, end, duration, Services(services)));
}

bool compare_times(const Time & a, const Time & b)
{
    return get<0>(a) < get<0>(b);
}

bool compare_frequencies(const Frequency & a, const Frequency & b)
{
    return get<0>(a) < get<0>(b);
}

void Duration::sort()
{
    if(dur_type == TimetableDur)
    {
        std::sort(timetable.begin(), timetable.end(), compare_times);
    }
    else if(dur_type == FrequencyDur)
    {
        std::sort(frequencies.begin(), frequencies.end(), compare_frequencies);
    }
}

std::pair<bool, int> Duration::operator()(float start, int day, bool backward, int allowed_lookups) const
{
    bool has_traffic = false;
    
    if(backward) {
        float ret = -1;
        
        if (const_duration >= 0) 
            return std::make_pair<bool, int>(true, start - const_duration);
        else if(dur_type == FrequencyDur)
            return std::make_pair<bool, int>(false, start - const_duration);
        else 
        {
            int min = 0;
            int max = timetable.size()-1;
            int i = (max + min) /2;
            
            float tt_start, tt_arrival;
            Services s;
            boost::tie(tt_start, tt_arrival, s) = timetable[i];
                
            while(min != max) 
            {
                if(!(tt_arrival <= start)) 
                {
                    min = min;
                    max = i;
                    i = (max + min) /2;   
                    BOOST_ASSERT(i < (int) timetable.size());
                    boost::tie(tt_start, tt_arrival, s) = timetable[i];
                }
                else 
                {
                    BOOST_ASSERT(i < (int) timetable.size());
                    if(i==max) 
                    {
                        break;
                    }
                    else if(timetable[i+1].get<1>() > start)
                    {
                        break;
                    }
                    else
                    {
                        min = i+1;
                        max = max;
                        i = (min+max)/2;
                        BOOST_ASSERT(i < (int) timetable.size());
                        boost::tie(tt_start, tt_arrival, s) = timetable[i];
                    }
                }
            }
            if(tt_arrival > start)
                ret = -1;
            else
            {
                while(i >= 0) {
                    BOOST_ASSERT(i < (int) timetable.size());
                    boost::tie(tt_start, tt_arrival, s) = timetable[i];
                    if(s[day]) 
                    {
                        has_traffic = true;
                        break;
                    }
                    --i;
                }
                if(!has_traffic)
                    ret = -1;
                else 
                    ret = tt_start;
            }
            
            //there might be trips on the previous day for first few hours of the current day
            if(start < 2*3600 && allowed_lookups & PrevDay)
            {
                std::pair<bool, int> prev_day;
                prev_day = (*this)(start + 24*3600, day-1, backward, PrevDay);
                prev_day.second -= 24*3600;
                if((prev_day.first && !has_traffic) || (prev_day.first && prev_day.second > ret)) {
//                     cout << "Prev day better\n";
                    return prev_day;
                }
            }
            
            // Start is on the next day, we'd better lookup in this one too
            if(start >= 24*3600 && allowed_lookups & NextDay)
            {   
                std::pair<bool, int> next_day;
                next_day = (*this)(start - 24*3600, day+1, backward, NextDay);
                next_day.second += 24*3600;
                cerr<<"a  "<<next_day.first<<" "<<next_day.second<<" "<<has_traffic<<" "<<ret<<endl;
                
                if((next_day.first && !has_traffic) || (next_day.first && next_day.second > ret)) {
//                     cout << "Next day better\n";    
                    return next_day;
                }
            }

            return std::make_pair<bool, int>(has_traffic, ret);
        }
    }
    else /** forward **/
    {
        float ret = -1;
        
        if (const_duration >= 0) 
            return std::make_pair<bool, int>(true, start + const_duration);
        else if(dur_type == FrequencyDur)
            return std::make_pair<bool, int>(false, start - const_duration);
        else 
        {
            uint min = 0;
            uint max = timetable.size()-1;
            uint i = (max + min) /2;
            
            float tt_start, tt_arrival;
            Services s;
            boost::tie(tt_start, tt_arrival, s) = timetable[i];
                
            while(min != max) 
            {
                if(!(tt_start >= start)) 
                {
                    min = i+1;
                    max = max;
                    i = (max + min) /2;   
                    boost::tie(tt_start, tt_arrival, s) = timetable[i];
                }
                else 
                {
                    if(i==min) 
                    {
                        break;
                    }
                    else if(timetable[i-1].get<0>() < start)
                    {
                        break;
                    }
                    else
                    {
                        min = min;
                        max = i-1;
                        i = (min+max)/2;
                        boost::tie(tt_start, tt_arrival, s) = timetable[i];
                    }
                }
            }
            if( start > tt_start)
                ret = -1;
            else
            {
                while(i < timetable.size()) {
                    boost::tie(tt_start, tt_arrival, s) = timetable[i];
                    if(s[day]) 
                    {
                        has_traffic = true;
                        break;
                    }
                    ++i;
                }
                if(!has_traffic)
                    ret = -1;
                else 
                    ret = tt_arrival;
            }
            
            //there might be trips on the previous day for first few hours of the current day
            if(start < 2*3600 && allowed_lookups & PrevDay)
            {
                std::pair<bool, int> prev_day;
                prev_day = (*this)(start + 24*3600, day-1, backward, PrevDay);
                prev_day.second -= 24*3600;
                if((prev_day.first && !has_traffic) || (prev_day.first && prev_day.second < ret)) {
                    return prev_day;
                }
            }
            
            // Start is on the next day, we'd better lookup in this one too
            if(start >= 24*3600 && allowed_lookups & NextDay)
            {   
                std::pair<bool, int> next_day;
                next_day = (*this)(start - 24*3600, day+1, backward, NextDay);
                next_day.second += 24*3600;
                
                if((next_day.first && !has_traffic) || (next_day.first && next_day.second < ret)) {
                    return next_day;
                }
            }

            return std::make_pair<bool, int>(has_traffic, ret);
        }
        
    }
}

namespace Transport {

Graph::Graph(const std::string & filename)
{
    load(filename);
}

Graph::Graph(int nb_nodes) : g(nb_nodes+1)
{
}

void Graph::initEdgeIndexes() {
    std::cout << "Populating indexes\n";
    if(!edges_vec.empty())
        edges_vec.clear();
    edges_vec.resize(num_edges(g));
    int index = 0;
    BOOST_FOREACH(edge_t e, boost::edges(g)) {
        edges_vec[index] = e;
        g[e].edge_index = index; 
        BOOST_ASSERT(edges_vec[index] == e);
        index++;
    }
}

void Graph::add_edge(int source, int target, const Edge & e)
{
    boost::add_edge(source, target, e, g);
}

bool Graph::public_transport_edge(int source, int target, DurationType dur_type, float start, float arrival, 
                                  int duration, const std::string & services, const EdgeMode type)
{
    edge_t e;
    bool b;
    tie(e, b) = edge(source, target, g);
    if(!b)
    {
        bool c;
        tie(e, c) = boost::add_edge(source, target, g);
        g[e].type = type;
        g[e].duration.dur_type = dur_type;
    }
    
    if(dur_type == TimetableDur)
        g[e].duration.append_timetable(start, arrival, services);
    else if(dur_type == FrequencyDur)
        g[e].duration.append_frequency(start, arrival, duration, services);
    else if(dur_type == ConstDur)
        g[e].duration.const_duration = duration;
    
    return !b;
}
    struct found_goal
    {
    }; // exception for termination

    // visitor that terminates when we find the goal

    class dijkstra_goal_visitor : public boost::default_dijkstra_visitor
    {   
        public:
            
            dijkstra_goal_visitor(int goal) : m_goal(goal)
        {
        }   
            
            template <class Graph_t>
                void examine_vertex(int u, Graph_t& g)
                {   
                    if (u == m_goal)
                        throw found_goal();
                }
        private:
            int m_goal;
    };

float calc_duration(float in, Duration d)
{
    return d(in, 0).second;
}

struct Comp
{
    bool operator()(float a, float b) const {return a<b;}
    bool operator()(const Duration &, float) const {return false;}
};

bool Graph::dijkstra(int source, int target)
{
    std::vector<int> p(boost::num_vertices(g));
    std::vector<float> d(boost::num_vertices(g));
    try{
    boost::dijkstra_shortest_paths(g, source,
            boost::predecessor_map(&p[0])
            .distance_map(&d[0])
            .weight_map(get(&Edge::duration, g))
            .visitor(dijkstra_goal_visitor(target))
            .distance_zero(30000)
            .distance_combine(&calc_duration)
            .distance_compare(Comp())
            );
    return false;
    }
    catch(found_goal)
    {
        return true;
    }

}
    
void Graph::sort()
{
    BOOST_FOREACH(edge_t e, boost::edges(g))
    {
        g[e].duration.sort();
    }
}

void Graph::load(const std::string & filename)
{
    std::cout << "Loading graph from file " << filename << std::endl;
    std::ifstream ifile(filename.c_str());
    boost::archive::binary_iarchive iArchive(ifile);
    iArchive >> g; //graph;   
    std::cout << "   " << boost::num_vertices(g) << " nodes" << std::endl;
    std::cout << "   " << boost::num_edges(g) << " edges" << std::endl;
    initEdgeIndexes();
}

void Graph::save(const std::string & filename) const
{
    std::ofstream ofile(filename.c_str());
    boost::archive::binary_oarchive oArchive(ofile);
    oArchive << g;
}

EdgeList Graph::listEdges(const EdgeMode type)
{
    EdgeList edgeList;
    int index = 0;
    BOOST_FOREACH(edge_t e, boost::edges(g)) {
        if(g[e].edge_index != index++) {
            this->initEdgeIndexes();
            return this->listEdges(type);
        }
            
        if(type == WhateverEdge || g[e].type == type)
            edgeList.push_back(g[e].edge_index);
    }
    return edgeList;
}


/**
 * Return the edge descriptor corresponding to a an edge id
 */
inline edge_t Graph::edge_descriptor(const int edge_id) 
{    
    return edges_vec[edge_id];
}

Edge Graph::mapEdge(const int edge)
{
    return g[this->edge_descriptor(edge)];
}

int Graph::sourceNode(const int edge)
{
    return source(this->edge_descriptor(edge), g);
}

int Graph::targetNode(const int edge)
{
    return target(this->edge_descriptor(edge), g);
}

} // end namespace Transport