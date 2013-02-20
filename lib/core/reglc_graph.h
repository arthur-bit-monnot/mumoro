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

typedef std::pair<int, int> RLCVertice;
typedef std::pair<edge_t, edge_t> RLCEdge;

class RegLCGraph
{
public:
    RegLCGraph(Graph g, DFA dfa);
    Graph g;
    DFA dfa;
    
    
    /**
     * Returns the source node of an edge
     */
    RLCVertice source(RLCEdge edge);
    
    /**
     * Return the target node of an edge
     */
    RLCVertice target(RLCEdge);
    
    /**
     * Return a list containing every outgoing edge of a node
     */
    std::list<RLCEdge> out_edges(RLCVertice);
    
    /**
     * Returns the arrival time of trip starting at the source node of the edge at time
     * start_sec on day day
     */
    float duration(RLCEdge edge, float start_sec, int day);
    
    /**
     * Return the maximum index of a vertice in this graph
     */    
};



struct Dij_node 
{
    RLCVertice v;
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
    RegLCGraph *graph;
public:
    Dijkstra(RegLCGraph *graph, int source, int dest, float start_sec, int start_day);
    ~Dijkstra();
    
    std::list<int> run();
    
    inline float arrival(RLCVertice v) { return arr_times[v.second][v.first]; }
    inline void set_arrival(RLCVertice v, float time) { arr_times[v.second][v.first] = time; }
    
    inline Heap::handle_type dij_node(RLCVertice v) { return references[v.second][v.first]; }
    inline void put_dij_node(RLCVertice v) { 
        Dij_node n;
        n.v = v;
        references[v.second][v.first] = heap.push(n); //PB
    }
    inline void set_pred(RLCVertice v, RLCEdge pred) { predecessors[v.second][v.first] = pred; }
    inline RLCEdge get_pred(RLCVertice v) { return predecessors[v.second][v.first]; }
    
    inline Heap::handle_type handle(RLCVertice v) { return references[v.second][v.first]; }
    
    inline bool white(RLCVertice v) { return status[v.second][v.first] == 0; }
    inline bool gray(RLCVertice v) { return status[v.second][v.first] == 1; }
    inline bool black(RLCVertice v) { return status[v.second][v.first] == 2; }
    inline void set_white(RLCVertice v) { status[v.second][v.first] = 0; }
    inline void set_gray(RLCVertice v) { status[v.second][v.first] = 1; }
    inline void set_black(RLCVertice v) { status[v.second][v.first] = 2; }
    
    
    float **arr_times;
    Heap::handle_type **references;
    RLCEdge **predecessors;
    uint **status; //TODO : very big for only two bits ...
    
    int source;
    int dest;
    float start_sec;
    int start_day;
    
    Heap heap;
    
    std::list< int > touched_edges;
};


DFA foot_subway_dfa();
DFA all_dfa();

}

/********** Test and temporary functions *********/




#endif