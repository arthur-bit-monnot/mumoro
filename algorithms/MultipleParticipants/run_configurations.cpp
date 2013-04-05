
#include "run_configurations.h"
#include "node_filter_utils.h"

namespace MuPaRo {

Muparo * point_to_point(Transport::Graph * trans, int source, int dest)
{
    Muparo * mup = new Muparo(trans, 1);
    int day = 10;
    
    mup->dfas.push_back(RLC::bike_pt_dfa());
    
    for(int i=0; i<mup->num_layers ; ++i)
    {
        mup->graphs.push_back( new RLC::Graph(mup->transport, mup->dfas[i] ));
        mup->dij.push_back( new RLC::Dijkstra(mup->graphs[i], -1, -1, -1, day) );
    }
    
    mup->start_nodes.push_back( StartNode( StateFreeNode(0, source), 50000) );
    mup->goal_nodes.push_back( StateFreeNode(0, dest) );
    
    return mup;
}

Muparo * bi_point_to_point(Transport::Graph * trans, int source, int dest)
{
    MuparoParameters mup_params;
    mup_params.search_type = Bidirectional;;
    mup_params.bidir_layers = make_pair<int, int>(0, 1);
    
    Muparo * mup = new Muparo(trans, 2, mup_params);
    int day = 10;
    
    mup->dfas.push_back(RLC::foot_dfa());
    mup->dfas.push_back(RLC::foot_dfa());
    
    RLC::DijkstraParameters * param1 = new RLC::DijkstraParameters();
    param1->save_touched_nodes = true;
    RLC::DijkstraParameters * param2 = new RLC::DijkstraParameters();
    param2->save_touched_nodes = true;
    
    RLC::Graph *g = new RLC::Graph(mup->transport, mup->dfas[0] );
    mup->graphs.push_back( g );
    mup->graphs.push_back( new RLC::BackwardGraph(g));
    mup->dij.push_back( new RLC::Dijkstra(mup->graphs[0], -1, -1, -1, day, param1) );
    mup->dij.push_back( new RLC::Dijkstra(mup->graphs[1], -1, -1, -1, day, param2) );
    
    mup->start_nodes.push_back( StartNode( StateFreeNode(0, source), 50000) );
    mup->start_nodes.push_back( StartNode( StateFreeNode(1, dest), 0) );
    
    return mup;
}

Muparo * bidir_covoiturage(Transport::Graph * trans, int source1, int source2, int dest1, int dest2, RLC::DFA dfa_pass, RLC::DFA dfa_car, int limit)
{
    MuparoParameters mup_params;
    mup_params.search_type = Bidirectional;
    mup_params.bidir_layers = make_pair<int, int>(2, 5);
    
    Muparo * mup = new Muparo(trans, 6, mup_params);
    int day = 10;
    
    mup->dfas.push_back(dfa_pass);
    mup->dfas.push_back(dfa_car);
    mup->dfas.push_back(dfa_car);
    mup->dfas.push_back(dfa_pass);
    mup->dfas.push_back(dfa_car);
    mup->dfas.push_back(dfa_car);
    
    RLC::DijkstraParameters * param_car1 = new RLC::DijkstraParameters();
    RLC::DijkstraParameters * param_car2 = new RLC::DijkstraParameters();
    
    RLC::DijkstraParameters * param_car_bi1 = new RLC::DijkstraParameters();
    param_car_bi1->save_touched_nodes = true;
    RLC::DijkstraParameters * param_car_bi2 = new RLC::DijkstraParameters();
    param_car_bi2->save_touched_nodes = true;
    
    RLC::DijkstraParameters * param_passenger1 = new RLC::DijkstraParameters();
    param_passenger1->cost_limit = limit > 0;
    param_passenger1->cost_limit_value = limit;
    RLC::DijkstraParameters * param_passenger2 = new RLC::DijkstraParameters();
    param_passenger2->cost_limit = limit > 0;
    param_passenger2->cost_limit_value = limit;
    
    
    RLC::Graph *g1 = new RLC::Graph(mup->transport, mup->dfas[0] );
    RLC::Graph *g2 = new RLC::Graph(mup->transport, mup->dfas[1] );
    RLC::Graph *g3 = new RLC::Graph(mup->transport, mup->dfas[2] );
    mup->graphs.push_back( g1 );
    mup->graphs.push_back( g2 );
    mup->graphs.push_back( g3 );
    mup->graphs.push_back( new RLC::BackwardGraph(g1));
    mup->graphs.push_back( new RLC::BackwardGraph(g2));
    mup->graphs.push_back( new RLC::BackwardGraph(g3));
    mup->dij.push_back( new RLC::Dijkstra(mup->graphs[0], -1, -1, -1, day, param_passenger1) );
    mup->dij.push_back( new RLC::Dijkstra(mup->graphs[1], -1, -1, -1, day, param_car1) );
    mup->dij.push_back( new RLC::Dijkstra(mup->graphs[2], -1, -1, -1, day, param_car_bi1) );
    mup->dij.push_back( new RLC::Dijkstra(mup->graphs[3], -1, -1, -1, day, param_passenger2) );
    mup->dij.push_back( new RLC::Dijkstra(mup->graphs[4], -1, -1, -1, day, param_car2) );
    mup->dij.push_back( new RLC::Dijkstra(mup->graphs[5], -1, -1, -1, day, param_car_bi2) );
    
    mup->start_nodes.push_back( StartNode( StateFreeNode(0, source1), 50000) );
    mup->start_nodes.push_back( StartNode( StateFreeNode(1, source2), 0) );
    mup->start_nodes.push_back( StartNode( StateFreeNode(3, dest1), 0) );
    mup->start_nodes.push_back( StartNode( StateFreeNode(4, dest2), 0) );
    
    PropagationRule * cr = new PropagationRule(mup);
    cr->cost_comb = SumCost;
    cr->conditions.push_back(0);
    cr->conditions.push_back(1);
    cr->insertion = 2;
    PropagationRule * cr2 = new PropagationRule(mup);
    cr2->cost_comb = SumCost;
    cr2->conditions.push_back(3);
    cr2->conditions.push_back(4);
    cr2->insertion = 5;
    mup->propagation_rules.push_back(cr);
    mup->propagation_rules.push_back(cr2);
    
    return mup;
}

Muparo * time_dep_covoiturage(Transport::Graph * trans, int source1, int source2, int dest1, int dest2, RLC::DFA dfa_pass, RLC::DFA dfa_car, int limit)
{
    MuparoParameters mup_params;
    mup_params.search_type = Connection;
    
    Muparo * mup = new Muparo(trans, 5, mup_params);
    
    mup->vres.a_nodes.push_back(source1);
    mup->vres.a_nodes.push_back(source2);
    mup->vres.b_nodes.push_back(dest1);
    mup->vres.b_nodes.push_back(dest2);
    
    int day = 10;
    
    mup->dfas.push_back(dfa_pass);
    mup->dfas.push_back(dfa_car);
    mup->dfas.push_back(dfa_car);
    mup->dfas.push_back(dfa_pass);
    mup->dfas.push_back(dfa_car);
    
    RLC::DijkstraParameters * param_car1 = new RLC::DijkstraParameters();
    RLC::DijkstraParameters * param_car2 = new RLC::DijkstraParameters();
    RLC::DijkstraParameters * param_car3 = new RLC::DijkstraParameters();
    
    RLC::DijkstraParameters * param_passenger = new RLC::DijkstraParameters();
    param_passenger->cost_limit = limit > 0;
    param_passenger->cost_limit_value = limit;
    
    RLC::DijkstraParameters * param_passenger_back = new RLC::DijkstraParameters();
    param_passenger_back->cost_limit = limit > 0;
    param_passenger_back->cost_limit_value = limit;
    param_passenger_back->use_cost_lower_bounds = true;
    
    
    RLC::Graph *g1 = new RLC::Graph(mup->transport, mup->dfas[0] );
    RLC::Graph *g2 = new RLC::Graph(mup->transport, mup->dfas[1] );
    RLC::Graph *g3 = new RLC::Graph(mup->transport, mup->dfas[2] );
    mup->graphs.push_back( g1 );
    mup->graphs.push_back( g2 );
    mup->graphs.push_back( g3 );
    mup->graphs.push_back( new RLC::BackwardGraph(g1));
    mup->graphs.push_back( new RLC::BackwardGraph(g2));
    mup->dij.push_back( new RLC::Dijkstra(mup->graphs[0], -1, -1, -1, day, param_passenger) );
    mup->dij.push_back( new RLC::Dijkstra(mup->graphs[1], -1, -1, -1, day, param_car1) );
    mup->dij.push_back( new RLC::Dijkstra(mup->graphs[2], -1, -1, -1, day, param_car2) );
    mup->dij.push_back( new RLC::Dijkstra(mup->graphs[3], -1, -1, -1, day, param_passenger_back) );
    mup->dij.push_back( new RLC::Dijkstra(mup->graphs[4], -1, -1, -1, day, param_car3) );
    
    mup->start_nodes.push_back( StartNode( StateFreeNode(0, source1), 50000) );
    mup->start_nodes.push_back( StartNode( StateFreeNode(1, source2), 50000) );
    mup->start_nodes.push_back( StartNode( StateFreeNode(3, dest1), 0) );
    mup->start_nodes.push_back( StartNode( StateFreeNode(4, dest2), 0) );
    
    PropagationRule * pr = new PropagationRule(mup);
    pr->conditions.push_back(0);
    pr->conditions.push_back(1);
    pr->insertion = 2;
    mup->propagation_rules.push_back(pr);
    
    ConnectionRule cr(mup);
    cr.conditions.push_back(2);
    cr.conditions.push_back(3);
    cr.conditions.push_back(4);
    cr.l3_dest = dest1;
    mup->connection_rules.push_back(cr);
    return mup;
}

Muparo* conv_time_dep_covoiturage ( Transport::Graph* trans, int source1, int source2, int dest1, int dest2, RLC::DFA dfa_pass, RLC::DFA dfa_car )
{
    MuparoParameters mup_params;
    mup_params.search_type = DestNodes;
    
    Muparo * mup = new Muparo(trans, 5, mup_params);
    
    mup->vres.a_nodes.push_back(source1);
    mup->vres.a_nodes.push_back(source2);
    mup->vres.b_nodes.push_back(dest1);
    mup->vres.b_nodes.push_back(dest2);
    
    int day = 10;
    
    mup->dfas.push_back(dfa_pass);
    mup->dfas.push_back(dfa_car);
    mup->dfas.push_back(dfa_car);
    mup->dfas.push_back(dfa_car);
    mup->dfas.push_back(dfa_pass);
    
    RLC::DijkstraParameters * param_car_shared = new RLC::DijkstraParameters();
    param_car_shared->cost_factor = 2;
    
    
    RLC::Graph *g1 = new RLC::Graph(mup->transport, mup->dfas[0] );
    RLC::Graph *g2 = new RLC::Graph(mup->transport, mup->dfas[1] );
    RLC::Graph *g3 = new RLC::Graph(mup->transport, mup->dfas[2] );
    RLC::Graph *g5 = new RLC::Graph(mup->transport, mup->dfas[4] );
    
    mup->graphs.push_back( g1 );
    mup->graphs.push_back( g2 );
    mup->graphs.push_back( g3 );
    mup->graphs.push_back( new RLC::BackwardGraph(g2));
    mup->graphs.push_back( g5 );
    mup->dij.push_back( new RLC::Dijkstra(mup->graphs[0], -1, -1, -1, day) );
    mup->dij.push_back( new RLC::Dijkstra(mup->graphs[1], -1, -1, -1, day) );
    mup->dij.push_back( new RLC::Dijkstra(mup->graphs[2], -1, -1, -1, day, param_car_shared) );
    mup->dij.push_back( new RLC::Dijkstra(mup->graphs[3], -1, -1, -1, day) );
    mup->dij.push_back( new RLC::Dijkstra(mup->graphs[4], -1, -1, -1, day) );
    
    mup->start_nodes.push_back( StartNode( StateFreeNode(0, source1), 50000) );
    mup->start_nodes.push_back( StartNode( StateFreeNode(1, source2), 50000) );
    mup->start_nodes.push_back( StartNode( StateFreeNode(3, dest2), 0) );
    
    mup->goal_nodes.push_back( StateFreeNode(4, dest1) ) ;
    
    PropagationRule * pr = new PropagationRule(mup);
    pr->cost_comb = SumPlusWaitCost;
    pr->conditions.push_back(0);
    pr->conditions.push_back(1);
    pr->insertion = 2;
    pr->filter = cap_jj_nf( trans );
    mup->propagation_rules.push_back(pr);
    
    PropagationRule * pr2 = new PropagationRule(mup);
    pr2->cost_comb = SumCost;
    pr2->arr_comb = FirstLayerArrival;
    pr2->conditions.push_back(2);
    pr2->conditions.push_back(3);
    pr2->insertion = 4;
    pr2->filter = cap_jj_nf( trans );
    mup->propagation_rules.push_back(pr2);

    return mup;
}

Muparo * covoiturage ( Transport::Graph* trans, int source1, int source2, int dest1, int dest2, 
                                 RLC::DFA dfa_pass, RLC::DFA dfa_car )
{
    cout <<"Covoiturage :"<< source1<<" "<<source2<<" "<<dest1<<" "<<dest2<<endl;
    MuparoParameters mup_params;
    mup_params.search_type = DestNodes;
    
    Muparo * mup = new Muparo(trans, 5, mup_params);
    
    mup->vres.a_nodes.push_back(source1);
    mup->vres.a_nodes.push_back(source2);
    mup->vres.b_nodes.push_back(dest1);
    mup->vres.b_nodes.push_back(dest2);
    
    int day = 10;
    
    mup->dfas.push_back(dfa_pass);
    mup->dfas.push_back(dfa_car);
    mup->dfas.push_back(dfa_car);
    mup->dfas.push_back(dfa_car);
    mup->dfas.push_back(dfa_pass);
    
    RLC::DijkstraParameters * param_car_start = new RLC::DijkstraParameters();   
    RLC::DijkstraParameters * param_car_arr = new RLC::DijkstraParameters();
    RLC::DijkstraParameters * param_car_shared = new RLC::DijkstraParameters();
    param_car_shared->cost_factor = 2;
    
    RLC::DijkstraParameters * param_passenger_start = new RLC::DijkstraParameters();    
    RLC::DijkstraParameters * param_passenger_arr = new RLC::DijkstraParameters();
    
    RLC::Graph *g1 = new RLC::Graph(mup->transport, mup->dfas[0] );
    RLC::Graph *g2 = new RLC::Graph(mup->transport, mup->dfas[1] );
    RLC::Graph *g3 = new RLC::Graph(mup->transport, mup->dfas[2] );
    RLC::Graph *g5 = new RLC::Graph(mup->transport, mup->dfas[4] );
    
    mup->graphs.push_back( g1 );
    mup->graphs.push_back( g2 );
    mup->graphs.push_back( g3 );
    mup->graphs.push_back( new RLC::BackwardGraph(g2));
    mup->graphs.push_back( g5 );
    mup->dij.push_back( new RLC::Dijkstra(mup->graphs[0], -1, -1, -1, day, param_passenger_start) );
    mup->dij.push_back( new RLC::Dijkstra(mup->graphs[1], -1, -1, -1, day, param_car_start) );
    mup->dij.push_back( new RLC::Dijkstra(mup->graphs[2], -1, -1, -1, day, param_car_shared) );
    mup->dij.push_back( new RLC::Dijkstra(mup->graphs[3], -1, -1, -1, day, param_car_arr) );
    mup->dij.push_back( new RLC::Dijkstra(mup->graphs[4], -1, -1, -1, day, param_passenger_arr) );
    
    mup->start_nodes.push_back( StartNode( StateFreeNode(0, source1), 50000) );
    mup->start_nodes.push_back( StartNode( StateFreeNode(1, source2), 50000) );
    mup->start_nodes.push_back( StartNode( StateFreeNode(3, dest2), 0) );
    
    mup->goal_nodes.push_back( StateFreeNode(4, dest1) ) ;
    
    PropagationRule * pr = new PropagationRule(mup);
    pr->cost_comb = SumPlusWaitCost;
    pr->conditions.push_back(0);
    pr->conditions.push_back(1);
    pr->insertion = 2;
    mup->propagation_rules.push_back(pr);
    
    PropagationRule * pr2 = new PropagationRule(mup);
    pr2->cost_comb = SumCost;
    pr2->arr_comb = FirstLayerArrival;
    pr2->conditions.push_back(2);
    pr2->conditions.push_back(3);
    pr2->insertion = 4;
    mup->propagation_rules.push_back(pr2);
    
    add_rectangle_restriction_on_car(mup, trans, source2, dest2, 0.015);

    return mup;
}

void add_rectangle_restriction_on_car(Muparo * mup, Transport::Graph* trans, int source, int dest, float margin)
{
    RLC::DijkstraParameters * param_car_start = mup->dij[1]->params;
    param_car_start->filter_nodes = true;
    param_car_start->filter = rectangle_containing(trans, source, dest, margin);
    
    RLC::DijkstraParameters * param_car_arr = mup->dij[2]->params;
    param_car_arr->filter_nodes = true;
    param_car_arr->filter = rectangle_containing(trans, source, dest, margin);
    
    RLC::DijkstraParameters * param_car_shared = mup->dij[3]->params;
    param_car_shared->filter_nodes = true;
    param_car_shared->filter = rectangle_containing(trans, source, dest, margin);
}

void add_isochrone_restriction_on_passenger(Muparo * mup, Transport::Graph* trans, int source, int dest, float max_time)
{
    RLC::DijkstraParameters * param_passenger_start = mup->dij[0]->params;
    param_passenger_start->filter_nodes = true;
    param_passenger_start->filter = isochrone( trans, mup->dfas[0], source, max_time);
    
    RLC::DijkstraParameters * param_passenger_arr = mup->dij[4]->params;
    param_passenger_arr->filter_nodes = true;
    param_passenger_arr->filter = isochrone( trans, mup->dfas[4], dest, max_time);
}

} //end namespace MuPaRo