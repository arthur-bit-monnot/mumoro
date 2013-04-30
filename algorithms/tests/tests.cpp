#include <stdlib.h>
#include <iostream>
using std::cerr;
using std::cout;
using std::endl;
#include <fstream>
using std::ifstream;
#include <time.h>

#include "GraphFactory.h"
#include "muparo.h"
#include "run_configurations.h"
#include "utils.h"
#include "node_filter_utils.h"
#include "../RegLC/AlgoTypedefs.h"
#include "Landmark.h"

#include "JsonWriter.h"


using namespace MuPaRo;
using namespace AlgoMPR;

RLC::DFA * dfas[2];
JsonWriter * out;
Area * toulouse;
Area * bordeaux;
Landmark * toulouse_lm;
Landmark * bordeaux_lm;

void run_test(std::string directory, const Transport::Graph * trans, int car_start_node, int passenger_start_node, int car_arrival_node,
              int passenger_arrival_node, int time, int day, RLC::DFA dfa_car, RLC::DFA dfa_passenger)
{
    out->step_in();
    
    out->add("car-start", car_start_node);
    out->add("passenger-start", passenger_start_node);
    out->add("car-arrival", car_arrival_node);
    out->add("passenger-arrival", passenger_arrival_node);
    out->add("time", time);
    out->add("day", day);
    
    
    {
        typedef CarSharingTest CurrAlgo;
        
        out->step_in("unrestricted");

        START_TICKING;
        CurrAlgo::ParamType p(
            MuparoParams( trans, 5 ),
            AspectTargetParams( 4, passenger_arrival_node ),
            AspectPropagationRuleParams( SumCost, MaxArrival, 2, 0, 1),
            AspectPropagationRuleParams( SumCost, FirstLayerArrival, 4, 2, 3)
        );
        
        CurrAlgo cs( p );
        
        init_car_sharing<CurrAlgo>( &cs, trans, passenger_start_node, car_start_node, passenger_arrival_node, car_arrival_node, dfa_passenger, dfa_car );

        STOP_TICKING;
        out->add("init-time", RUNTIME);
        
        START_TICKING;
        cs.run();
        STOP_TICKING;
        
        out->add("runtime", RUNTIME);
        out->add("visited-nodes", cs.count);
        std::vector<int> per_layer;
        for(int i=0 ; i<cs.num_layers ; ++i) {
            per_layer.push_back( cs.dij[i]->count );
        }
        out->add("visited-per-layer", per_layer);
        out->add("solution-cost", cs.solution_cost());

        
        out->step_out();
    }
    
    /*
    {
        BBNodeFilter * toulouse = toulouse_bb(trans);
        BBNodeFilter * bordeaux = bordeaux_bb(trans);
        
        out->step_in("restricted");

        START_TICKING;
        CarSharingFilteredTest::ParamType p(
            MuparoParams( trans, 5 ),
            AspectTargetParams( 4, passenger_arrival_node ),
            AspectPropagationRuleParams( SumCost, MaxArrival, 2, 0, 1),
            AspectPropagationRuleParams( SumCost, FirstLayerArrival, 4, 2, 3)
        );
        
        std::vector<NodeFilter*> filters;
        RLC::Graph g1(trans, dfa_passenger );
        RLC::BackwardGraph g2(&g1);
        
        if( toulouse->isIn(passenger_start_node) )
            filters.push_back( toulouse_bb(trans) );
        else if( bordeaux->isIn(passenger_start_node) )
            filters.push_back( bordeaux_bb(trans) );
        else
            filters.push_back(isochrone(&g1, passenger_start_node, 600));
        
        filters.push_back( rectangle_containing( trans, car_start_node, car_arrival_node, 0.02) );
        filters.push_back( rectangle_containing( trans, car_start_node, car_arrival_node, 0.02) );
        filters.push_back( rectangle_containing( trans, car_start_node, car_arrival_node, 0.02) );
        
        if( toulouse->isIn(passenger_arrival_node) )
            filters.push_back( toulouse_bb(trans) );
        else if( bordeaux->isIn(passenger_arrival_node) )
            filters.push_back( bordeaux_bb(trans) );
        else
            filters.push_back(isochrone(&g2, passenger_arrival_node, 600));
        
        delete toulouse;
        delete bordeaux;
        
        CarSharingFilteredTest cs( p );
        
        init_car_sharing_filtered<CarSharingFilteredTest>( &cs, trans, passenger_start_node, car_start_node, passenger_arrival_node, car_arrival_node, dfa_passenger, dfa_car, filters );

        STOP_TICKING;
        out->add("init-time", RUNTIME);
        START_TICKING;
        cs.run();
        STOP_TICKING;
        
        out->add("runtime", RUNTIME);
        out->add("visited-nodes", cs.count);
        
        std::vector<int> per_layer;
        for(int i=0 ; i<cs.num_layers ; ++i) {
            per_layer.push_back( cs.dij[i]->count );
        }
        out->add("visited-per-layer", per_layer);
        
        out->add("solution-cost", cs.solution_cost());
        out->step_out();
    }
    */
    
    
    
    
    {
        out->step_in("stop-conditions");

        START_TICKING;
        CarSharingTest::ParamType p(
            MuparoParams( trans, 5 ),
            AspectTargetParams( 4, passenger_arrival_node ),
            AspectPropagationRuleParams( SumCost, MaxArrival, 2, 0, 1),
            AspectPropagationRuleParams( SumCost, FirstLayerArrival, 4, 2, 3)
        );
        
        std::vector<NodeFilter*> filters;
        RLC::Graph g1(trans, dfa_passenger );
        RLC::BackwardGraph g2(&g1);
        
        Area * area_start;
        Area * area_dest;
        
        if( toulouse->isIn(passenger_start_node) )
            area_start = toulouse;
        else if( bordeaux->isIn(passenger_start_node) )
            area_start = bordeaux;
        else
            return ;
        
        
        if( toulouse->isIn(passenger_arrival_node) )
            area_dest = toulouse;
        else if( bordeaux->isIn(passenger_arrival_node) )
            area_dest = bordeaux;
        else
            return ;

        
        CarSharingTest cs( p );
        
        init_car_sharing_with_areas<CarSharingTest>( &cs, trans, passenger_start_node, car_start_node, passenger_arrival_node, car_arrival_node, dfa_passenger, dfa_car, area_start, area_dest );

        STOP_TICKING;
        out->add("init-time", RUNTIME);
        START_TICKING;
        cs.run();
        STOP_TICKING;
        
        out->add("runtime", RUNTIME);
        out->add("visited-nodes", cs.count);
        
        std::vector<int> per_layer;
        for(int i=0 ; i<cs.num_layers ; ++i) {
            per_layer.push_back( cs.dij[i]->count );
        }
        out->add("visited-per-layer", per_layer);
        
        out->add("solution-cost", cs.solution_cost());
        out->step_out();
    }
    
    
    {
        out->step_in("stop-conditions-landmarks");

        START_TICKING;
        CarSharingTest::ParamType p(
            MuparoParams( trans, 5 ),
            AspectTargetParams( 4, passenger_arrival_node ),
            AspectPropagationRuleParams( SumCost, MaxArrival, 2, 0, 1),
            AspectPropagationRuleParams( SumCost, FirstLayerArrival, 4, 2, 3)
        );
        
        std::vector<NodeFilter*> filters;
        RLC::Graph g1(trans, dfa_passenger );
        RLC::BackwardGraph g2(&g1);
        
        Area * area_start, * area_dest;
        Landmark * lm_start, * lm_dest;
        
        if( toulouse->isIn(passenger_start_node) ) {
            area_start = toulouse;
            lm_start = toulouse_lm;
        } else if( bordeaux->isIn(passenger_start_node) ) {
            area_start = bordeaux;
            lm_start = bordeaux_lm;
        } else {
            return ;
        }
        
        if( toulouse->isIn(passenger_arrival_node) ) {
            area_dest = toulouse;
            lm_dest = toulouse_lm;
        } else if( bordeaux->isIn(passenger_arrival_node) ) {
            area_dest = bordeaux;
            lm_dest = bordeaux_lm;
        } else {
            return ;
        }

        
        CarSharingTest cs( p );
        
        init_car_sharing_with_areas<CarSharingTest>( &cs, trans, passenger_start_node, car_start_node, passenger_arrival_node, car_arrival_node, 
                                                     dfa_passenger, dfa_car, area_start, area_dest, 
                                                     true, lm_start, lm_dest );

        STOP_TICKING;
        out->add("init-time", RUNTIME);
        START_TICKING;
        cs.run();
        STOP_TICKING;
        
        out->add("runtime", RUNTIME);
        out->add("visited-nodes", cs.count);
        
        std::vector<int> per_layer;
        for(int i=0 ; i<cs.num_layers ; ++i) {
            per_layer.push_back( cs.dij[i]->count );
        }
        out->add("visited-per-layer", per_layer);
        
        out->add("solution-cost", cs.solution_cost());
        out->step_out();
    }
    
    out->step_out();
}

void hundred_inits(Transport::Graph * trans) {
    AlgoMPR::CarSharing * mup;
    for(int i=0 ; i<100 ; ++i) {
        mup = car_sharing(trans, 0, 0, 0, 0, *dfas[1], *dfas[0]);
        delete mup;
    }
}

void new_test(const Transport::Graph * g)
{
    int bordeaux1=-1, bordeaux2=-1, toulouse1=-1, toulouse2=-1;
    
    
    
    bordeaux1 = bordeaux->get( rand() % bordeaux->size() );
    bordeaux2 = bordeaux->get( rand() % bordeaux->size() );
    toulouse1 = toulouse->get( rand() % toulouse->size() );
    toulouse2 = toulouse->get( rand() % toulouse->size() );
    
    {
        const RLC::Graph pt_car(g, RLC::pt_car_dfa());
        Algo::PtToPt::ParamType p(
            RLC::DRegLCParams( &pt_car, 10 ), RLC::AspectTargetParams( bordeaux2 )    );
        Algo::PtToPt * dij = new Algo::PtToPt(p);
        dij->insert_node(RLC::Vertice(bordeaux1, 0), 0, 0);
        dij->run();
        if( !dij->success ) {
            delete dij;
            return new_test(g);
        }
        delete dij;
    }
    {
        const RLC::Graph pt_car(g, RLC::pt_car_dfa());
        Algo::PtToPt::ParamType p(
            RLC::DRegLCParams( &pt_car, 10 ), RLC::AspectTargetParams( toulouse2 )    );
        Algo::PtToPt * dij = new Algo::PtToPt(p);
        dij->insert_node(RLC::Vertice(toulouse1, 0), 0, 0);
        dij->run();
        if( !dij->success ) {
            delete dij;
            return new_test(g);
        }
        delete dij;
    }
    {
        const RLC::Graph pt_car(g, RLC::car_dfa());
        Algo::PtToPt::ParamType p(
            RLC::DRegLCParams( &pt_car, 10 ), RLC::AspectTargetParams( toulouse2 )    );
        Algo::PtToPt * dij = new Algo::PtToPt(p);
        dij->insert_node(RLC::Vertice(bordeaux2, 0), 0, 0);
        dij->run();
        if( !dij->success ) {
            delete dij;
            return new_test(g);
        }
        delete dij;
    }
    
    
    if( (rand()  % 2) == 0 )
        cout << bordeaux1 <<" "<< bordeaux2 <<" "<< toulouse1 <<" "<< toulouse2 <<" 32140 10 1 0" <<endl;
    else
        cout << toulouse1 <<" "<< toulouse2 <<" "<< bordeaux1 <<" "<< bordeaux2 <<" 32140 10 1 0" <<endl;
}


int main(void)
{
    bool small_areas = false;
    string config;
    if(small_areas) 
        config = "/home/arthur/LAAS/mumoro/algorithms/tests/smaller_areas.conf";
    else
        config = "/home/arthur/LAAS/mumoro/algorithms/tests/basic_test.conf";
     
    srand (time(NULL));
    
    dfas[0] = new RLC::DFA(RLC::pt_foot_dfa());
    dfas[1] = new RLC::DFA(RLC::car_dfa());
    
    std::string name, dump_file;
    
    char buff[255];
    int car_start_node, passenger_start_node, car_arrival_node, passenger_arrival_node, time, day, dfa_car, dfa_passenger;
    
    ifstream indata; // indata is like cin
    
    indata.open(config); // opens the file
    if(!indata) { // file couldn't be opened
        cerr << "Error: file could not be opened" << endl;
        exit(1);
    }
    
    while( indata.peek() == '#' ) {
        indata.getline(buff, 255);
    }
    
    indata >> name;
    indata >> dump_file;
    JsonWriter writer("/home/arthur/LAAS/Data/Results/" + name + ".txt");
    out = &writer;
    Transport::GraphFactory gf("/home/arthur/LAAS/Data/Graphs/" + dump_file);
    const Transport::Graph * transport = gf.get();
    
    if(small_areas) {
        toulouse = toulouse_area_small(transport);
        bordeaux = bordeaux_area_small(transport);
    } else {
        toulouse = toulouse_area(transport);
        bordeaux = bordeaux_area(transport);
    }
    toulouse_lm = RLC::create_car_landmark(transport, toulouse->center);
    bordeaux_lm = RLC::create_car_landmark(transport, bordeaux->center);
    
    cout << "R(Toulouse) : " << toulouse->radius << " ; R(Bordeaux) : " << bordeaux->radius <<endl;

    while ( !indata.eof() ) { // keep reading until end-of-file
        indata >> passenger_start_node >> car_start_node >> passenger_arrival_node >> car_arrival_node
               >> time >> day >> dfa_car >> dfa_passenger; 
        run_test("1", transport, car_start_node, passenger_start_node, car_arrival_node, passenger_arrival_node, 
                 time, day, *dfas[dfa_car], *dfas[dfa_passenger]);
    }
    
    
//      for(int i=0 ; i<20 ; ++i)
//          new_test(transport);
    
    
}