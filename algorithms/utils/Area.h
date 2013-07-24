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





#endif