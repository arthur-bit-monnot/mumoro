#ifndef MUPARO_RUN_CONFIGURATIONS
#define MUPARO_RUN_CONFIGURATIONS


#include "MuparoTypedefs.h"
#include "node_filter_utils.h"

namespace MuPaRo {

AlgoMPR::PtToPt * point_to_point( Transport::Graph * trans, int source, int dest, RLC::DFA dfa = RLC::pt_foot_dfa() );

VisualResult show_point_to_point( Transport::Graph * trans, int source, int dest, RLC::DFA dfa = RLC::pt_foot_dfa());

AlgoMPR::SharedPath * shared_path(Transport::Graph * trans, int src1, int src2, int dest);

VisualResult show_shared_path(Transport::Graph * trans, int src1, int src2, int dest);

AlgoMPR::CarSharing * car_sharing(const Transport::Graph * trans, int src_ped, int src_car, int dest_ped, int dest_car,
                                  RLC::DFA dfa_ped, RLC::DFA dfa_car);

VisualResult show_car_sharing(const Transport::Graph * trans, int src_ped, int src_car, int dest_ped, int dest_car,
                                  RLC::DFA dfa_ped, RLC::DFA dfa_car);

// typedef CarSharing AlgoStruct;
template<typename T>
void init_car_sharing(T * cs, const Transport::Graph* trans, int src_ped, int src_car, int dest_ped, 
                 int dest_car, RLC::DFA dfa_ped, RLC::DFA dfa_car )
{
    cs->vres.a_nodes.push_back(src_ped);
    cs->vres.a_nodes.push_back(src_car);
    cs->vres.b_nodes.push_back(dest_ped);
    cs->vres.b_nodes.push_back(dest_car);
    
    int day = 10;
    int time = 50000;
    
    cs->dfas.push_back(dfa_ped);
    cs->dfas.push_back(dfa_car);
    cs->dfas.push_back(dfa_car);
    cs->dfas.push_back(dfa_car);
    cs->dfas.push_back(dfa_ped);

    
    RLC::Graph *g1 = new RLC::Graph(cs->transport, cs->dfas[0] );
    RLC::Graph *g2 = new RLC::Graph(cs->transport, cs->dfas[1] );
    RLC::Graph *g3 = new RLC::Graph(cs->transport, cs->dfas[2] );
    RLC::BackwardGraph *g4 = new RLC::BackwardGraph(g2);
    RLC::Graph *g5 = new RLC::Graph(cs->transport, cs->dfas[4] );
    
    cs->graphs.push_back( g1 );
    cs->graphs.push_back( g2 );
    cs->graphs.push_back( g3 );
    cs->graphs.push_back( g4 );
    cs->graphs.push_back( g5 );
    cs->dij.push_back( new typename T::Dijkstra( typename T::Dijkstra::ParamType(RLC::DRegLCParams(g1, day, 1)) ) );
    cs->dij.push_back( new typename T::Dijkstra( typename T::Dijkstra::ParamType(RLC::DRegLCParams(g2, day, 1)) ) );
    cs->dij.push_back( new typename T::Dijkstra( typename T::Dijkstra::ParamType(RLC::DRegLCParams(g3, day, 2)) ) );
    cs->dij.push_back( new typename T::Dijkstra( typename T::Dijkstra::ParamType(RLC::DRegLCParams(g4, day, 1)) ) );
    cs->dij.push_back( new typename T::Dijkstra( typename T::Dijkstra::ParamType(RLC::DRegLCParams(g5, day, 1)) ) );
    
    cs->insert( StateFreeNode(0, src_ped), time, 0);
    cs->insert( StateFreeNode(1, src_car), time, 0);
    cs->insert( StateFreeNode(3, dest_car), 0, 0);
}

template<typename T>
void init_car_sharing_filtered(T * cs, const Transport::Graph* trans, int src_ped, int src_car, int dest_ped, 
                 int dest_car, RLC::DFA dfa_ped, RLC::DFA dfa_car, std::vector<NodeFilter*> filters )
{
    cs->vres.a_nodes.push_back(src_ped);
    cs->vres.a_nodes.push_back(src_car);
    cs->vres.b_nodes.push_back(dest_ped);
    cs->vres.b_nodes.push_back(dest_car);
    
    int day = 10;
    int time = 50000;
    
    cs->dfas.push_back(dfa_ped);
    cs->dfas.push_back(dfa_car);
    cs->dfas.push_back(dfa_car);
    cs->dfas.push_back(dfa_car);
    cs->dfas.push_back(dfa_ped);

    
    RLC::Graph *g1 = new RLC::Graph(cs->transport, cs->dfas[0] );
    RLC::Graph *g2 = new RLC::Graph(cs->transport, cs->dfas[1] );
    RLC::Graph *g3 = new RLC::Graph(cs->transport, cs->dfas[2] );
    RLC::BackwardGraph *g4 = new RLC::BackwardGraph(g2);
    RLC::Graph *g5 = new RLC::Graph(cs->transport, cs->dfas[4] );
    
    cs->graphs.push_back( g1 );
    cs->graphs.push_back( g2 );
    cs->graphs.push_back( g3 );
    cs->graphs.push_back( g4 );
    cs->graphs.push_back( g5 );
    cs->dij.push_back( new typename T::Dijkstra( 
        typename T::Dijkstra::ParamType(
            RLC::DRegLCParams(g1, day, 1),
            RLC::AspectNodePruningParams( filters[0] ) ) ) );
    cs->dij.push_back( new typename T::Dijkstra( 
        typename T::Dijkstra::ParamType(
            RLC::DRegLCParams(g2, day, 1),
            RLC::AspectNodePruningParams( filters[1] ) ) ) );
    cs->dij.push_back( new typename T::Dijkstra( 
        typename T::Dijkstra::ParamType(
            RLC::DRegLCParams(g3, day, 2),
            RLC::AspectNodePruningParams( filters[2] ) ) ) );
    cs->dij.push_back( new typename T::Dijkstra( 
        typename T::Dijkstra::ParamType(
            RLC::DRegLCParams(g4, day, 1),
            RLC::AspectNodePruningParams( filters[3] ) ) ) );
    cs->dij.push_back( new typename T::Dijkstra( 
        typename T::Dijkstra::ParamType(
            RLC::DRegLCParams(g5, day, 1),
            RLC::AspectNodePruningParams( filters[4] ) ) ) );
    
    cs->insert( StateFreeNode(0, src_ped), time, 0);
    cs->insert( StateFreeNode(1, src_car), time, 0);
    cs->insert( StateFreeNode(3, dest_car), 0, 0);
}
/*
Muparo * bi_point_to_point(Transport::Graph * trans, int source, int dest);
Muparo * bidir_covoiturage(Transport::Graph * trans, int source1, int source2, int dest1, int dest2,
                     RLC::DFA dfa1, RLC::DFA dfa2, int limit = -1);
Muparo * time_dep_covoiturage(Transport::Graph * trans, int source1, int source2, int dest1, int dest2,
                              RLC::DFA dfa_pass, RLC::DFA dfa_car, int limit = -1);

Muparo * conv_time_dep_covoiturage(Transport::Graph * trans, int source1, int source2, int dest1, int dest2,
                              RLC::DFA dfa_pass, RLC::DFA dfa_car);

Muparo * covoiturage(Transport::Graph * trans, int source1, int source2, int dest1, int dest2,
                              RLC::DFA dfa_pass, RLC::DFA dfa_car );
*/

/**
 * Those functions allow to add further restrictions on a Muparo instance for car-sharing
 */
/*
void add_rectangle_restriction_on_car(Muparo * mup, Transport::Graph* trans, int source, int dest, float margin);
void add_isochrone_restriction_on_passenger(Muparo * mup, Transport::Graph* trans, int source, int dest, float max_time);
*/
}


#endif
