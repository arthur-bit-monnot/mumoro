
#include "run_configurations.h"
#include "node_filter_utils.h"

namespace MuPaRo {
    
using namespace AlgoMPR;

AlgoMPR::PtToPt * point_to_point( Transport::Graph * trans, int source, int dest, RLC::DFA dfa )
{
    PtToPt::ParamType p( MuparoParams(trans, 1), AspectTargetParams( 0, dest ) );
    std::cout << "Dest ::: "<<dest <<endl;
    PtToPt * mup = new PtToPt( p );
    int day = 10;
    
    mup->dfas.push_back(dfa);
    
    for(int i=0; i<mup->num_layers ; ++i)
    {
        mup->graphs.push_back( new RLC::Graph(mup->transport, mup->dfas[i] ));
        PtToPt::Dijkstra::ParamType p( RLC::DRegLCParams( mup->graphs[i], day) );
        mup->dij.push_back( new PtToPt::Dijkstra( p ) );
    }
    
    mup->start_nodes.push_back( StartNode( StateFreeNode(0, source), 50000) );
    mup->goal_nodes.push_back( StateFreeNode(0, dest) );
    
    return mup;
}

VisualResult show_point_to_point ( Transport::Graph* trans, int source, int dest, RLC::DFA dfa )
{
    PtToPt * ptp = point_to_point( trans, source, dest, dfa );
    ptp->run();
    ptp->build_result();
    return ptp->get_result();
}

SharedPath* shared_path ( Transport::Graph* trans, int src1, int src2, int dest )
{
    int day = 10;
    SharedPath::ParamType p(
        MuparoParams( trans, 3 ),
        AspectTargetParams( 2, dest ),
        AspectPropagationRuleParams( SumPlusWaitCost, MaxArrival, 2, 0, 1)
    );
    SharedPath * sp = new SharedPath( p );
    
    
    
    for(int i=0; i<sp->num_layers ; ++i)
    {
        sp->dfas.push_back(RLC::bike_pt_dfa());
        sp->graphs.push_back( new RLC::Graph(sp->transport, sp->dfas[i] ));
        PtToPt::Dijkstra::ParamType p( RLC::DRegLCParams( sp->graphs[i], day) );
        sp->dij.push_back( new PtToPt::Dijkstra( p ) );
    }
    
    sp->start_nodes.push_back( StartNode( StateFreeNode(0, src1), 50000) );
    sp->start_nodes.push_back( StartNode( StateFreeNode(1, src2), 50000) );
    
    return sp;
}

VisualResult show_shared_path ( Transport::Graph* trans, int src1, int src2, int dest )
{
    SharedPath * sp = shared_path( trans, src1, src2, dest );
    sp->run();
    sp->build_result();
    VisualResult res = sp->get_result();
    delete sp;
    return res;
}


CarSharing * car_sharing ( const Transport::Graph* trans, int src_ped, int src_car, int dest_ped, int dest_car, 
                                   RLC::DFA dfa_ped, RLC::DFA dfa_car )
{
    CarSharing::ParamType p(
        MuparoParams( trans, 5 ),
        AspectTargetParams( 4, dest_ped ),
        AspectPropagationRuleParams( SumPlusWaitCost, MaxArrival, 2, 0, 1),
        AspectPropagationRuleParams( SumCost, FirstLayerArrival, 4, 2, 3)
    );
    
    CarSharing * cs = new CarSharing( p );
    
    init_car_sharing<CarSharing>( cs, trans, src_ped, src_car, dest_ped, dest_car, dfa_ped, dfa_car );
    
    return cs;
}


VisualResult show_car_sharing ( const Transport::Graph* trans, int src_ped, int src_car, int dest_ped, int dest_car, 
                                        RLC::DFA dfa_ped, RLC::DFA dfa_car )
{
    CarSharing * cs = car_sharing(trans, src_ped, src_car, dest_ped, dest_car, dfa_ped, dfa_car);
    START_TICKING;
    cs->run();
    STOP_TICKING;
    std::cout<< "Car Sharing algo finished in "<< RUNTIME <<"ms" <<endl;
    cs->build_result();
    VisualResult res = cs->get_result();
    delete cs;
    return res;
}



} //end namespace MuPaRo