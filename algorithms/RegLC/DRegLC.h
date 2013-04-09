#ifndef DREGLC_H
#define DREGLC_H

#include <boost/heap/fibonacci_heap.hpp>
#include <boost/foreach.hpp> 

#include "utils.h"
#include "reglc_graph.h"
#include "nodes_filter.h"

namespace RLC {
    


struct DRegLCParams {
    DRegLCParams( AbstractGraph * graph, const int day, const int cost_factor = 1 ) : 
    graph(graph), 
    day(day),
    cost_factor(cost_factor) {}
    
    AbstractGraph * graph;
    const int day;
    const int cost_factor;
};


struct DRegCompare
{
    int ***costs;
    DRegCompare() {}
    DRegCompare( int *** dist ) { costs = dist; }
    bool operator()(RLC::Vertice a, RLC::Vertice b) const
    {
        return (*costs)[a.second][a.first] > (*costs)[b.second][b.first];
    }
    
};

    
typedef boost::heap::fibonacci_heap<RLC::Vertice, boost::heap::compare<DRegCompare> > DRegHeap;



/**
 * Implementation of DRegLC defined by Barret & al.
 */
class DRegLC
{
public:
    typedef LISTPARAM<DRegLCParams> ParamType;

    DRegLC( ParamType parameters );
    DRegLC() : trans_num_vert(0), dfa_num_vert(0) {}
    virtual ~DRegLC();
    
    virtual bool finished() const;
    virtual bool check_termination( const RLC::Vertice & vert ) const { return false; }
    
    /**
     * Run the dijkstra algorithm until a stop condition is met
     */
    virtual bool run();
    
    /**
     * Pop the vertice with minimum cost in heap and expand its outgoing edges.
     */
    virtual Vertice treat_next();
    
    /**
     * Insert/update a node in the heap.
     * Return True if the node was updated, false otherwise.
     */
    virtual bool insert_node_with_predecessor(const Vertice & node, const int arrival, const int cost, const RLC::Edge & pred);
    virtual bool insert_node(const Vertice & node, const int arrival, const int cost);
    
    virtual std::pair <bool, int> duration( const RLC::Edge& edge, const float start_sec, const int day ) const {
        return graph->duration( edge, start_sec, day );
    }
    
    /**
     * Heap in which the Vertices will be stored
     */
    DRegHeap heap;

    inline float arrival(const RLC::Vertice v) { return arr_times[v.second][v.first]; }
    inline void set_arrival(const RLC::Vertice v, float time) { arr_times[v.second][v.first] = time; }
    inline float cost(const RLC::Vertice v) const { return costs[v.second][v.first]; }
    inline void set_cost(const RLC::Vertice v, const int cost) { costs[v.second][v.first] = cost; }
    
    inline DRegHeap::handle_type dij_node(const RLC::Vertice v) const { return references[v.second][v.first]; }
    inline void put_dij_node(const RLC::Vertice v) { 
        references[v.second][v.first] = heap.push(v);
    }
    inline void clear_pred(const RLC::Vertice v) { has_predecessor[v.second]->reset(v.first); }
    inline void set_pred(const RLC::Vertice v, const RLC::Edge & pred) { 
        has_predecessor[v.second]->set(v.first);
        predecessors[v.second][v.first] = pred; 
    }

    inline RLC::Edge get_pred(const RLC::Vertice v) const { return predecessors[v.second][v.first]; }
    inline bool has_pred(const RLC::Vertice v) const { return has_predecessor[v.second]->test(v.first); }
    
    inline DRegHeap::handle_type handle(const RLC::Vertice v) const { return references[v.second][v.first]; }
    
    inline bool white(const RLC::Vertice v) const { return status[v.second][v.first] == 0; }
    inline bool gray(const RLC::Vertice v) const { return status[v.second][v.first] == 1; }
    inline bool black(const RLC::Vertice v) const { return status[v.second][v.first] == 2; }
    inline void set_white(const RLC::Vertice v) { status[v.second][v.first] = 0; }
    inline void set_gray(const RLC::Vertice v) { status[v.second][v.first] = 1; }
    inline void set_black(const RLC::Vertice v) { status[v.second][v.first] = 2; }
    
    
    bool success;
    
protected:
    /**
     * Number of vertices in the transport (resp. DFA) graph.
     * 
     * Those are here to make sure we don't need the graph in the destructor
     * This is usefull since, the graph might be deleted before the DRegLC instance
     */
    int trans_num_vert;
    int dfa_num_vert;
    
    AbstractGraph * graph;
    int day;
    int cost_factor;
    
    float **arr_times;
    int **costs;
    DRegHeap::handle_type **references;
    boost::dynamic_bitset<> ** has_predecessor;
    RLC::Edge **predecessors;
    uint **status; //TODO : very big for only two bits ...
};





} // end namespace RLC

#endif