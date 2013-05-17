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
#include "../MultipleParticipants/MPR_AspectConnectionRule.h"
#include "Landmark.h"
#include "LandmarkSet.h"
#include "../MultiObjectives/Martins.h"

#include "JsonWriter.h"


using namespace MuPaRo;
using namespace AlgoMPR;
using RLC::Martins;

RLC::DFA * dfas[2];
JsonWriter * out;
Area * toulouse;
Area * bordeaux;
Landmark * toulouse_lm;
Landmark * bordeaux_lm;
RLC::LandmarkSet * lmset;


void run_test(const Transport::Graph * trans, int car_start_node, int passenger_start_node, int car_arrival_node,
              int time, int day, RLC::DFA dfa_car, RLC::DFA dfa_passenger)
{
    RLC::Graph car_for_graph(trans, dfa_car);
    RLC::Graph ped_for_graph(trans, dfa_passenger);
    RLC::BackwardGraph car_back_graph(&car_for_graph);
    RLC::BackwardGraph ped_back_graph(&ped_for_graph);
    
    /*
    {   
        typedef AspectListConnections<AspectConnectionRule<Muparo<DRegLC>>> AlgoBrute;
        
        AlgoBrute::ParamType p(
            MuparoParams( trans, 2 ),
            AspectConnectionRuleParams( SumCost, FirstLayerArrival, &ped_for_graph, 0, 1, passenger_start_node )
        );
        
        AlgoBrute algo( p );
        
        { // layer 0 forward search from bordeaux (car)
            algo.graphs.push_back( new RLC::Graph(trans, dfa_car) );
            DRegLC::ParamType p( RLC::DRegLCParams( algo.graphs[0], day) );
            algo.dij.push_back( new DRegLC( p ) );
        }
        
        { // layer 1 backward search from toulouse (car)
            algo.graphs.push_back( new RLC::BackwardGraph(&car_for_graph) );
            DRegLC::ParamType p( RLC::DRegLCParams( algo.graphs[1], day) );
            algo.dij.push_back( new DRegLC( p ) );
        }
        
        algo.insert( StateFreeNode(0, car_start_node), time, 0);
        algo.insert( StateFreeNode(1, car_arrival_node), time, 0);
        algo.run();
        
        cout <<"Num connections : "<< algo.get_connection_points().size() <<endl;
    }
    */
    
    {
        typedef AspectPropagationRule<Muparo<DRegLC>> AlgoMulti;
        
        AlgoMulti::ParamType p(
            MuparoParams( trans, 3),
            AspectPropagationRuleParams( SumPlusWaitCost, MaxArrival, 2, 0, 1 )
        );
        
        AlgoMulti algo( p );
        
        { // layer 0 forward search from bordeaux (car)
            algo.graphs.push_back( new RLC::Graph(trans, dfa_car) );
            DRegLC::ParamType p( RLC::DRegLCParams( algo.graphs[0], day) );
            algo.dij.push_back( new DRegLC( p ) );
        }
        
        { // layer 1 backward search from toulouse (car)
            algo.graphs.push_back( new RLC::BackwardGraph(&car_for_graph) );
            DRegLC::ParamType p( RLC::DRegLCParams( algo.graphs[1], day, 2) );
            algo.dij.push_back( new DRegLC( p ) );
        }

//         algo.graphs.push_back( new RLC::BackwardGraph( &ped_for_graph ) );
        algo.graphs.push_back( new RLC::Graph( trans, dfa_passenger ) );

        Martins * m = new Martins(algo.graphs[2], passenger_start_node, day, bordeaux );
        algo.dij.push_back( m );
        
        algo.insert( StateFreeNode(0, car_start_node), time, 0);
        algo.insert( StateFreeNode(1, car_arrival_node), time, 0);
        algo.run();
        
        if( m->success ) {
//             cout << "Finished with cost "<< last.cost <<endl;
            cout << "Stats : "<< m->undominated_iter <<" " << m->total_iter <<endl;
        }
    }
    
    
    
}


void new_test(const Transport::Graph * g)
{
    int bordeaux_ped=-1, bordeaux_car=-1, toulouse_car=-1;
    
    
    
    bordeaux_ped = bordeaux->get( rand() % bordeaux->size() );
    bordeaux_car = bordeaux->get( rand() % bordeaux->size() );
    toulouse_car = toulouse->get( rand() % toulouse->size() );
    
    {
        const RLC::Graph pt_car(g, RLC::pt_car_dfa());
        Algo::PtToPt::ParamType p(
            RLC::DRegLCParams( &pt_car, 10 ), RLC::AspectTargetParams( bordeaux_car )    );
        Algo::PtToPt * dij = new Algo::PtToPt(p);
        dij->insert_node(RLC::Vertice(bordeaux_ped, 0), 0, 0);
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
            RLC::DRegLCParams( &pt_car, 10 ), RLC::AspectTargetParams( toulouse_car )    );
        Algo::PtToPt * dij = new Algo::PtToPt(p);
        dij->insert_node(RLC::Vertice(bordeaux_car, 0), 0, 0);
        dij->run();
        if( !dij->success ) {
            delete dij;
            return new_test(g);
        }
        delete dij;
    }
    
    cout << bordeaux_ped <<" "<< bordeaux_car <<" "<< toulouse_car <<" 32140 10 1 0" <<endl;
}


int main(void)
{
    bool small_areas = true;
    string config;
    config = "/home/arthur/LAAS/mumoro/algorithms/tests/SharedPath.conf";
     
    srand (time(NULL));
    
    dfas[0] = new RLC::DFA(RLC::pt_foot_dfa());
    dfas[1] = new RLC::DFA(RLC::car_dfa());
    
    std::string name, dump_file;
    
    char buff[255];
    int car_start_node, passenger_start_node, car_arrival_node, time, day, dfa_car, dfa_passenger;
    
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
    
    
    //                          tlse, bordeaux, albi
    int landmarks_nodes[3] = { 269647, 546063, 294951 };
    std::vector<const Landmark *> lms;
    BOOST_FOREACH( int n, landmarks_nodes ) {
        lms.push_back( RLC::create_car_landmark(transport, n) );
    }
    lmset = new RLC::LandmarkSet( lms, transport );
    
    cout << "R(Toulouse) : " << toulouse->radius << " ; R(Bordeaux) : " << bordeaux->radius <<endl;

    while ( !indata.eof() ) { // keep reading until end-of-file
        indata >> passenger_start_node >> car_start_node >> car_arrival_node
               >> time >> day >> dfa_car >> dfa_passenger; 
        run_test(transport, car_start_node, passenger_start_node, car_arrival_node, 
                 time, day, *dfas[dfa_car], *dfas[dfa_passenger]);
    }
    
    
    
    
}