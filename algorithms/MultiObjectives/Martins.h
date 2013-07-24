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

#ifndef MARTINS_H
#define MARTINS_H

#include <iostream>
#include <boost/heap/fibonacci_heap.hpp>
#include <boost/heap/d_ary_heap.hpp>
#include <boost/foreach.hpp> 
#include <boost/dynamic_bitset.hpp>
#include <reglc_graph.h>
#include "LabelSettingAlgo.h"
#include <Area.h>


/**
 * This file contains an adaptation of the Many-to-one algorithm for time-dependent networks
 */

struct NotImplemented {};

using namespace std;

namespace RLC {


typedef boost::heap::d_ary_heap<
    Label, 
    boost::heap::arity<4>,
    boost::heap::mutable_<true> > Heap;

class Martins : public LabelSettingAlgo
{
public:
    int total_iter = 0, undominated_iter = 0;
    
    const RLC::AbstractGraph * graph;
    int target;
    int day;
    const Area * area;
    bool success = false;
    
    Heap heap;
    std::vector<std::list<Label>> P;
    
    // results
    Label target_label;
    
    Martins( const RLC::AbstractGraph * rlc, const int target, const int day, const Area * area = NULL ) : 
    graph(rlc), 
    target(target), 
    day(day),
    area(area)
    {
        P.resize(graph->num_transport_vertices());
    }
    
    virtual bool finished() const override {
        return heap.empty() || success;
    }
    
    virtual bool run() override {
        while( !finished() ) {
            treat_next();
        }
        return success;
    }
    
    virtual Label treat_next() override { 
        count++;
        Label lab = next_undominated();
        heap.pop();
        
        BOOST_ASSERT( !is_dominated(lab) );
        
        P[lab.node.first].push_back( lab );
        
        if( lab.node.first == target ) {
            success = true;
            target_label = lab;
//             cout << "Found destination with cost " << lab.cost <<endl;
            return lab;
        }
        
        std::list<RLC::Edge> n_out_edges = graph->out_edges(lab.node);
        BOOST_FOREACH(RLC::Edge e, n_out_edges) 
        {
            RLC::Vertice target = graph->target(e);
            
            bool has_traffic;
            int edge_cost;
            boost::tie(has_traffic, edge_cost) = graph->duration(e, lab.time, day);

            int target_cost = lab.cost + edge_cost;
            
            // ignore the edge if it provokes an overflow
            if(target_cost < lab.cost)
                has_traffic = false;
            
            float target_arr;
            if(graph->forward)
                target_arr = lab.time + edge_cost;
            else 
                target_arr = lab.time - edge_cost;
            
            if(has_traffic) {
                if(edge_cost >= 0) {
                    insert_node(RLC::Vertice(target.first, 0), target_arr, target_cost, lab.source );
                }
            }
        }
        return lab;
    }
    
    bool insert_node(const Vertice & vert, const int arrival, const int vert_cost, const int source ) override {
        if(area == NULL || area->isIn( vert.first ) ) {
            Label l( vert, arrival, vert_cost, source);
            heap.push( l );
            return true;
        } else {
            return false;
        }
    }
    
    bool is_dominated( const Label & lab ) const {
        bool is_dominated = false;
        BOOST_FOREACH( const Label & ref, P[lab.node.first] ) {
            is_dominated |= lab.dominated_by( ref );
        }
        return is_dominated;
    }
    
    /**
     * Return first undominated label in heap. All dominated labels encountered are removed.
     * 
     * This function is to be used in place of heap.top()
     */
    Label next_undominated() {
        while( is_dominated( heap.top() ) ) {
            heap.pop();
        }
        return heap.top();
    }

    
    virtual int best_cost_in_heap() override { return next_undominated().cost; }
    
};


}


#endif