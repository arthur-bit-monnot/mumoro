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
    
    void summary() {
        int count_set = 0;
        int tot_labels = 0;
        int printed = 0;
        BOOST_FOREACH( std::list<Label> & labels, P ) {
            if( ! labels.empty() ) {
                count_set++;
                tot_labels += labels.size();
                if( printed < 1) {
                    BOOST_FOREACH( Label l, labels ) {
                        cout << l <<endl;
                    }
                    cout << endl <<endl;
                    printed ++;
                }
            }
        }
        
        cout << "Num set: " << count_set <<" - tot labels: "<<tot_labels<<" - moy labels: " <<((float)tot_labels / (float)count_set) << endl;
    }
    
    
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
        total_iter++;
        Label lab = next_undominated();
        heap.pop();
        
        BOOST_ASSERT( !is_dominated(lab) );
        
//         if( (total_iter % 100000) == 0 )
//             summary();
        
        P[lab.node.first].push_back( lab );
        
        if( lab.node.first == target ) {
            success = true;
            cout << "Found destination with cost " << lab.cost <<endl;
            cout << undominated_iter <<" / " << total_iter <<endl;
            summary();
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
    
    virtual bool insert_node(const Vertice & vert, const int arrival, const int vert_cost) override {
        return insert_node( vert, arrival, vert_cost, vert.first );
    }
    
    bool insert_node(const Vertice & vert, const int arrival, const int vert_cost, const int source ) {
        if(area == NULL || area->isIn( vert.first ) ) {
            Label l( vert, arrival, vert_cost, source);
            //TODO: check domination
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