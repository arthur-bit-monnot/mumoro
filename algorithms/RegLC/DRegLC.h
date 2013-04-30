#ifndef DREGLC_H
#define DREGLC_H

#include <boost/heap/fibonacci_heap.hpp>
#include <boost/heap/d_ary_heap.hpp>
#include <boost/foreach.hpp> 
#include <boost/dynamic_bitset.hpp>

#include "utils.h"
#include "reglc_graph.h"

namespace RLC {
    


struct DRegLCParams {
    DRegLCParams( const AbstractGraph * graph, const int day, const int cost_factor = 1 ) : 
    graph(graph), 
    day(day),
    cost_factor(cost_factor) {}
    
    const AbstractGraph * graph;
    const int day;
    const int cost_factor;
};


struct DRegCompare
{
    int ** & costs;
    DRegCompare( int ** & dist ) : costs(dist) {}
    bool operator()(RLC::Vertice a, RLC::Vertice b) const
    {
        return costs[a.second][a.first] > costs[b.second][b.first];
    }
    
};

    
// typedef boost::heap::fibonacci_heap<RLC::Vertice, boost::heap::compare<DRegCompare> > DRegHeap;
typedef boost::heap::d_ary_heap<
    RLC::Vertice, 
    boost::heap::arity<4>,
    boost::heap::mutable_<true>, 
    boost::heap::compare<DRegCompare> > DRegHeap;


/**
 * Implementation of DRegLC defined by Barret & al.
 */
class DRegLC
{
public:
    typedef LISTPARAM<DRegLCParams> ParamType;

    DRegLC( ParamType parameters ) :
    success( false )
    {
        DRegLCParams & p = parameters.value;
        
        if( heap != NULL )
            delete heap;
        heap = new DRegHeap( DRegCompare( this-> costs ) );
        
        this->graph = p.graph;
        day = p.day;
        cost_factor = p.cost_factor;
        
        trans_num_vert = graph->num_transport_vertices();
        dfa_num_vert = graph->num_dfa_vertices();
        
        arr_times = new float*[dfa_num_vert];
        costs = new int*[dfa_num_vert];
        references = new DRegHeap::handle_type*[dfa_num_vert];
        status = new uint*[dfa_num_vert];
        has_predecessor = new boost::dynamic_bitset<>*[dfa_num_vert];
        predecessors = new RLC::Edge*[dfa_num_vert];
        for(int i=0 ; i<dfa_num_vert ; ++i) {
            arr_times[i] = new float[trans_num_vert];
            costs[i] = new int[trans_num_vert];
            references[i] = new DRegHeap::handle_type[trans_num_vert];
            status[i] = new uint[trans_num_vert];
            has_predecessor[i] = new boost::dynamic_bitset<>(trans_num_vert);
            predecessors[i] = (RLC::Edge *) malloc(trans_num_vert * sizeof(RLC::Edge));

            // all vertices are white
            memset(status[i], 0, trans_num_vert * sizeof(status[0][0]));
        }
    }
    DRegLC() : trans_num_vert(0), dfa_num_vert(0) {}
    
    
    virtual ~DRegLC()
    {
        for(int i=0 ; i<dfa_num_vert ; ++i) {
            delete[] arr_times[i];
            delete[] costs[i];
            delete[] references[i];
            delete[] status[i];
            delete has_predecessor[i];
            delete[] predecessors[i];
        }
        delete[] arr_times;
        delete[] costs;
        delete[] references;
        delete[] status;
        delete[] has_predecessor;
        delete[] predecessors;
    }
    
    virtual bool finished() const
    {
        return heap->empty();
    }
    
    virtual bool check_termination( const RLC::Vertice & vert ) const { return false; }
    
    /**
     * Run the dijkstra algorithm until a stop condition is met
     */
    virtual bool run()
    {    
        while( !finished() ) 
        {
            treat_next();
        }
            
        return success;
    }
    
    /**
     * Pop the vertice with minimum cost in heap and expand its outgoing edges.
     */
    virtual Vertice treat_next()
    {
        RLC::Vertice curr = heap->top();
        heap->pop();
        set_black(curr);
        
        if( check_termination(curr) ) {
            success = true;
            return curr;
        }
        
        std::list<RLC::Edge> n_out_edges = graph->out_edges(curr);
        BOOST_FOREACH(RLC::Edge e, n_out_edges) 
        {
            RLC::Vertice target = graph->target(e);
            
            bool has_traffic;
            int edge_cost;
            boost::tie(has_traffic, edge_cost) = duration(e, arrival(curr), day);

            int target_cost = cost(curr) + edge_cost * cost_factor;
            
//             if(!(!has_traffic || (edge_cost * cost_factor - cost_eval(curr, 0)  + cost_eval(target, 0) >= 0) ))
            BOOST_ASSERT(!has_traffic || (edge_cost * cost_factor - cost_eval(curr, 0)  + cost_eval(target, 0) >= 0) );
            
            // ignore the edge if it provokes an overflow
            if(target_cost < cost(curr))
                has_traffic = false;
            
            float target_arr;
            if(graph->forward)
                target_arr = arrival(curr) + edge_cost;
            else 
                target_arr = arrival(curr) - edge_cost;
            
            if(has_traffic) {
                if(edge_cost >= 0) {
                    insert_node_with_predecessor(target, target_arr, target_cost, e);
                }
            }
            
            
        }
        return curr;
    }
    
    /**
     * Insert/update a node in the heap.
     * Return True if the node was updated, false otherwise.
     */
    virtual bool insert_node_with_predecessor(const Vertice & vert, const int arrival, const int cost, const RLC::Edge & pred)
    {
        bool was_inserted = insert_node( vert, arrival, cost );
        if( was_inserted )
            set_pred( vert, pred );
        return was_inserted;
    }
    
    virtual bool insert_node(const Vertice & vert, const int arrival, const int vert_cost)
    {
        if( white(vert) )
        {
            set_arrival(vert, arrival);
            set_cost(vert, vert_cost);
            put_dij_node(vert);
            set_gray(vert);
            
            return true;
        }
        else if( vert_cost < cost(vert) )
        {
            if(black(vert))
                BOOST_ASSERT(!black(vert));
            
            set_arrival(vert, arrival);
            set_cost(vert, vert_cost);
            heap->update(handle(vert));

            return true;
        }
        else
        {
            return false;
        }
    }
    
    virtual std::pair <bool, int> duration( const RLC::Edge& edge, const float start_sec, const int day ) const {
        return graph->duration( edge, start_sec, day );
    }
    
    /**
     * Heap in which the Vertices will be stored
     */
    DRegHeap * heap = NULL;

    inline float arrival(const RLC::Vertice v) { return arr_times[v.second][v.first]; }
    inline void set_arrival(const RLC::Vertice v, float time) { arr_times[v.second][v.first] = time; }
    inline float cost(const RLC::Vertice v) const { return costs[v.second][v.first]; }
    inline virtual void set_cost(const RLC::Vertice v, const int cost) { costs[v.second][v.first] = cost; }
    
    /**
     * Used to provide an evaluation of the cost of node v.
     * This is to be replaced with an evaluation of the current cost + cost to target
     */
    virtual inline int cost_eval( const Vertice & v, const int cost ) const { return cost; }
    
    inline DRegHeap::handle_type dij_node(const RLC::Vertice v) const { return references[v.second][v.first]; }
    inline void put_dij_node(const RLC::Vertice v) { 
        references[v.second][v.first] = heap->push(v);
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
    
    const AbstractGraph * graph;
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