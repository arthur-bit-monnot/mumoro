#ifndef REGLC_GRAPH_H
#define REGLC_GRAPH_H

#include "graph_wrapper.h"

// TODO : this type def should be a tuple and the third part shoul be a EdgeMode.
// It is done this way to workaround problems with SWIG
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
    int max_vertices() { return (int) _max_vertices; }
    
    
    RLCVertice toVertice(uint composed_vertice);
    uint toInt(RLCVertice vertice);
    
private:
    const uint _max_vertices;
    uint vertice_conv_mask;
    const uint length_dfa;
    const uint length_graph;
    
};

/********** Test and temporary functions *********/

DFA default_dfa();
void RLC_test(Graph g);


#endif