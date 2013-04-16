#include <stdlib.h>
#include <iostream>
using std::cerr;
using std::cout;
using std::endl;
#include <fstream>
using std::ifstream;

#include "muparo.h"
#include "run_configurations.h"
#include "utils.h"
#include "node_filter_utils.h"


using namespace MuPaRo;
using namespace AlgoMPR;

RLC::DFA * dfas[2];

void run_test(std::string directory, Transport::Graph * trans, int car_start_node, int passenger_start_node, int car_arrival_node,
              int passenger_arrival_node, int time, int day, RLC::DFA dfa_car, RLC::DFA dfa_passenger)
{
    {
        typedef CarSharingTest CurrAlgo;
        
        cout << "unrestricted: { "<<endl;
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
        cout << " init-time: " << RUNTIME;
        START_TICKING;
        cs.run();
        STOP_TICKING;
        
        cout << ",\n run-time: " <<RUNTIME;
        cout << ",\n visited-nodes: "<<cs.count;
        cout << ",\n visited-per-layer: [";
        BOOST_FOREACH( CurrAlgo::Dijkstra * dij, cs.dij )
            cout << dij->count <<", ";
        cout <<"]";
        cout << ",\n solution-cost: "<<cs.solution_cost();
        cout << endl << "}" << endl;
    }
    
    
    {
        BBNodeFilter * toulouse = toulouse_bb(trans);
        BBNodeFilter * bordeaux = bordeaux_bb(trans);
        
        cout << "restricted: { "<<endl;
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
        cout << " init-time: " << RUNTIME;
        START_TICKING;
        cs.run();
        STOP_TICKING;
        
        cout << ",\n run-time: " <<RUNTIME;
        cout << ",\n visited-nodes: "<<cs.count;
        cout << ",\n visited-per-layer: [";
        BOOST_FOREACH( CarSharingFilteredTest::Dijkstra * dij, cs.dij )
            cout << dij->count <<", ";
        cout <<"]";
        cout << ",\n solution-cost: "<<cs.solution_cost();
        cout << endl << "}" << endl;
    }
    cout << endl;
}

void hundred_inits(Transport::Graph * trans) {
    AlgoMPR::CarSharing * mup;
    for(int i=0 ; i<100 ; ++i) {
        mup = car_sharing(trans, 0, 0, 0, 0, *dfas[1], *dfas[0]);
        delete mup;
    }
}


int main(void)
{
    dfas[0] = new RLC::DFA(RLC::pt_foot_dfa());
    dfas[1] = new RLC::DFA(RLC::car_dfa());
    
    std::string name, dump_file;
    
    char buff[255];
    int car_start_node, passenger_start_node, car_arrival_node, passenger_arrival_node, time, day, dfa_car, dfa_passenger;
    
    ifstream indata; // indata is like cin
    
    indata.open("/home/arthur/LAAS/mumoro/algorithms/tests/basic_test.conf"); // opens the file
    if(!indata) { // file couldn't be opened
        cerr << "Error: file could not be opened" << endl;
        exit(1);
    }
    
    char look_ahead;
    while( (look_ahead = indata.peek()) == '#' ) {
        indata.getline(buff, 255);
//         cout << buff;
    }
    indata.putback(look_ahead);
    
    double total = 0;
    int num_tests = 0;
    
    char * test_name ;
    
    indata >> name;
    indata >> dump_file;
    Transport::Graph * transport = new Transport::Graph("/home/arthur/LAAS/Data/Graphs/" + dump_file);
  
    /*
    double start, end;
    START_TICKING;
    hundred_inits(transport);
    STOP_TICKING;
    cout << ",\n time = " <<RUNTIME;
    */
    while ( !indata.eof() ) { // keep reading until end-of-file
        indata >> passenger_start_node >> car_start_node >> passenger_arrival_node >> car_arrival_node
               >> time >> day >> dfa_car >> dfa_passenger;
        run_test("1", transport, car_start_node, passenger_start_node, car_arrival_node, passenger_arrival_node, 
                 time, day, *dfas[dfa_car], *dfas[dfa_passenger]);
        ++num_tests;
    }
    
}