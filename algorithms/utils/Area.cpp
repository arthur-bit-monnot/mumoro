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

#include "Area.h"

#include <reglc_graph.h>
#include <AlgoTypedefs.h>

using namespace RLC;


void Area::init_num_car_accessible()
{
    num_car_accessible = 0;
    BOOST_FOREACH(int node, this->nodes) {
        if( this->g->car_accessible( node ) )
            num_car_accessible++;
    }
    
    cout << "Num car accessible : " << num_car_accessible <<endl;
}


Area * build_area_around ( const Transport::Graph* trans, int start, int end, int max_cost, DFA dfa )
{
    Area * area = new Area(trans, trans->num_vertices());
    
    RLC::Graph g(trans, dfa);
    
    
    typedef AspectMaxCostPruning<AspectMinCost<DRegLC> > Dij;
    
    Dij::ParamType p(
        DRegLCParams( &g, 0, 1 ),
        AspectMaxCostPruningParams( max_cost )
    );
    
    Dij dij( p );
    for(int i=start ; i <= end ; ++i) {
        BOOST_FOREACH( int state, g.dfa_start_states() ) {
            dij.add_source_node( RLC::Vertice(i, state), 0, 0 );
        }
    }
    while( !dij.finished() ) {
        RLC::Label lab = dij.treat_next();
        area->add_node( lab.node.first );
    }
    
    return area;
}

Area * build_area_around_with_start_time ( const Transport::Graph* trans, int start, int end, int start_time, int max_cost, DFA dfa )
{
    Area * area = new Area(trans, trans->num_vertices());
    
    RLC::Graph g(trans, dfa);
    
    
    typedef AspectMaxCostPruning<DRegLC> Dij;
    
    Dij::ParamType p(
        DRegLCParams( &g, 0, 1 ),
        AspectMaxCostPruningParams( max_cost )
    );
    
    Dij dij( p );
    for(int i=start ; i <= end ; ++i) {
        BOOST_FOREACH( int state, g.dfa_start_states() ) {
            dij.add_source_node( RLC::Vertice(i, state), start_time, 0 );
        }
    }
    while( !dij.finished() ) {
        RLC::Label lab = dij.treat_next();
        area->add_node( lab.node.first );
    }
    
    return area;
}

/*** NOTE : the areas defined here after are dependent on the graph 
 * They are used for the `sud-ouest` configuration
 */

Area * toulouse_area ( const Transport::Graph* g )
{
    Area * area = build_area_around( g, 608327, 618191, 600);
    area->init();
    return area;
}

Area * toulouse_area_small ( const Transport::Graph* g )
{
    Area * area = build_area_around( g, 608327, 618191, 600);
    area->init();
    return area;
}

Area * bordeaux_area ( const Transport::Graph* g )
{
    Area * area = build_area_around( g, 618192, 629765, 600);
    area->init();
    return area;
}

Area * bordeaux_area_small ( const Transport::Graph* g )
{
    Area * area = build_area_around( g, 629421, 629765, 600);
    area->init();
    return area;
}
