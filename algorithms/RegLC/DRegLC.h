#ifndef DREGLC_H
#define DREGLC_H

#include <boost/heap/fibonacci_heap.hpp>
#include <boost/heap/d_ary_heap.hpp>
#include <boost/foreach.hpp> 
#include <boost/dynamic_bitset.hpp>

#include "utils.h"
#include "reglc_graph.h"
#include "LabelSettingAlgo.h"

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

    
// typedef boost::heap::fibonacci_heap<RLC::Vertice, boost::heap::compare<DRegCompare> > DRegHeap;
typedef boost::heap::d_ary_heap<
    Label, 
    boost::heap::arity<4>,
    boost::heap::mutable_<true> > DRegHeap;


/**
 * Implementation of DRegLC defined by Barret & al.
 */
class DRegLC : public LabelSettingAlgo
{
public:
    typedef LISTPARAM<DRegLCParams> ParamType;

    DRegLC( ParamType parameters ) :
    success( false )
    {
        DRegLCParams & p = parameters.value;
        
        if( heap != NULL )
            delete heap;
        heap = new DRegHeap(  );
        
        this->graph = p.graph;
        day = p.day;
        cost_factor = p.cost_factor;
        
        trans_num_vert = graph->num_transport_vertices();
        dfa_num_vert = graph->num_dfa_vertices();
        
        references = new DRegHeap::handle_type*[dfa_num_vert];
        status = new uint*[dfa_num_vert];
        has_predecessor = new boost::dynamic_bitset<>*[dfa_num_vert];
        predecessors = new RLC::Edge*[dfa_num_vert];
        for(int i=0 ; i<dfa_num_vert ; ++i) {
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
            delete[] references[i];
            delete[] status[i];
            delete has_predecessor[i];
            delete[] predecessors[i];
        }
        delete[] references;
        delete[] status;
        delete[] has_predecessor;
        delete[] predecessors;
    }
    
    virtual void clear() {
        heap->clear();
        for(int i=0 ; i<dfa_num_vert ; ++i) {
            has_predecessor[i]->reset();
            // all vertices are white
            memset(status[i], 0, trans_num_vert * sizeof(status[0][0]));
        }
        success = false;
    }
    
    virtual bool finished() const override
    {
        return heap->empty();
    }
    
    virtual bool check_termination( const RLC::Label & vert ) { return false; }
    
    /**
     * Run the dijkstra algorithm until a stop condition is met
     */
    virtual bool run() override
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
    virtual Label treat_next() override
    {
        Label curr = heap->top();
        heap->pop();
        set_black(curr.node);
        
        if( check_termination(curr) ) {
            success = true;
            return curr;
        }
        
        std::list<RLC::Edge> n_out_edges = graph->out_edges(curr.node);
        BOOST_FOREACH(RLC::Edge e, n_out_edges) 
        {
            RLC::Vertice target = graph->target(e);
            BOOST_ASSERT( target.first < graph->num_transport_vertices() );
            BOOST_ASSERT( target.second < graph->num_dfa_vertices() );
            
            bool has_traffic;
            int edge_cost;
            boost::tie(has_traffic, edge_cost) = duration(e, curr.time, day);

            int target_cost = curr.cost + edge_cost * cost_factor;
            
//             if(!(!has_traffic || (edge_cost * cost_factor - cost_eval(curr, 0)  + cost_eval(target, 0) >= 0) ))
            BOOST_ASSERT(!has_traffic || (edge_cost * cost_factor - (curr.cost + curr.h)  + cost_eval(target, 0) >= 0) );
            
            // ignore the edge if it provokes an overflow
            if(target_cost < curr.cost)
                has_traffic = false;
            
            float target_arr;
            if(graph->forward)
                target_arr = curr.time + edge_cost;
            else 
                target_arr = curr.time - edge_cost;
            
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
    
    virtual bool insert_node(const Vertice & vert, const int arrival, const int vert_cost) override
    {
        Label lab(vert, arrival, vert_cost);
        BOOST_ASSERT( lab.node.first < graph->num_transport_vertices() );
        BOOST_ASSERT( lab.node.second < graph->num_dfa_vertices() );
        
        if( white(lab.node) )
        {
            put_dij_node(lab);
            set_gray(lab.node);
            
            return true;
        }
        else if( vert_cost < lab.cost )
        {
            BOOST_ASSERT(!black(lab.node));
            
            heap->update(handle(lab.node));

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
     * Creates a new label with the given information.
     * 
     * This method should always be used in place of the constructor to allow automatic fill up (for instance for A*)
     */
    virtual Label label(RLC::Vertice vert, int time, int cost, int source = -1) {
        return Label(vert, time, cost, source);
    }
    
    /**
     * Heap in which the Vertices will be stored
     */
    DRegHeap * heap = NULL;
    
    /**
     * Used to provide an evaluation of the cost of node v.
     * This is to be replaced with an evaluation of the current cost + cost to target
     */
    virtual inline int cost_eval( const Vertice & v, const int cost ) const { return cost; }
    
    virtual inline int best_cost_in_heap() const { return heap->top().cost; }
    
    inline DRegHeap::handle_type dij_node(const RLC::Vertice v) const { return references[v.second][v.first]; }
    inline void put_dij_node(const Label l) { 
        references[l.node.second][l.node.first] = heap->push(l);
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
    
    DRegHeap::handle_type **references;
    boost::dynamic_bitset<> ** has_predecessor;
    RLC::Edge **predecessors;
    uint **status; //TODO : very big for only two bits ...
};





} // end namespace RLC

#endif