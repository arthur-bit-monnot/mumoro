/** Copyright : Arthur Bit-Monnot (2013)  arthur.bit-monnot@laas.fr

This software is a computer program whose purpose is to [describe
functionalities and technical features of your software].

This software is governed by the CeCILL-B license under French law and
abiding by the rules of distribution of free software.  You can  use, 
modify and/ or redistribute the software under the terms of the CeCILL-B
license as circulated by CEA, CNRS and INRIA at the following URL
"http://www.cecill.info". 

As a counterpart to the access to the source code and  rights to copy,
modify and redistribute granted by the license, users are provided only
with a limited warranty  and the software's author,  the holder of the
economic rights,  and the successive licensors  have only  limited
liability. 

In this respect, the user's attention is drawn to the risks associated
with loading,  using,  modifying and/or developing or reproducing the
software by the user in light of its specific status of free software,
that may mean  that it is complicated to manipulate,  and  that  also
therefore means  that it is reserved for developers  and  experienced
professionals having in-depth computer knowledge. Users are therefore
encouraged to load and test the software's suitability as regards their
requirements in conditions enabling the security of their systems and/or 
data to be ensured and,  more generally, to use and operate it in the 
same conditions as regards security. 

The fact that you are presently reading this means that you have had
knowledge of the CeCILL-B license and that you accept its terms. 
*/

#ifndef DREGLC_H
#define DREGLC_H

#include <boost/heap/fibonacci_heap.hpp>
#include <boost/heap/d_ary_heap.hpp>
#include <boost/foreach.hpp> 
#include <boost/dynamic_bitset.hpp>

#include "utils.h"
#include "reglc_graph.h"
#include "LabelSettingAlgo.h"

using std::cout;
using std::cerr;
using std::endl;

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
        
        this->graph = p.graph;
        day = p.day;
        cost_factor = p.cost_factor;
        
        trans_num_vert = graph->num_transport_vertices();
        dfa_num_vert = graph->num_dfa_vertices();
        
        references = new DRegHeap::handle_type*[dfa_num_vert];
        status = new uint*[dfa_num_vert];
        for(int i=0 ; i<dfa_num_vert ; ++i) {
            references[i] = new DRegHeap::handle_type[trans_num_vert];
            status[i] = new uint[trans_num_vert];

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
        }
        delete[] references;
        delete[] status;
    }
    
    virtual void clear() {
        heap.clear();
        for(int i=0 ; i<dfa_num_vert ; ++i) {
            // all vertices are white
            memset(status[i], 0, trans_num_vert * sizeof(status[0][0]));
        }
        success = false;
    }
    
    virtual bool finished() const override
    {
        return heap.empty();
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
        Label curr = heap.top();
        heap.pop();
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
            
            // ignore the edge if it provokes an overflow
            if(target_cost < curr.cost)
                has_traffic = false;
            
            float target_arr;
            if(graph->forward)
                target_arr = curr.time + edge_cost;
            else 
                target_arr = curr.time - edge_cost;
            
            if(has_traffic) {
                BOOST_ASSERT( edge_cost >= 0 );
                BOOST_ASSERT( target_cost >= 0 );
                BOOST_ASSERT( (edge_cost * cost_factor - (curr.cost + curr.h)  + (target_cost + label(target, target_arr, target_cost).h) >= 0) );
                
                insert_node_with_predecessor(target, target_arr, target_cost, e, curr.source);
            }
        }
        return curr;
    }
    
    /**
     * Insert/update a node in the heap.
     * Return True if the node was updated or inserted, false otherwise.
     */
    virtual bool insert_node_with_predecessor(const Vertice & vert, const int arrival, const int cost, const RLC::Edge & pred, const int source)
    {
        return insert_node( vert, arrival, cost, source );
    }
    
    virtual bool insert_node(const Vertice & vert, const int arrival, const int vert_cost, const int source) override {
        return insert_node_impl(label(vert, arrival, vert_cost, source));
    }
    
    virtual bool insert_node_impl(const Label & lab)
    {
        BOOST_ASSERT( lab.node.first < graph->num_transport_vertices() );
        BOOST_ASSERT( lab.node.second < graph->num_dfa_vertices() );
        
        if( white(lab.node) )
        {
            put_dij_node(lab);
            set_grey(lab.node);
            
            return true;
        }
        else if ( grey(lab.node) && lab.cost < (*handle(lab.node)).cost )
        {
            BOOST_ASSERT( (*handle(lab.node)).node == lab.node );
            (*handle(lab.node)) = lab;
            BOOST_ASSERT( (*handle(lab.node)).cost == lab.cost );
            
            heap.update(handle(lab.node));

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
    virtual Label label(RLC::Vertice vert, int time, int cost, int source = -1) const {
        BOOST_ASSERT( vert.first >= 0 );
        BOOST_ASSERT( vert.second >= 0 );
        BOOST_ASSERT( cost >= 0 );
        return Label(vert, time, cost, source);
    }
    
    /**
     * Heap in which the Vertices will be stored
     */
    DRegHeap heap;
    
    virtual inline int best_cost_in_heap() { 
        Label best = heap.top();
        return best.cost + best.h; 
    }
    
    inline void put_dij_node(const Label l) { references[l.node.second][l.node.first] = heap.push(l); }
    
    inline DRegHeap::handle_type handle(const RLC::Vertice v) const { return references[v.second][v.first]; }
    
    inline bool white(const RLC::Vertice v) const { return status[v.second][v.first] == 0; }
    inline bool grey(const RLC::Vertice v) const { return status[v.second][v.first] == 1; }
    inline bool black(const RLC::Vertice v) const { return status[v.second][v.first] == 2; }
    inline void set_white(const RLC::Vertice v) { status[v.second][v.first] = 0; }
    inline void set_grey(const RLC::Vertice v) { status[v.second][v.first] = 1; }
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
    uint **status; //TODO : very big for only two bits ...
};





} // end namespace RLC

#endif