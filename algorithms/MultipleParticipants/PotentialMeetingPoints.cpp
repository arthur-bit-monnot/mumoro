

#include "PotentialMeetingPoints.h"
#include <Area.h>

using namespace MuPaRo;

typedef AspectPropagationRule<Muparo< Algo::Basic>> MeetingPoints;


VisualResult show_meeting_points( const Transport::Graph * g, int source, int time )
{
    int day = 10;
    
    MeetingPoints::ParamType p(
        MuparoParams(g, 4),
        AspectPropagationRuleParams(SumCost, MaxArrival, 3, 1, 2)
    );
    
    MeetingPoints mp( p );
    
    RLC::DFA dfa_pt = RLC::pt_foot_dfa();
    RLC::DFA dfa_car = RLC::car_dfa();
    
    mp.dfas.push_back(dfa_pt);
    mp.dfas.push_back(dfa_car);
    mp.dfas.push_back(dfa_car);
    mp.dfas.push_back(dfa_car);

    
    RLC::Graph *g1 = new RLC::Graph(mp.transport, mp.dfas[0] );
    RLC::Graph *g2 = new RLC::Graph(mp.transport, mp.dfas[1] );
    RLC::BackwardGraph *g3 = new RLC::BackwardGraph(g2);
    RLC::Graph *g4 = new RLC::Graph(mp.transport, mp.dfas[3] );
    
    mp.graphs.push_back( g1 );
    mp.graphs.push_back( g2 );
    mp.graphs.push_back( g3 );
    mp.graphs.push_back( g4 );
    mp.dij.push_back( new RLC::AspectMinCost<MeetingPoints::Dijkstra>(RLC::AspectMinCost<MeetingPoints::Dijkstra>::ParamType(RLC::DRegLCParams(g1, day, 1)) ) );
    mp.dij.push_back( new MeetingPoints::Dijkstra( MeetingPoints::Dijkstra::ParamType(RLC::DRegLCParams(g2, day, 2)) ) );
    mp.dij.push_back( new MeetingPoints::Dijkstra( MeetingPoints::Dijkstra::ParamType(RLC::DRegLCParams(g3, day, 1)) ) );
    mp.dij.push_back( new RLC::AspectNoRun<MeetingPoints::Dijkstra>( RLC::AspectNoRun<MeetingPoints::Dijkstra>::ParamType(RLC::DRegLCParams(g4, day, 1)) ) );
    
    mp.insert( StateFreeNode(0, source), time, 0);
    mp.insert( StateFreeNode(1, source), time, 0);
    mp.insert( StateFreeNode(2, source), time, 0);
    
    mp.run();
    
    VisualResult vres;
    
    int count = 0;
    
    float min_lon=99999.0f, min_lat=99999.0f, max_lon=-99999.0f, max_lat=-99999.0f;
    
    for(int i=0 ; i<g1->num_transport_vertices() ; ++i) {
        StateFreeNode n0(0, i);
        StateFreeNode n1(1, i);
        StateFreeNode n2(2, i);
        
        if(i == source)
            cout << "source\n";
        
        if( mp.is_node_set( n0 ) && mp.is_node_set( n1 ) && mp.is_node_set( n2 ) ) {
            if( mp.get_cost( n0 ) <= (mp.get_cost( n1 ) + mp.get_cost( n2 ) ) ) {
                vres.a_nodes.push_back( i );
                std::cout << i <<endl;
                ++count;
                
                if( g->longitude(i) < min_lon )
                    min_lon = g->longitude(i);
                if( g->longitude(i) > max_lon )
                    max_lon = g->longitude(i);
                if( g->latitude(i) < min_lat )
                    min_lat = g->latitude(i);
                if( g->latitude(i) > max_lat )
                    max_lat = g->latitude(i);
                
            }
        }
    }
    
    
    cout << "Number of potential meeting points : " <<count <<endl;
    cout << "BoundingBox : " << max_lon <<", "<< min_lon <<", "<< max_lat <<", "<< min_lat <<endl;
    
    return vres;
}

typedef Muparo< RLC::AspectNodePruning<RLC::AspectMinCost<Algo::Basic>>> MeetingPointsLB;

NodeSet * meeting_points( const Transport::Graph * g, int source, Area * area )
{
    int day = 10;
    
    MeetingPointsLB::ParamType p(
        MuparoParams(g, 3)
    );
    
    MeetingPointsLB mp( p );
    
    RLC::DFA dfa_pt = RLC::pt_foot_dfa();
    RLC::DFA dfa_car = RLC::car_dfa();
    
    mp.dfas.push_back(dfa_pt);
    mp.dfas.push_back(dfa_car);
    mp.dfas.push_back(dfa_car);

    
    RLC::Graph *g1 = new RLC::Graph(mp.transport, mp.dfas[0] );
    RLC::Graph *g2 = new RLC::Graph(mp.transport, mp.dfas[1] );
    RLC::BackwardGraph *g3 = new RLC::BackwardGraph(g2);
    
    mp.graphs.push_back( g1 );
    mp.graphs.push_back( g2 );
    mp.graphs.push_back( g3 );
    mp.dij.push_back( new RLC::AspectMinCost<MeetingPointsLB::Dijkstra>(RLC::AspectMinCost<MeetingPointsLB::Dijkstra>::ParamType(
        RLC::DRegLCParams(g1, day, 1),
        RLC::AspectNodePruningParams( area->geo_filter() ) ) ) );
    mp.dij.push_back( new MeetingPointsLB::Dijkstra( MeetingPointsLB::Dijkstra::ParamType(
        RLC::DRegLCParams(g2, day, 2),
        RLC::AspectNodePruningParams( area->geo_filter() ) ) ) );
    mp.dij.push_back( new MeetingPointsLB::Dijkstra( MeetingPointsLB::Dijkstra::ParamType(
        RLC::DRegLCParams(g3, day, 1),
        RLC::AspectNodePruningParams( area->geo_filter() ) ) ) );
    
    mp.insert( StateFreeNode(0, source), 0, 0);
    mp.insert( StateFreeNode(1, source), 0, 0);
    mp.insert( StateFreeNode(2, source), 0, 0);
    
    mp.run();
    
    NodeSet * ns = new NodeSet(g->num_vertices());
    
    for(int i=0 ; i<area->size() ; ++i) {
        int node = area->get(i);
        
        StateFreeNode n0(0, node);
        StateFreeNode n1(1, node);
        StateFreeNode n2(2, node);
        
        if(i == source)
            cout << "source\n";
        
        if( mp.is_node_set( n0 ) && mp.is_node_set( n1 ) && mp.is_node_set( n2 ) ) {
            if( mp.get_cost( n0 ) <= (mp.get_cost( n1 ) + mp.get_cost( n2 ) ) ) {
                ns->addNode(node);
            }
        }
    }
    return ns;
}