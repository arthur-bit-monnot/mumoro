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
DFA bike_dfa();
DFA foot_dfa();
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
    virtual RLC::Vertice source(RLC::Edge edge) = 0;
    
    /**
     * Return the target node of an edge
     */
    virtual RLC::Vertice target(RLC::Edge) = 0;
    
    /**
     * Return a list containing every outgoing edge of a node
     */
    virtual std::list<RLC::Edge> out_edges(RLC::Vertice) = 0;
    
    /**
     * Returns the arrival time of trip starting at the source node of the edge at time
     * start_sec on day day
     */
    virtual std::pair<bool, int> duration(RLC::Edge edge, float start_sec, int day) = 0;
    
    virtual std::set<int> dfa_start_states() = 0;
    virtual std::set<int> dfa_accepting_states() = 0;
    virtual int num_transport_vertices() = 0;
    virtual int num_dfa_vertices() = 0;
    
    inline bool is_accepting(RLC::Vertice v) { return dfa_accepting_states().find(v.second) != dfa_accepting_states().end(); }
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
    std::pair<bool, int> duration(RLC::Edge edge, float start_sec, int day);
    
    /**
     * Start states in the DFA.
     * 
     * For backward graphs, this is the set of accepting states.
     */
    std::set<int> dfa_start_states();
    
    /**
     * Accepting states in the DFA.
     * 
     * For backward graphs, this is the start state.
     */
    std::set<int> dfa_accepting_states();
    
    /**
     * Number of vertices in the transport graph
     */
    int num_transport_vertices();
    
    /**
     * Number of vertices in the DFA
     */
    int num_dfa_vertices();
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
    std::pair<bool, int> duration(RLC::Edge edge, float start_sec, int day);
    
    /**
     * Start states in the DFA.
     * 
     * For backward graphs, this is the set of accepting states.
     */
    std::set<int> dfa_start_states();
    
    /**
     * Accepting states in the DFA.
     * 
     * For backward graphs, this is the start state.
     */
    std::set<int> dfa_accepting_states();
    int num_transport_vertices();
    int num_dfa_vertices();
    
};



struct Dij_node 
{
    RLC::Vertice v;
};

struct Predecessor
{
    Predecessor() : has_pred(false) {}
    Predecessor(RLC::Edge e) : has_pred(true), pred(e) {}
    bool has_pred;
    RLC::Edge pred;
};

struct Compare
{
    int ***costs;
    Compare() {}
    Compare( int *** dist ) { costs = dist; }
    bool operator()(Dij_node a, Dij_node b) const
    {
        return (*costs)[a.v.second][a.v.first] > (*costs)[b.v.second][b.v.first];
    }
    
};


struct DijkstraParameters
{
    DijkstraParameters() : save_touched_nodes(false), cost_limit(false) {}
    
    /**
     * If set to True, every time a node is touched (inserted or modified in heap)
     * ti is added to the `touched_nodes` list.
     * 
     * This is mainly useful to run a bidirectional Dijkstra
     */
    bool save_touched_nodes;
    
    /**
     * If set to true, nodes with cost above `cost_limit_value` won't be inserted in heap.
     */
    bool cost_limit;
    int  cost_limit_value;
};


typedef boost::heap::fibonacci_heap<Dij_node, boost::heap::compare<Compare> > Heap;



/**
 * Implementation of DRegLC defined by Barret & al.
 */
class Dijkstra
{
    AbstractGraph *graph;
public:
    Dijkstra(AbstractGraph *graph, int source, int dest, float start_sec, int start_day, DijkstraParameters params = DijkstraParameters());
    Dijkstra() : trans_num_vert(0), dfa_num_vert(0) {}
    ~Dijkstra();
    
    /**
     * Run the dijkstra algorithm until a stop condition is met
     */
    bool run();
    
    /**
     * Pop the vertice with minimum cost in heap and expand its outgoing edges.
     */
    Vertice treat_next();
    
    /**
     * Insert/update a node in the heap.
     * Return True if the node was updated, false otherwise.
     */
    bool insert_node(Vertice node, int arrival, int cost, Predecessor pred);
    
    DijkstraParameters params;
    
    float **arr_times;
    int **costs;
    Heap::handle_type **references;
    Predecessor **predecessors;
    uint **status; //TODO : very big for only two bits ...

    /**
     * Source (resp. destination) nodes in the transport graph.
     * Sources (with corresponding dfa start states) will be insert in heap.
     * Dest (with corresponding accepting dfa states) will be used as a stop condition.
     * 
     * A value of -1 means that there is no known source (resp. destination) at the beginning of the algorithm.
     */
    int source;
    int dest;
    
    /**
     * Departure time from source nodes
     */
    float start_sec;
    int start_day;
    
    /**
     * Set to true when a path has been found.
     * False otherwise.
     */
    bool path_found;
    
    /**
     * Heap in which the Vertices will be stored
     */
    Heap heap;
    
    
    std::set<Vertice> source_vertices;
    std::set<Vertice> dest_vertices;
    
    std::list< int > touched_edges;
    std::list< Vertice > touched_nodes;
    
    float path_arrival;
    std::list< RLC::Edge > path;
    
    EdgeList get_transport_path();
    VisualResult get_result();
    

    inline float arrival(const RLC::Vertice v) { return arr_times[v.second][v.first]; }
    inline void set_arrival(const RLC::Vertice v, float time) { arr_times[v.second][v.first] = time; }
    inline float cost(const RLC::Vertice v) const { return costs[v.second][v.first]; }
    inline void set_cost(const RLC::Vertice v, const int cost) { costs[v.second][v.first] = cost; }
    
    inline Heap::handle_type dij_node(const RLC::Vertice v) const { return references[v.second][v.first]; }
    inline void put_dij_node(const RLC::Vertice v) { 
        Dij_node n;
        n.v = v;
        references[v.second][v.first] = heap.push(n);
    }
    inline void clear_pred(const RLC::Vertice v) { predecessors[v.second][v.first].has_pred = false; }
    inline void set_pred(const RLC::Vertice v, const RLC::Edge pred) { 
        predecessors[v.second][v.first].has_pred = true;
        predecessors[v.second][v.first].pred = pred; 
    }
    inline void set_pred(const RLC::Vertice v, const Predecessor pred) { 
        predecessors[v.second][v.first] = pred;
    }
    inline RLC::Edge get_pred(const RLC::Vertice v) const { return predecessors[v.second][v.first].pred; }
    inline bool has_pred(const RLC::Vertice v) const { return predecessors[v.second][v.first].has_pred; }
    
    inline Heap::handle_type handle(const RLC::Vertice v) const { return references[v.second][v.first]; }
    
    inline bool white(const RLC::Vertice v) const { return status[v.second][v.first] == 0; }
    inline bool gray(const RLC::Vertice v) const { return status[v.second][v.first] == 1; }
    inline bool black(const RLC::Vertice v) const { return status[v.second][v.first] == 2; }
    inline void set_white(const RLC::Vertice v) { status[v.second][v.first] = 0; }
    inline void set_gray(const RLC::Vertice v) { status[v.second][v.first] = 1; }
    inline void set_black(const RLC::Vertice v) { status[v.second][v.first] = 2; }
    
private:
    /**
     * Number of vertices in the transport (resp. DFA) graph.
     * 
     * Those are here to make sure we don't need the graph in the destructor
     * This is usefull since, the graph might be deleted before the Dijkstra instance
     */
    const int trans_num_vert;
    const int dfa_num_vert;
};


}

#endif
