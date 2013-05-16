#include "Area.h"

#include <reglc_graph.h>
#include <AlgoTypedefs.h>
#include "../MultipleParticipants/PotentialMeetingPoints.h"

using namespace RLC;


void Area::init_max_dist()
{
    DFA car = RLC::car_dfa();
    
    RLC::Graph rlc(this->g, car);
    RLC::BackwardGraph rlc_back(&rlc);
    
    int max_cost_forward, max_cost_backward;
    RLC::Label max_node;
    {
        Algo::Filtered::ParamType p(
            RLC::DRegLCParams(&rlc, 0),
            RLC::AspectNodePruningParams(bb)
        );
        
        Algo::Filtered dij(p);
        dij.insert_node(Vertice(center, 0), 0, 0);
        
        RLC::Label last;
        max_cost_forward = 0;
        
        while( !dij.finished() ) {
            last = dij.treat_next();
            
            if(ns.isIn(last.node.first) && last.cost > max_cost_forward) {
                max_cost_forward = last.cost;
                max_node = last;
            }
        }
        
        RLC::Label curr = max_node;
        
        /*
        vres.edges.push_front(curr.first);
        while( dij.has_pred(curr) ) {
            vres.edges.push_front(g->edgeIndex( dij.get_pred( curr ).first ));
            curr = rlc.source(dij.get_pred( curr ));
        }
        */
    }    
    
    {
        Algo::Filtered::ParamType p(
            RLC::DRegLCParams(&rlc_back, 0),
            RLC::AspectNodePruningParams(bb)
        );
        
        Algo::Filtered dij(p);
        dij.insert_node(Vertice(center, 0), 0, 0);
        
        RLC::Label last;
        max_cost_backward = 0;
        
        while( !dij.finished() ) {
            last = dij.treat_next();
            if(ns.isIn(last.node.first) && last.cost > max_cost_backward) {
                max_cost_backward = last.cost;
            }
            
        }
    }
    
    
    this->diameter = max_cost_backward + max_cost_forward;
    this->radius = max_cost_backward > max_cost_forward ? max_cost_backward : max_cost_forward;
}

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
            dij.insert_node( RLC::Vertice(i, state), 0, 0 );
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


NodeSet * meeting_points_in_area( Area * area )
{
    NodeSet * base = new NodeSet(area->g->num_vertices());
    
    for(int i=0 ; i<area->size() && i < 9900 ; ++i) {
        cout << i <<endl;
        int node = area->get(i);
        if(node <  608327) {
            NodeSet * ns = meeting_points(area->g, node, area);
            
            base->add(ns->bitset);
            delete ns;
        }
    }
    
    int count = 0;
    for(int i=0 ; i < area->g->num_vertices() ; ++i) {
        if(base->isIn(i)) {
            count++;
        }
    }
    return base;
}