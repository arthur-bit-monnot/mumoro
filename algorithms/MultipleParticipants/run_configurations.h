#ifndef MUPARO_RUN_CONFIGURATIONS
#define MUPARO_RUN_CONFIGURATIONS


#include "MuparoTypedefs.h"
#include "node_filter_utils.h"
#include <AspectTargetAreaLandmark.h>
#include "AspectTargetAreaStop.h"
#include "../MultiObjectives/Martins.h"

using RLC::DRegLC;
using RLC::AspectCount;
using RLC::LandmarkSet;


namespace MuPaRo {

AlgoMPR::PtToPt * point_to_point( const Transport::Graph * trans, int source, int dest, RLC::DFA dfa = RLC::pt_foot_dfa() );

VisualResult show_point_to_point( const Transport::Graph * trans, int source, int dest, RLC::DFA dfa = RLC::pt_foot_dfa() );

AlgoMPR::SharedPath * shared_path( const Transport::Graph * trans, int src1, int src2, int dest, RLC::DFA dfa = RLC::bike_pt_dfa() );

VisualResult show_shared_path( const Transport::Graph * trans, int src1, int src2, int dest);

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
    
    RLC::Graph *g1 = new RLC::Graph(cs->transport, dfa_ped );
    RLC::Graph *g2 = new RLC::Graph(cs->transport, dfa_car );
    RLC::Graph *g3 = new RLC::Graph(cs->transport, dfa_car );
    RLC::BackwardGraph *g4 = new RLC::BackwardGraph(g2);
    RLC::Graph *g5 = new RLC::Graph(cs->transport, dfa_ped );
    
    cs->graphs.push_back( g1 );
    cs->graphs.push_back( g2 );
    cs->graphs.push_back( g3 );
    cs->graphs.push_back( g4 );
    cs->graphs.push_back( g5 );
    cs->dij.push_back( new typename T::Dijkstra( typename T::Dijkstra::ParamType(RLC::DRegLCParams(g1, day, 1)) ) );
    cs->dij.push_back( new typename T::Dijkstra( typename T::Dijkstra::ParamType(RLC::DRegLCParams(g2, day, 1)) ) );
    cs->dij.push_back( new typename T::Dijkstra( typename T::Dijkstra::ParamType(RLC::DRegLCParams(g3, day, 2)) ) );
    cs->dij.push_back( new typename T::Dijkstra( typename T::Dijkstra::ParamType(RLC::DRegLCParams(g4, day, 1)) ) );
//     cs->dij.push_back( new typename T::Dijkstra( typename T::Dijkstra::ParamType(RLC::DRegLCParams(g5, day, 1)) ) );
    cs->dij.push_back( new RLC::Martins(g5, dest_ped, day) );
    
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
    
    RLC::Graph *g1 = new RLC::Graph(cs->transport, dfa_ped );
    RLC::Graph *g2 = new RLC::Graph(cs->transport, dfa_car );
    RLC::Graph *g3 = new RLC::Graph(cs->transport, dfa_car );
    RLC::BackwardGraph *g4 = new RLC::BackwardGraph(g2);
    RLC::Graph *g5 = new RLC::Graph(cs->transport, dfa_ped );
    
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

template<typename T>
void init_car_sharing_with_areas(T * cs, const Transport::Graph* trans, int src_ped, int src_car, int dest_ped, 
                 int dest_car, RLC::DFA dfa_ped, RLC::DFA dfa_car, Area * area_start, Area * area_dest, 
                 bool use_landmarks = false, LandmarkSet * h_start = NULL, LandmarkSet * h_dest = NULL )
{
    typedef RLC::AspectTargetAreaStop<RLC::AspectCount<RLC::DRegLC>> CarAlgo;
    typedef typename RLC::AspectTargetAreaStop<RLC::AspectTargetAreaLandmark<RLC::AspectCount<RLC::DRegLC>>> CarAlgoLM;
    typedef RLC::AspectNodePruning<RLC::AspectCount<RLC::DRegLC>> PassAlgo;
    
    cs->vres.a_nodes.push_back(src_ped);
    cs->vres.a_nodes.push_back(src_car);
    cs->vres.b_nodes.push_back(dest_ped);
    cs->vres.b_nodes.push_back(dest_car);
    
    int day = 10;
    int time = 50000;
    
    RLC::Graph *g1 = new RLC::Graph(cs->transport, dfa_ped );
    RLC::Graph *g2 = new RLC::Graph(cs->transport, dfa_car );
    RLC::Graph *g3 = new RLC::Graph(cs->transport, dfa_car );
    RLC::BackwardGraph *g4 = new RLC::BackwardGraph(g2);
    RLC::Graph *g5 = new RLC::Graph(cs->transport, dfa_ped );
    
    cs->graphs.push_back( g1 );
    cs->graphs.push_back( g2 );
    cs->graphs.push_back( g3 );
    cs->graphs.push_back( g4 );
    cs->graphs.push_back( g5 );
    cs->dij.push_back( new PassAlgo( 
        PassAlgo::ParamType(
            RLC::DRegLCParams(g1, day, 1),
            RLC::AspectNodePruningParams( &area_start->ns ) ) ) );
    if(!use_landmarks) {
        cs->dij.push_back( new CarAlgo( 
            CarAlgo::ParamType(
                RLC::DRegLCParams(g2, day, 1),
                RLC::AspectTargetAreaStopParams(area_start) ) ) );
        cs->dij.push_back( new CarAlgo( 
            CarAlgo::ParamType(
                RLC::DRegLCParams(g3, day, 2),
                RLC::AspectTargetAreaStopParams(area_dest) ) ) );
        cs->dij.push_back( new CarAlgo( 
            CarAlgo::ParamType(
                RLC::DRegLCParams(g4, day, 1),
                RLC::AspectTargetAreaStopParams(area_dest) ) ) );
    } else {
        cs->dij.push_back( new CarAlgoLM( 
            typename CarAlgoLM::ParamType(
                RLC::DRegLCParams(g2, day, 1),
                RLC::AspectTargetAreaLandmarkParams<>(area_start, h_start),
                RLC::AspectTargetAreaStopParams(area_start) ) ) );
        cs->dij.push_back( new CarAlgoLM( 
            typename CarAlgoLM::ParamType(
                RLC::DRegLCParams(g3, day, 2),
                RLC::AspectTargetAreaLandmarkParams<>(area_dest, h_dest),
                RLC::AspectTargetAreaStopParams(area_dest) ) ) );
        cs->dij.push_back( new CarAlgoLM( 
            typename CarAlgoLM::ParamType(
                RLC::DRegLCParams(g4, day, 1),
                RLC::AspectTargetAreaLandmarkParams<>(area_dest, h_dest),
                RLC::AspectTargetAreaStopParams(area_dest) ) ) );
    }
    cs->dij.push_back( new PassAlgo( 
        PassAlgo::ParamType(
            RLC::DRegLCParams(g5, day, 1),
            RLC::AspectNodePruningParams( &area_dest->ns ) ) ) );
    
    cs->insert( StateFreeNode(0, src_ped), time, 0);
    cs->insert( StateFreeNode(1, src_car), time, 0);
    cs->insert( StateFreeNode(3, dest_car), 0, 0);
}

template<typename T>
void init_multi_car_sharing_with_areas(T * cs, const Transport::Graph* trans, int src_ped, int src_car, int dest_ped, 
                 int dest_car, RLC::DFA dfa_ped, RLC::DFA dfa_car, Area * area_start, Area * area_dest, 
                 bool use_landmarks = false, LandmarkSet * h_start = NULL, LandmarkSet * h_dest = NULL )
{
    typedef RLC::AspectTargetAreaStop<RLC::AspectCount<RLC::DRegLC>> CarAlgo;
    typedef typename RLC::AspectTargetAreaStop<RLC::AspectTargetAreaLandmark<RLC::AspectCount<RLC::DRegLC>>> CarAlgoLM;
    typedef RLC::AspectNodePruning<RLC::AspectCount<RLC::DRegLC>> PassAlgo;
    
    cs->vres.a_nodes.push_back(src_ped);
    cs->vres.a_nodes.push_back(src_car);
    cs->vres.b_nodes.push_back(dest_ped);
    cs->vres.b_nodes.push_back(dest_car);
    
    int day = 10;
    int time = 50000;
    
    RLC::Graph *g1 = new RLC::Graph(cs->transport, dfa_ped );
    RLC::Graph *g2 = new RLC::Graph(cs->transport, dfa_car );
    RLC::Graph *g3 = new RLC::Graph(cs->transport, dfa_car );
    RLC::BackwardGraph *g4 = new RLC::BackwardGraph(g2);
    RLC::Graph *g5 = new RLC::Graph(cs->transport, dfa_ped );
    
    cs->graphs.push_back( g1 );
    cs->graphs.push_back( g2 );
    cs->graphs.push_back( g3 );
    cs->graphs.push_back( g4 );
    cs->graphs.push_back( g5 );
    cs->dij.push_back( new PassAlgo( 
        PassAlgo::ParamType(
            RLC::DRegLCParams(g1, day, 1),
            RLC::AspectNodePruningParams( &area_start->ns ) ) ) );
    if(!use_landmarks) {
        cs->dij.push_back( new CarAlgo( 
            CarAlgo::ParamType(
                RLC::DRegLCParams(g2, day, 1),
                RLC::AspectTargetAreaStopParams(area_start) ) ) );
        cs->dij.push_back( new CarAlgo( 
            CarAlgo::ParamType(
                RLC::DRegLCParams(g3, day, 2),
                RLC::AspectTargetAreaStopParams(area_dest) ) ) );
        cs->dij.push_back( new CarAlgo( 
            CarAlgo::ParamType(
                RLC::DRegLCParams(g4, day, 1),
                RLC::AspectTargetAreaStopParams(area_dest) ) ) );
    } else {
        cs->dij.push_back( new CarAlgoLM( 
            typename CarAlgoLM::ParamType(
                RLC::DRegLCParams(g2, day, 1),
                RLC::AspectTargetAreaLandmarkParams<>(area_start, h_start),
                RLC::AspectTargetAreaStopParams(area_start) ) ) );
        cs->dij.push_back( new CarAlgoLM( 
            typename CarAlgoLM::ParamType(
                RLC::DRegLCParams(g3, day, 2),
                RLC::AspectTargetAreaLandmarkParams<>(area_dest, h_dest),
                RLC::AspectTargetAreaStopParams(area_dest) ) ) );
        cs->dij.push_back( new CarAlgoLM( 
            typename CarAlgoLM::ParamType(
                RLC::DRegLCParams(g4, day, 1),
                RLC::AspectTargetAreaLandmarkParams<>(area_dest, h_dest),
                RLC::AspectTargetAreaStopParams(area_dest) ) ) );
    }
    /*
    cs->dij.push_back( new PassAlgo( 
        PassAlgo::ParamType(
            RLC::DRegLCParams(g5, day, 1),
            RLC::AspectNodePruningParams( &area_dest->ns ) ) ) );
    */
    cs->dij.push_back( new RLC::Martins(g5, dest_ped, day, area_dest) );
    
    cs->insert( StateFreeNode(0, src_ped), time, 0);
    cs->insert( StateFreeNode(1, src_car), time, 0);
    cs->insert( StateFreeNode(3, dest_car), 0, 0);
}


}


#endif
