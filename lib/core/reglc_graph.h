#ifndef REGLC_GRAPH_H
#define REGLC_GRAPH_H

#include <boost/heap/fibonacci_heap.hpp>
#include <boost/foreach.hpp> //TODO : remove when not used

#include "graph_wrapper.h"


// TODO : this type def should be a tuple and the third part shoul be a EdgeMode.
// It is done this way to workaround problems with SWIG

namespace RLC {

typedef std::pair<std::pair<int, int>, int> DfaEdge;
typedef std::vector<DfaEdge> DfaEdgeList;

class DFA
{
public:
    DFA(int start, std::set<int> accepting, DfaEdgeList edges);
    int start_state;
    std::set<int> accepting_states;
    Graph_t graph;
};

DFA foot_subway_dfa();
DFA all_dfa();
DFA pt_foot_dfa();

typedef std::pair<int, int> Vertice;
typedef std::pair<edge_t, edge_t> Edge;

class Graph
{
public:
    Graph(Transport::Graph *trans, DFA dfa);
    Transport::Graph *transport;
    DFA dfa;
    
    
    /**
     * Returns the source node of an edge
     */
    RLC::Vertice source(RLC::Edge edge);
    
    /**
     * Return the target node of an edge
     */
    RLC::Vertice target(RLC::Edge);
    
    /**
     * Return a list containing every outgoing edge of a node
     */
    std::list<RLC::Edge> out_edges(RLC::Vertice);
    
    /**
     * Returns the arrival time of trip starting at the source node of the edge at time
     * start_sec on day day
     */
    float duration(RLC::Edge edge, float start_sec, int day);
    
    /**
     * Return the maximum index of a vertice in this graph
     */    
};



struct Dij_node 
{
    RLC::Vertice v;
};

struct Compare
{
    float ***vec;
    Compare() {}
    Compare(float *** dist) { vec = dist; }
    bool operator()(Dij_node a, Dij_node b) const
    {
        return (*vec)[a.v.second][a.v.first] > (*vec)[b.v.second][b.v.first];
    }
    
};

typedef boost::heap::fibonacci_heap<Dij_node, boost::heap::compare<Compare> > Heap;

class Dijkstra
{
    Graph *graph;
public:
    Dijkstra(Graph *graph, int source, int dest, float start_sec, int start_day);
    ~Dijkstra();
    
    bool run();
    

    
    
    float **arr_times;
    Heap::handle_type **references;
    RLC::Edge **predecessors;
    uint **status; //TODO : very big for only two bits ...
    
    int source;
    int dest;
    float start_sec;
    int start_day;
    bool path_found;
    Heap heap;
    
    std::list< int > touched_edges;
    float path_arrival;
    std::list< RLC::Edge > path;
    
    EdgeList get_transport_path();
    
private:
    inline float arrival(RLC::Vertice v) { return arr_times[v.second][v.first]; }
    inline void set_arrival(RLC::Vertice v, float time) { arr_times[v.second][v.first] = time; }
    
    inline Heap::handle_type dij_node(RLC::Vertice v) { return references[v.second][v.first]; }
    inline void put_dij_node(RLC::Vertice v) { 
        Dij_node n;
        n.v = v;
        references[v.second][v.first] = heap.push(n); //PB
    }
    inline void set_pred(RLC::Vertice v, RLC::Edge pred) { predecessors[v.second][v.first] = pred; }
    inline RLC::Edge get_pred(RLC::Vertice v) { return predecessors[v.second][v.first]; }
    
    inline Heap::handle_type handle(RLC::Vertice v) { return references[v.second][v.first]; }
    
    inline bool white(RLC::Vertice v) { return status[v.second][v.first] == 0; }
    inline bool gray(RLC::Vertice v) { return status[v.second][v.first] == 1; }
    inline bool black(RLC::Vertice v) { return status[v.second][v.first] == 2; }
    inline void set_white(RLC::Vertice v) { status[v.second][v.first] = 0; }
    inline void set_gray(RLC::Vertice v) { status[v.second][v.first] = 1; }
    inline void set_black(RLC::Vertice v) { status[v.second][v.first] = 2; }
};




}

/********** Test and temporary functions *********/




#endif