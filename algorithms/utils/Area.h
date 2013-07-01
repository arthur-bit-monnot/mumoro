#ifndef AREA_H
#define AREA_H

#include <vector>
#include <nodes_filter.h>
#include "node_filter_utils.h"

namespace RLC {
    class Landmark;
}



class Area
{
public:
    
    std::vector<int> nodes;
    NodeSet ns;
    BBNodeFilter * bb = NULL;
    const Transport::Graph * g;
    int center;
    int diameter;
    int radius;
    int num_car_accessible;
    
    int id;
    
    Area( const Transport::Graph * g, const int size ) : ns(size), g(g) {
        static int next_id = 0;
        id = next_id++;
    }
    ~Area() { delete bb; }
    
    void add_node( const int n ) {
        nodes.push_back( n );
        ns.addNode( n );
    }
    
    inline int size() const { return nodes.size(); }
    inline int get( const int i ) const { return nodes[i]; }
    const std::vector<int> & get_nodes () const { return nodes; }
    
    inline bool isIn( const int node ) const { return ns.isIn( node ); }
    
    inline NodeFilter * geo_filter() {
        if( bb == NULL )
            init();
        return new BBNodeFilter(*bb);
    }

    void init() {
        bb = rectangle_containing(g, nodes, 1.0);
        init_num_car_accessible();
    }
    
    VisualResult get_res() { return vres; }
    
private:    
    void init_num_car_accessible();

    VisualResult vres;
};


Area * build_area_around ( const Transport::Graph * g, int start, int end, int max_cost, RLC::DFA dfa = RLC::pt_foot_dfa() );

Area * build_area_around_with_start_time ( const Transport::Graph * g, int start, int end, int start_time, int max_cost, RLC::DFA dfa = RLC::pt_foot_dfa() );

Area * toulouse_area ( const Transport::Graph * g );
Area * toulouse_area_small ( const Transport::Graph * g );
Area * bordeaux_area ( const Transport::Graph * g );
Area * bordeaux_area_small ( const Transport::Graph * g );
Area * small_area ( const Transport::Graph * g );




#endif