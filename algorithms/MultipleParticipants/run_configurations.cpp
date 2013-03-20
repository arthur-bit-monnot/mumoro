
#include "run_configurations.h"

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
    
    RLC::DijkstraParameters param;
    param.save_touched_nodes = true;
    RLC::Graph *g = new RLC::Graph(mup->transport, mup->dfas[0] );
    mup->graphs.push_back( g );
    mup->graphs.push_back( new RLC::BackwardGraph(g));
    mup->dij.push_back( new RLC::Dijkstra(mup->graphs[0], -1, -1, -1, day, param) );
    mup->dij.push_back( new RLC::Dijkstra(mup->graphs[1], -1, -1, -1, day, param) );
    
    mup->start_nodes.push_back( StartNode( StateFreeNode(0, source), 50000) );
    mup->start_nodes.push_back( StartNode( StateFreeNode(1, dest), 0) );
    
    return mup;
}

Muparo * covoiturage(Transport::Graph * trans, int source1, int source2, int dest1, int dest2, RLC::DFA dfa_pass, RLC::DFA dfa_car, int limit)
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
    
    RLC::DijkstraParameters param_car;
    
    RLC::DijkstraParameters param_car_bi;
    param_car_bi.save_touched_nodes = true;
    
    RLC::DijkstraParameters param_passenger;
    param_passenger.cost_limit = limit > 0;
    param_passenger.cost_limit_value = limit;
    
    
    RLC::Graph *g1 = new RLC::Graph(mup->transport, mup->dfas[0] );
    RLC::Graph *g2 = new RLC::Graph(mup->transport, mup->dfas[1] );
    RLC::Graph *g3 = new RLC::Graph(mup->transport, mup->dfas[2] );
    mup->graphs.push_back( g1 );
    mup->graphs.push_back( g2 );
    mup->graphs.push_back( g3 );
    mup->graphs.push_back( new RLC::BackwardGraph(g1));
    mup->graphs.push_back( new RLC::BackwardGraph(g2));
    mup->graphs.push_back( new RLC::BackwardGraph(g3));
    mup->dij.push_back( new RLC::Dijkstra(mup->graphs[0], -1, -1, -1, day, param_passenger) );
    mup->dij.push_back( new RLC::Dijkstra(mup->graphs[1], -1, -1, -1, day, param_car) );
    mup->dij.push_back( new RLC::Dijkstra(mup->graphs[2], -1, -1, -1, day, param_car_bi) );
    mup->dij.push_back( new RLC::Dijkstra(mup->graphs[3], -1, -1, -1, day, param_passenger) );
    mup->dij.push_back( new RLC::Dijkstra(mup->graphs[4], -1, -1, -1, day, param_car) );
    mup->dij.push_back( new RLC::Dijkstra(mup->graphs[5], -1, -1, -1, day, param_car_bi) );
    
    mup->start_nodes.push_back( StartNode( StateFreeNode(0, source1), 50000) );
    mup->start_nodes.push_back( StartNode( StateFreeNode(1, source2), 0) );
    mup->start_nodes.push_back( StartNode( StateFreeNode(3, dest1), 0) );
    mup->start_nodes.push_back( StartNode( StateFreeNode(4, dest2), 0) );
    
    PropagationRule cr(mup);
    cr.conditions.push_back(0);
    cr.conditions.push_back(1);
    cr.insertion = 2;
    PropagationRule cr2(mup);
    cr2.conditions.push_back(3);
    cr2.conditions.push_back(4);
    cr2.insertion = 5;
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
    
    RLC::DijkstraParameters param_car;
    
    RLC::DijkstraParameters param_passenger;
    param_passenger.cost_limit = limit > 0;
    param_passenger.cost_limit_value = limit;
    
    RLC::DijkstraParameters param_passenger_back;
    param_passenger_back.cost_limit = limit > 0;
    param_passenger_back.cost_limit_value = limit;
    param_passenger_back.use_cost_lower_bounds = true;
    
    
    RLC::Graph *g1 = new RLC::Graph(mup->transport, mup->dfas[0] );
    RLC::Graph *g2 = new RLC::Graph(mup->transport, mup->dfas[1] );
    RLC::Graph *g3 = new RLC::Graph(mup->transport, mup->dfas[2] );
    mup->graphs.push_back( g1 );
    mup->graphs.push_back( g2 );
    mup->graphs.push_back( g3 );
    mup->graphs.push_back( new RLC::BackwardGraph(g1));
    mup->graphs.push_back( new RLC::BackwardGraph(g2));
    mup->dij.push_back( new RLC::Dijkstra(mup->graphs[0], -1, -1, -1, day, param_passenger) );
    mup->dij.push_back( new RLC::Dijkstra(mup->graphs[1], -1, -1, -1, day, param_car) );
    mup->dij.push_back( new RLC::Dijkstra(mup->graphs[2], -1, -1, -1, day, param_car) );
    mup->dij.push_back( new RLC::Dijkstra(mup->graphs[3], -1, -1, -1, day, param_passenger_back) );
    mup->dij.push_back( new RLC::Dijkstra(mup->graphs[4], -1, -1, -1, day, param_car) );
    
    mup->start_nodes.push_back( StartNode( StateFreeNode(0, source1), 50000) );
    mup->start_nodes.push_back( StartNode( StateFreeNode(1, source2), 50000) );
    mup->start_nodes.push_back( StartNode( StateFreeNode(3, dest1), 0) );
    mup->start_nodes.push_back( StartNode( StateFreeNode(4, dest2), 0) );
    
    PropagationRule pr(mup);
    pr.conditions.push_back(0);
    pr.conditions.push_back(1);
    pr.insertion = 2;
    mup->propagation_rules.push_back(pr);
    
    ConnectionRule cr(mup);
    cr.conditions.push_back(2);
    cr.conditions.push_back(3);
    cr.conditions.push_back(4);
    cr.l3_dest = dest1;
    mup->connection_rules.push_back(cr);
    return mup;
}

} //end namespace MuPaRo