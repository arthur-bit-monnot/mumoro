#ifndef MARTINS_H
#define MARTINS_H

#include <iostream>
#include <boost/heap/fibonacci_heap.hpp>
#include <boost/heap/d_ary_heap.hpp>
#include <boost/foreach.hpp> 
#include <boost/dynamic_bitset.hpp>
#include <reglc_graph.h>


/**
 * This file contains an adaptation of the Many-to-one algorithm for time-dependent networks
 */


using namespace std;

struct Label {
  Label( int node, int time, int cost) : cost(cost), time(time), node(node) {}
  int cost;
  int time;
  int node;
  
  bool operator<(const Label & rhs) const { return cost > rhs.cost; }
  bool dominated_by(const Label & other) const { 
      return (cost >= other.cost && time >= other.time)
          || ((cost >= other.cost) && (cost - other.cost > other.time - time));
  }
};

ostream & operator<<(ostream & os, Label l)
{
    os << "Label[" << l.node << " " << l.time<<" "<<l.cost<< "]";
    return os;
}


typedef boost::heap::d_ary_heap<
    Label, 
    boost::heap::arity<4>,
    boost::heap::mutable_<true> > Heap;


int martins( RLC::AbstractGraph * graph, std::list<Label> starts, const int target ) {
    int total_iter = 0, undominated_iter = 0;
    int day = 10;
    
    Heap heap;
    std::vector<std::list<Label>> P;
    P.resize(graph->num_transport_vertices());
    
    BOOST_FOREACH( Label l, starts ) {
        heap.push( l );
    }
    
    while( !heap.empty() ) {
        total_iter++;
        Label lab = heap.top();
        heap.pop();
        
        bool is_dominated = false;
        BOOST_FOREACH( Label & ref, P[lab.node] ) {
            is_dominated |= lab.dominated_by( ref );
        }
        if( is_dominated )
            continue;
        
        undominated_iter++;
        
        P[lab.node].push_back( lab );
        
        if( lab.node == target ) {
            cout << "End: " << lab.cost <<" - Total iters: "<< total_iter<<" - undominated iters: "<<undominated_iter<<endl;
            break;
        }
        
        RLC::Vertice curr( lab.node, 0 );
        
        
        std::list<RLC::Edge> n_out_edges = graph->out_edges(curr);
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
                    Label v( target.first, target_arr, target_cost);
                    //TODO: check domination
                    heap.push( v );
                }
            }
            
            
        }
        
        
    }
}



#endif