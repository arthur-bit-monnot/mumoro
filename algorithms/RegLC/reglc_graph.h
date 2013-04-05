#ifndef REGLC_GRAPH_H
#define REGLC_GRAPH_H

#include "graph_wrapper.h"


namespace RLC {

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

DFA foot_subway_dfa();
DFA bike_dfa();
DFA foot_dfa();
DFA car_dfa();
DFA pt_foot_dfa();
DFA bike_pt_dfa();
DFA pt_dfa();

typedef std::pair<int, int> Vertice;
typedef std::pair<edge_t, edge_t> Edge;

class AbstractGraph {
public:
    /**
     * Is this graph used for forward or backward path search
     */
    const bool forward;
    
    /**
     * Associated transport graph. 
     * This should not used directly except for information regarding edges and node properties.
     */
    Transport::Graph *transport;
    
    AbstractGraph( bool forward, Transport::Graph *transport ) : forward(forward), transport(transport) {}
    virtual ~AbstractGraph() {}
    
    /**
     * Returns the source node of an edge
     */
    virtual RLC::Vertice source( const RLC::Edge & edge ) const = 0;
    
    /**
     * Return the target node of an edge
     */
    virtual RLC::Vertice target( const RLC::Edge & ) const = 0;
    
    /**
     * Return a list containing every outgoing edge of a node
     */
    virtual std::list<RLC::Edge> out_edges( const RLC::Vertice & ) const = 0;
    
    /**
     * Returns the cost (duration) of trip starting at the source node of the edge at time
     * start_sec on day day
     */
    virtual std::pair<bool, int> duration( const RLC::Edge & edge, const float start_sec, const int day ) const = 0;
    
    /**
     * Returns the minimal cost (duration) of this edge
     */
    virtual std::pair<bool, int> min_duration( const RLC::Edge & edge ) const = 0;
    
    virtual std::set<int> dfa_start_states() const = 0;
    virtual std::set<int> dfa_accepting_states() const = 0;
    virtual int num_transport_vertices() const = 0;
    virtual int num_dfa_vertices() const = 0;
    
    inline bool is_accepting( const RLC::Vertice & v ) const { return dfa_accepting_states().find(v.second) != dfa_accepting_states().end(); }
};

class Graph : public AbstractGraph
{
public:
    Graph(Transport::Graph *trans, DFA dfa);
    virtual ~Graph() {}
    
    Transport::Graph *transport;
    DFA dfa;
    
    
    /**
     * Returns the source node of an edge
     */
    RLC::Vertice source( const RLC::Edge & edge ) const;
    
    /**
     * Return the target node of an edge
     */
    RLC::Vertice target( const RLC::Edge & ) const;
    
    /**
     * Return a list containing every outgoing edge of a node
     */
    std::list<RLC::Edge> out_edges(const RLC::Vertice & ) const;
    
    /**
     * Returns the arrival time of trip starting at the source node of the edge at time
     * start_sec on day day
     */
    std::pair<bool, int> duration(const RLC::Edge & edge, const float start_sec, const int day) const;
    
    /**
     * Returns the minimal cost (duration) of this edge
     */
    std::pair<bool, int> min_duration(const RLC::Edge & edge) const;
    
    /**
     * Start states in the DFA.
     * 
     * For backward graphs, this is the set of accepting states.
     */
    std::set<int> dfa_start_states() const;
    
    /**
     * Accepting states in the DFA.
     * 
     * For backward graphs, this is the start state.
     */
    std::set<int> dfa_accepting_states() const;
    
    /**
     * Number of vertices in the transport graph
     */
    int num_transport_vertices() const;
    
    /**
     * Number of vertices in the DFA
     */
    int num_dfa_vertices() const;
};

class BackwardGraph : public AbstractGraph
{
public:
    Graph *forward_graph;
    
    BackwardGraph ( Graph *forward_graph );
    virtual ~BackwardGraph() {} 
    
    /**
     * Returns the source node of an edge
     */
    RLC::Vertice source( const RLC::Edge & edge ) const;
    
    /**
     * Return the target node of an edge
     */
    RLC::Vertice target( const RLC::Edge & ) const;
    
    /**
     * Return a list containing every outgoing edge of a node
     */
    std::list<RLC::Edge> out_edges( const RLC::Vertice & ) const;
    
    /**
     * Returns the arrival time of trip starting at the source node of the edge at time
     * start_sec on day day
     */
    std::pair<bool, int> duration( const RLC::Edge & edge, const float start_sec, const int day ) const;
    
    /**
     * Returns the minimal cost (duration) of this edge
     */
    std::pair<bool, int> min_duration(const RLC::Edge & edge) const;
    
    /**
     * Start states in the DFA.
     * 
     * For backward graphs, this is the set of accepting states.
     */
    std::set<int> dfa_start_states() const;
    
    /**
     * Accepting states in the DFA.
     * 
     * For backward graphs, this is the start state.
     */
    std::set<int> dfa_accepting_states()const ;
    int num_transport_vertices() const;
    int num_dfa_vertices() const;
    
};



}

#endif
