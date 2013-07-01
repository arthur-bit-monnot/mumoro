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

Area * toulouse_area ( const Transport::Graph* g )
{
    Area * area = build_area_around( g, 608327, 618191, 600);
    area->center = 329194;
    area->init();
    return area;
}

Area * toulouse_area_small ( const Transport::Graph* g )
{
    Area * area = build_area_around( g, 608327, 618191, 600);
    area->center = 329194;
    area->init();
    return area;
}

Area * bordeaux_area ( const Transport::Graph* g )
{
    Area * area = build_area_around( g, 618192, 629765, 600);
    area->center = 580025;
    area->init();
    return area;
}

Area * bordeaux_area_small ( const Transport::Graph* g )
{
    Area * area = build_area_around( g, 629421, 629765, 600);
    area->center = 379083;
    area->init();
    return area;
}

Area* small_area ( const Transport::Graph* g )
{
    Area * area = build_area_around( g, 251665, 251665, 120 );
    area->center = 251665;
    area->init();
    cout << "Radius : " << area->radius << "  count : " << area->size() <<endl;;
    return area;
}
