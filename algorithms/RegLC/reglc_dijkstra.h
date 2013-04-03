#ifndef REGLC_DIJKSTRA_H
#define REGLC_DIJKSTRA_H

#include <boost/heap/fibonacci_heap.hpp>

#include "reglc_graph.h"
#include "nodes_filter.h"

namespace RLC {


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
    DijkstraParameters() : 
    save_touched_nodes(false), 
    use_cost_lower_bounds(false) , 
    cost_limit(false), 
    cost_factor(1),
    filter_nodes(false),
    filter(NULL)
    {}
    
    ~DijkstraParameters() { if(filter != NULL) delete filter; }
    
    /**
     * If set to True, every time a node is touched (inserted or modified in heap)
     * ti is added to the `touched_nodes` list.
     * 
     * This is mainly useful to run a bidirectional Dijkstra
     */
    bool save_touched_nodes;
    
    /**
     * IF set to true, the algorithm wont use the arrival/departure times but systematicaly
     * use the lower bound of the edges cost.
     * 
     * This is mainly useful when searching bacvkward with no known departure times.
     */
    bool use_cost_lower_bounds;
    
    /**
     * If set to true, nodes with cost above `cost_limit_value` won't be inserted in heap.
     */
    bool cost_limit;
    int  cost_limit_value;
    
    /**
     * A factor to apply to the edge cost before adding it
     */
    int cost_factor;
    
    bool filter_nodes;
    NodeFilter * filter;
};


typedef boost::heap::fibonacci_heap<Dij_node, boost::heap::compare<Compare> > Heap;



/**
 * Implementation of DRegLC defined by Barret & al.
 */
class Dijkstra
{
    AbstractGraph *graph;
public:
    Dijkstra(AbstractGraph *graph, int source, int dest, float start_sec, int start_day, DijkstraParameters * params = NULL);
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
    
    DijkstraParameters * params;
    
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
     * Numbers of nodes visited by the algorithm
     */
    int visited_nodes;
    
    /**
     * Heap in which the Vertices will be stored
     */
    Heap heap;
    
    
    std::set<Vertice> source_vertices;
    std::set<Vertice> dest_vertices;
    
    std::list< int > touched_edges;
    std::list< Vertice > touched_nodes;
    
    /**
     * When a path is found, those store the arrival time and the cost of the destination node
     */
    float path_arrival;
    int path_cost;
    
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

} // end namespace RLC

#endif