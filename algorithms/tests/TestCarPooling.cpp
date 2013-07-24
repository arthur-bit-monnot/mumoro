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
#include "LandmarkSet.h"

#include "JsonWriter.h"

using RLC::Landmark;
using RLC::LandmarkSet;

using namespace MuPaRo;
using namespace AlgoMPR;

RLC::DFA * dfas[2];
JsonWriter * out;
Area * toulouse;
Area * bordeaux;
RLC::LandmarkSet * lmset;

void run_test(std::string directory, const Transport::Graph * trans, int car_start_node, int passenger_start_node, int car_arrival_node,
              int passenger_arrival_node, int time, int day, RLC::DFA dfa_car, RLC::DFA dfa_passenger)
{        
    Area * area_start;
    Area * area_dest;
    
    time = 50000;
    
    if( toulouse->isIn(passenger_start_node) ) {
        area_start = toulouse;
        car_start_node = 209194;
    } else if( bordeaux->isIn(passenger_start_node) ) {
        area_start = bordeaux;
    } else {
        return ;
    }
    
    if( toulouse->isIn(passenger_arrival_node) ) {
        area_dest = toulouse;
        car_arrival_node = 209194;
    } else if( bordeaux->isIn(passenger_arrival_node) )
        area_dest = bordeaux;
    else
        return ;
    
    
    out->step_in();
    
    out->add("car-start", car_start_node);
    out->add("passenger-start", passenger_start_node);
    out->add("car-arrival", car_arrival_node);
    out->add("passenger-arrival", passenger_arrival_node);
    out->add("time", time);
    out->add("day", day);
    
    AlgoMPR::PtToPt * mup =  point_to_point( trans, car_start_node, car_arrival_node, dfa_car );
    mup->run();
    out->add("driver-alone-cost", mup->get_cost(0, car_arrival_node));
    delete mup;
    
    
    {
        typedef CarSharingTest CurrAlgo;
        
        out->step_in("original");

        START_TICKING;
        CurrAlgo::ParamType p(
            MuparoParams( trans, 5 ),
            AspectTargetParams( 4, passenger_arrival_node ),
            AspectPropagationRuleParams( SumPlusWaitCost, MaxArrival, 2, 0, 1),
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
        int tot = 0;
        for(int i=0 ; i<cs.num_layers ; ++i) {
            per_layer.push_back( cs.dij[i]->count );
            tot += cs.dij[i]->count;
        }
        cout << cs.count <<" "<< tot <<endl;
        out->add("visited-per-layer", per_layer);
        out->add("solution-cost", cs.solution_cost());
        
        
        out->step_in("node-details");
        {
            out->step_in("pass-arr");
            out->add("node", passenger_arrival_node);
            out->step_in("5");
            out->add("cost", cs.get_cost(4, passenger_arrival_node));
            out->add("arrival", cs.arrival(4, passenger_arrival_node));
            
            out->step_out(); out->step_out();
        }
        
        int drop_off = cs.get_source(4, passenger_arrival_node);
        {
            out->step_in("drop_off");
            out->add("node", drop_off);
            
            out->step_in("5");
            out->add("cost", cs.get_cost(4, drop_off));
            out->add("arrival", cs.arrival(4, drop_off));
            out->step_out(); 
            
            out->step_in("4");
            out->add("cost", cs.get_cost(3, drop_off));
            out->add("arrival", cs.arrival(3, drop_off));
            out->step_out(); 
            
            out->step_in("3");
            out->add("cost", cs.get_cost(2, drop_off));
            out->add("arrival", cs.arrival(2, drop_off));
            out->step_out(); 
            
            out->step_out();
        }
        
        int pick_up = cs.get_source(2, drop_off);
        {
            out->step_in("pick-up");
            out->add("node", drop_off);
            
            out->step_in("3");
            out->add("cost", cs.get_cost(2, pick_up));
            out->add("arrival", cs.arrival(2, pick_up));
            out->step_out(); 
            
            out->step_in("2");
            out->add("cost", cs.get_cost(1, pick_up));
            out->add("arrival", cs.arrival(1, pick_up));
            out->step_out(); 
            
            out->step_in("1");
            out->add("cost", cs.get_cost(0, pick_up));
            out->add("arrival", cs.arrival(0, pick_up));
            out->step_out(); 
            
            out->step_out();
        }
        out->step_out(); //node details
        
        {
            out->add("1-length", cs.arrival(0, pick_up) - time);
            out->add("2-length", cs.arrival(1, pick_up) - time);
            out->add("3-length", cs.arrival(2, drop_off) - cs.arrival(2, pick_up));
            out->add("4-length", cs.get_cost(3, drop_off));
            out->add("5-length", cs.arrival(4, passenger_arrival_node) - cs.arrival(4, drop_off));
            out->add("wait-time", abs(cs.arrival(0, pick_up) - cs.arrival(1, pick_up)) );
            out->add("num-pick-up", cs.num_pick_up);
            out->add("num-drop-off", cs.num_drop_off);
            
//             out->add("nodes-set-in-MOM", cs.dij[4]->count_set);
//             out->add("avg-label-in-MOM", cs.dij[4]->average_label);
        }
        
        out->step_out();
    }
    
    {
        out->step_in("cities-stop-conditions");

        START_TICKING;
        CarSharingTest::ParamType p(
            MuparoParams( trans, 5 ),
            AspectTargetParams( 4, passenger_arrival_node ),
            AspectPropagationRuleParams( SumPlusWaitCost, MaxArrival, 2, 0, 1),
            AspectPropagationRuleParams( SumCost, FirstLayerArrival, 4, 2, 3)
        );
        
        std::vector<NodeFilter*> filters;
        RLC::Graph g1(trans, dfa_passenger );
        RLC::BackwardGraph g2(&g1);

        CarSharingTest cs( p );
        
        init_multi_car_sharing_with_areas<CarSharingTest>( &cs, trans, passenger_start_node, car_start_node, passenger_arrival_node, car_arrival_node, dfa_passenger, dfa_car, area_start, area_dest );

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
        
        out->step_in("node-details");
        {
            out->step_in("pass-arr");
            out->add("node", passenger_arrival_node);
            out->step_in("5");
            out->add("cost", cs.get_cost(4, passenger_arrival_node));
            out->add("arrival", cs.arrival(4, passenger_arrival_node));
            
            out->step_out(); out->step_out();
        }
        
        int drop_off = cs.get_source(4, passenger_arrival_node);
        {
            out->step_in("drop_off");
            out->add("node", drop_off);
            
            out->step_in("5");
            out->add("cost", cs.get_cost(4, drop_off));
            out->add("arrival", cs.arrival(4, drop_off));
            out->step_out(); 
            
            out->step_in("4");
            out->add("cost", cs.get_cost(3, drop_off));
            out->add("arrival", cs.arrival(3, drop_off));
            out->step_out(); 
            
            out->step_in("3");
            out->add("cost", cs.get_cost(2, drop_off));
            out->add("arrival", cs.arrival(2, drop_off));
            out->step_out(); 
            
            out->step_out();
        }
        
        int pick_up = cs.get_source(2, drop_off);
        {
            out->step_in("pick-up");
            out->add("node", drop_off);
            
            out->step_in("3");
            out->add("cost", cs.get_cost(2, pick_up));
            out->add("arrival", cs.arrival(2, pick_up));
            out->step_out(); 
            
            out->step_in("2");
            out->add("cost", cs.get_cost(1, pick_up));
            out->add("arrival", cs.arrival(1, pick_up));
            out->step_out(); 
            
            out->step_in("1");
            out->add("cost", cs.get_cost(0, pick_up));
            out->add("arrival", cs.arrival(0, pick_up));
            out->step_out(); 
            
            out->step_out();
        }
        out->step_out(); //node details
        
        {
            out->add("1-length", cs.arrival(0, pick_up) - time);
            out->add("2-length", cs.arrival(1, pick_up) - time);
            out->add("3-length", cs.arrival(2, drop_off) - cs.arrival(2, pick_up));
            out->add("4-length", cs.get_cost(3, drop_off));
            out->add("5-length", cs.arrival(4, passenger_arrival_node) - cs.arrival(4, drop_off));
            out->add("wait-time", abs(cs.arrival(0, pick_up) - cs.arrival(1, pick_up)) );
            out->add("num-pick-up", cs.num_pick_up);
            out->add("num-drop-off", cs.num_drop_off);
            
//             out->add("nodes-set-in-MOM", cs.dij[4]->count_set);
//             out->add("avg-label-in-MOM", cs.dij[4]->average_label);
        }
        out->step_out();
    }
    
    {
        out->step_in("cities-stop-conditions-landmarks");

        START_TICKING;
        CarSharingTest::ParamType p(
            MuparoParams( trans, 5 ),
            AspectTargetParams( 4, passenger_arrival_node ),
            AspectPropagationRuleParams( SumPlusWaitCost, MaxArrival, 2, 0, 1),
            AspectPropagationRuleParams( SumCost, FirstLayerArrival, 4, 2, 3)
        );
        
        std::vector<NodeFilter*> filters;
        RLC::Graph g1(trans, dfa_passenger );
        RLC::BackwardGraph g2(&g1);

        
        CarSharingTest cs( p );
        
        init_multi_car_sharing_with_areas<CarSharingTest>( &cs, trans, passenger_start_node, car_start_node, passenger_arrival_node, car_arrival_node, 
                                                     dfa_passenger, dfa_car, area_start, area_dest, 
                                                     true, lmset, lmset );

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
        
        out->step_in("node-details");
        {
            out->step_in("pass-arr");
            out->add("node", passenger_arrival_node);
            out->step_in("5");
            out->add("cost", cs.get_cost(4, passenger_arrival_node));
            out->add("arrival", cs.arrival(4, passenger_arrival_node));
            
            out->step_out(); out->step_out();
        }
        
        int drop_off = cs.get_source(4, passenger_arrival_node);
        {
            out->step_in("drop_off");
            out->add("node", drop_off);
            
            out->step_in("5");
            out->add("cost", cs.get_cost(4, drop_off));
            out->add("arrival", cs.arrival(4, drop_off));
            out->step_out(); 
            
            out->step_in("4");
            out->add("cost", cs.get_cost(3, drop_off));
            out->add("arrival", cs.arrival(3, drop_off));
            out->step_out(); 
            
            out->step_in("3");
            out->add("cost", cs.get_cost(2, drop_off));
            out->add("arrival", cs.arrival(2, drop_off));
            out->step_out(); 
            
            out->step_out();
        }
        
        int pick_up = cs.get_source(2, drop_off);
        {
            out->step_in("pick-up");
            out->add("node", drop_off);
            
            out->step_in("3");
            out->add("cost", cs.get_cost(2, pick_up));
            out->add("arrival", cs.arrival(2, pick_up));
            out->step_out(); 
            
            out->step_in("2");
            out->add("cost", cs.get_cost(1, pick_up));
            out->add("arrival", cs.arrival(1, pick_up));
            out->step_out(); 
            
            out->step_in("1");
            out->add("cost", cs.get_cost(0, pick_up));
            out->add("arrival", cs.arrival(0, pick_up));
            out->step_out(); 
            
            out->step_out();
        }
        out->step_out(); //node details
        
        {
            out->add("1-length", cs.arrival(0, pick_up) - time);
            out->add("2-length", cs.arrival(1, pick_up) - time);
            out->add("3-length", cs.arrival(2, drop_off) - cs.arrival(2, pick_up));
            out->add("4-length", cs.get_cost(3, drop_off));
            out->add("5-length", cs.arrival(4, passenger_arrival_node) - cs.arrival(4, drop_off));
            out->add("wait-time", abs(cs.arrival(0, pick_up) - cs.arrival(1, pick_up)) );
            out->add("num-pick-up", cs.num_pick_up);
            out->add("num-drop-off", cs.num_drop_off);
            
//             out->add("nodes-set-in-MOM", cs.dij[4]->count_set);
//             out->add("avg-label-in-MOM", cs.dij[4]->average_label);
        }
        
        out->step_out();
    }
    
    //      For very small areas around the passenger
    area_start = build_area_around_with_start_time(trans, passenger_start_node, passenger_start_node, time, 10 * 60);
    area_start->init();
    area_dest =  build_area_around(trans, passenger_arrival_node, passenger_arrival_node, 10 * 60);
    area_dest->init();
    
    {
        out->step_in("10-min-stop-conditions");

        START_TICKING;
        CarSharingTest::ParamType p(
            MuparoParams( trans, 5 ),
            AspectTargetParams( 4, passenger_arrival_node ),
            AspectPropagationRuleParams( SumPlusWaitCost, MaxArrival, 2, 0, 1),
            AspectPropagationRuleParams( SumCost, FirstLayerArrival, 4, 2, 3)
        );
        
        std::vector<NodeFilter*> filters;
        RLC::Graph g1(trans, dfa_passenger );
        RLC::BackwardGraph g2(&g1);

        CarSharingTest cs( p );
        
        init_multi_car_sharing_with_areas<CarSharingTest>( &cs, trans, passenger_start_node, car_start_node, passenger_arrival_node, car_arrival_node, dfa_passenger, dfa_car, area_start, area_dest );

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
        
        out->step_in("node-details");
        {
            out->step_in("pass-arr");
            out->add("node", passenger_arrival_node);
            out->step_in("5");
            out->add("cost", cs.get_cost(4, passenger_arrival_node));
            out->add("arrival", cs.arrival(4, passenger_arrival_node));
            
            out->step_out(); out->step_out();
        }
        
        int drop_off = cs.get_source(4, passenger_arrival_node);
        {
            out->step_in("drop_off");
            out->add("node", drop_off);
            
            out->step_in("5");
            out->add("cost", cs.get_cost(4, drop_off));
            out->add("arrival", cs.arrival(4, drop_off));
            out->step_out(); 
            
            out->step_in("4");
            out->add("cost", cs.get_cost(3, drop_off));
            out->add("arrival", cs.arrival(3, drop_off));
            out->step_out(); 
            
            out->step_in("3");
            out->add("cost", cs.get_cost(2, drop_off));
            out->add("arrival", cs.arrival(2, drop_off));
            out->step_out(); 
            
            out->step_out();
        }
        
        int pick_up = cs.get_source(2, drop_off);
        {
            out->step_in("pick-up");
            out->add("node", drop_off);
            
            out->step_in("3");
            out->add("cost", cs.get_cost(2, pick_up));
            out->add("arrival", cs.arrival(2, pick_up));
            out->step_out(); 
            
            out->step_in("2");
            out->add("cost", cs.get_cost(1, pick_up));
            out->add("arrival", cs.arrival(1, pick_up));
            out->step_out(); 
            
            out->step_in("1");
            out->add("cost", cs.get_cost(0, pick_up));
            out->add("arrival", cs.arrival(0, pick_up));
            out->step_out(); 
            
            out->step_out();
        }
        out->step_out(); //node details
        
        {
            out->add("1-length", cs.arrival(0, pick_up) - time);
            out->add("2-length", cs.arrival(1, pick_up) - time);
            out->add("3-length", cs.arrival(2, drop_off) - cs.arrival(2, pick_up));
            out->add("4-length", cs.get_cost(3, drop_off));
            out->add("5-length", cs.arrival(4, passenger_arrival_node) - cs.arrival(4, drop_off));
            out->add("wait-time", abs(cs.arrival(0, pick_up) - cs.arrival(1, pick_up)) );
            out->add("num-pick-up", cs.num_pick_up);
            out->add("num-drop-off", cs.num_drop_off);
            
//             out->add("nodes-set-in-MOM", cs.dij[4]->count_set);
//             out->add("avg-label-in-MOM", cs.dij[4]->average_label);
        }
        
        out->step_out();
    }
    
    {
        out->step_in("10min-stop-conditions-landmarks");

        START_TICKING;
        CarSharingTest::ParamType p(
            MuparoParams( trans, 5 ),
            AspectTargetParams( 4, passenger_arrival_node ),
            AspectPropagationRuleParams( SumPlusWaitCost, MaxArrival, 2, 0, 1),
            AspectPropagationRuleParams( SumCost, FirstLayerArrival, 4, 2, 3)
        );
        
        std::vector<NodeFilter*> filters;
        RLC::Graph g1(trans, dfa_passenger );
        RLC::BackwardGraph g2(&g1);

        
        CarSharingTest cs( p );
        
        init_multi_car_sharing_with_areas<CarSharingTest>( &cs, trans, passenger_start_node, car_start_node, passenger_arrival_node, car_arrival_node, 
                                                     dfa_passenger, dfa_car, area_start, area_dest, 
                                                     true, lmset, lmset );

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
        
        out->step_in("node-details");
        {
            out->step_in("pass-arr");
            out->add("node", passenger_arrival_node);
            out->step_in("5");
            out->add("cost", cs.get_cost(4, passenger_arrival_node));
            out->add("arrival", cs.arrival(4, passenger_arrival_node));
            
            out->step_out(); out->step_out();
        }
        
        int drop_off = cs.get_source(4, passenger_arrival_node);
        {
            out->step_in("drop_off");
            out->add("node", drop_off);
            
            out->step_in("5");
            out->add("cost", cs.get_cost(4, drop_off));
            out->add("arrival", cs.arrival(4, drop_off));
            out->step_out(); 
            
            out->step_in("4");
            out->add("cost", cs.get_cost(3, drop_off));
            out->add("arrival", cs.arrival(3, drop_off));
            out->step_out(); 
            
            out->step_in("3");
            out->add("cost", cs.get_cost(2, drop_off));
            out->add("arrival", cs.arrival(2, drop_off));
            out->step_out(); 
            
            out->step_out();
        }
        
        int pick_up = cs.get_source(2, drop_off);
        {
            out->step_in("pick-up");
            out->add("node", drop_off);
            
            out->step_in("3");
            out->add("cost", cs.get_cost(2, pick_up));
            out->add("arrival", cs.arrival(2, pick_up));
            out->step_out(); 
            
            out->step_in("2");
            out->add("cost", cs.get_cost(1, pick_up));
            out->add("arrival", cs.arrival(1, pick_up));
            out->step_out(); 
            
            out->step_in("1");
            out->add("cost", cs.get_cost(0, pick_up));
            out->add("arrival", cs.arrival(0, pick_up));
            out->step_out(); 
            
            out->step_out();
        }
        out->step_out(); //node details
        
        {
            out->add("1-length", cs.arrival(0, pick_up) - time);
            out->add("2-length", cs.arrival(1, pick_up) - time);
            out->add("3-length", cs.arrival(2, drop_off) - cs.arrival(2, pick_up));
            out->add("4-length", cs.get_cost(3, drop_off));
            out->add("5-length", cs.arrival(4, passenger_arrival_node) - cs.arrival(4, drop_off));
            out->add("wait-time", abs(cs.arrival(0, pick_up) - cs.arrival(1, pick_up)) );
            out->add("num-pick-up", cs.num_pick_up);
            out->add("num-drop-off", cs.num_drop_off);
            
//             out->add("nodes-set-in-MOM", cs.dij[4]->count_set);
//             out->add("avg-label-in-MOM", cs.dij[4]->average_label);
        }
        
        out->step_out();
    }
    
    out->step_out();
    
    delete area_dest;
    delete area_start;
}


void new_test(const Transport::Graph * g, bool from_bordeaux)
{
    int bordeaux1=-1, bordeaux2=-1, toulouse1=-1, toulouse2=-1;
    int albi = 209194;
    
    
    bordeaux1 = bordeaux->get( rand() % bordeaux->size() );
    bordeaux2 = bordeaux->get( rand() % bordeaux->size() );
    toulouse1 = toulouse->get( rand() % toulouse->size() );
    toulouse2 = toulouse->get( rand() % toulouse->size() );
    
    {
        const RLC::Graph pt_car(g, RLC::pt_car_dfa());
        Algo::PtToPt::ParamType p(
            RLC::DRegLCParams( &pt_car, 10 ), RLC::AspectTargetParams( bordeaux2 )    );
        Algo::PtToPt * dij = new Algo::PtToPt(p);
        dij->add_source_node(RLC::Vertice(bordeaux1, 0), 0, 0);
        dij->run();
        if( !dij->success ) {
            delete dij;
            return new_test(g, from_bordeaux);
        }
        delete dij;
    }
    {
        const RLC::Graph pt_car(g, RLC::pt_car_dfa());
        Algo::PtToPt::ParamType p(
            RLC::DRegLCParams( &pt_car, 10 ), RLC::AspectTargetParams( toulouse2 )    );
        Algo::PtToPt * dij = new Algo::PtToPt(p);
        dij->add_source_node(RLC::Vertice(toulouse1, 0), 0, 0);
        dij->run();
        if( !dij->success ) {
            delete dij;
            return new_test(g, from_bordeaux);
        }
        delete dij;
    }
    {
        const RLC::Graph pt_car(g, RLC::car_dfa());
        Algo::PtToPt::ParamType p(
            RLC::DRegLCParams( &pt_car, 10 ), RLC::AspectTargetParams( toulouse2 )    );
        Algo::PtToPt * dij = new Algo::PtToPt(p);
        dij->add_source_node(RLC::Vertice(bordeaux2, 0), 0, 0);
        dij->run();
        if( !dij->success ) {
            delete dij;
            return new_test(g, from_bordeaux);
        }
        delete dij;
    }
    
    
    if( from_bordeaux )
        cout << bordeaux1 <<" "<< bordeaux2 <<" "<< toulouse1 <<" "<< albi <<" 32140 10 1 0" <<endl;
    else
        cout << toulouse1 <<" "<< albi <<" "<< bordeaux1 <<" "<< bordeaux2 <<" 32140 10 1 0" <<endl;
}


int main(int argc, char ** argv)
{
  if(argc == 2)
    {
      std::string conf_dir_environment_variable = "MUMORO_TESTS_CONF";
      std::string results_dir_environment_variable = "MUMORO_TESTS_RESULTS";
      std::string dumps_dir_environment_variable = "MUMORO_DUMPS";
      
      std::string undefined_environment_variable_msg = "Undefined environment variable";
      std::string loaded_environment_variable_msg = "Loaded environment variable";
      
      std::string test_conf_dir;
      std::string test_results_dir;
      std::string test_dumps_dir;
      
      char * test_conf_dir_char = ::getenv(conf_dir_environment_variable.c_str());//"/home/arthur/LAAS/Data/TestConfs/"
      
      if(test_conf_dir_char == NULL)
	{
	  std::cerr << undefined_environment_variable_msg << " '" << conf_dir_environment_variable << "'." << std::endl;
	  
	  return EXIT_FAILURE;
	}
      else
	{
	  std::cout << loaded_environment_variable_msg << " '" << conf_dir_environment_variable << "'." << std::endl;
	  test_conf_dir = std::string(test_conf_dir_char);
	}
      
      char * test_results_dir_char = ::getenv(results_dir_environment_variable.c_str());
      
      if(test_results_dir_char == NULL)
	{
	  std::cerr << undefined_environment_variable_msg << " '" << results_dir_environment_variable << "'." << std::endl;
	  
	  return EXIT_FAILURE;
	}
      else
	{
	  std::cout << loaded_environment_variable_msg << " '" << results_dir_environment_variable << "'." << std::endl;
	  test_results_dir = std::string(test_results_dir_char);
	}
      
      char * test_dumps_dir_char = ::getenv(dumps_dir_environment_variable.c_str());
      
      if(test_dumps_dir_char == NULL)
	{
	  std::cerr << undefined_environment_variable_msg << " '" << dumps_dir_environment_variable << "'." << std::endl;
	  
	  return EXIT_FAILURE;
	}
      else
	{
	  std::cout << loaded_environment_variable_msg << " '" << dumps_dir_environment_variable << "'." << std::endl;
	  test_dumps_dir = std::string(test_dumps_dir_char);
	}
      
      std::string config_file_name = std::string(argv[1]);
      
      string config = test_conf_dir + config_file_name;//"bordeaux-toulouse-albi.conf";
      bool small_areas = false;
      
      srand (time(NULL));
      
      dfas[0] = new RLC::DFA(RLC::pt_foot_dfa());
      dfas[1] = new RLC::DFA(RLC::car_dfa());
      
      std::string name, dump_file;
      
      char buff[255];
      int car_start_node, passenger_start_node, car_arrival_node, passenger_arrival_node, time, day, dfa_car, dfa_passenger;
      
      ifstream indata; // indata is like cin
      
      indata.open(config); // opens the file
      if(!indata) { // file couldn't be opened
        cerr << "Error: file could not be opened: "<< config << endl;
        exit(1);
      }
      
      while( indata.peek() == '#' ) {
        indata.getline(buff, 255);
      }
      
      indata >> name;
      indata >> dump_file;
      JsonWriter writer(/*"/home/arthur/LAAS/Data/Results/"*/ test_conf_dir + name + ".txt");
      out = &writer;
      Transport::GraphFactory gf(/*"/home/arthur/LAAS/Data/Graphs/"*/ test_dumps_dir + dump_file, true);
      const Transport::Graph * transport = gf.get();
      
      if(small_areas) {
        toulouse = toulouse_area_small(transport);
        bordeaux = bordeaux_area_small(transport);
      } else {
        toulouse = toulouse_area(transport);
        bordeaux = bordeaux_area(transport);
      }
      
      //                          tlse, bordeaux, albi
      int landmarks_nodes[3] = { 269647, 546063, 294951 };
      std::vector<const Landmark *> lms;
      BOOST_FOREACH( int n, landmarks_nodes ) {
        lms.push_back( RLC::create_car_landmark(transport, n) );
      }
      lmset = new RLC::LandmarkSet( lms, transport );
      
    while ( !indata.eof() ) { // keep reading until end-of-file
      indata >> passenger_start_node >> car_start_node >> passenger_arrival_node >> car_arrival_node
	     >> time >> day >> dfa_car >> dfa_passenger; 
      run_test("1", transport, car_start_node, passenger_start_node, car_arrival_node, passenger_arrival_node, 
	       time, day, *dfas[dfa_car], *dfas[dfa_passenger]);
    }
    
    cout << endl << endl;
    
    //      for(int i=0 ; i<50 ; ++i)
//          new_test(transport, true);
    
    }
  else
    {
      std::cerr << "Please provide a configuration file." << std::endl;
    }
}
