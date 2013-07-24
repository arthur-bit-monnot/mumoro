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

#include "run_configurations.h"
#include "node_filter_utils.h"

namespace MuPaRo {
    
using namespace AlgoMPR;

AlgoMPR::PtToPt * point_to_point( const Transport::Graph * trans, int source, int dest, RLC::DFA dfa )
{
    PtToPt::ParamType p( MuparoParams(trans, 1), AspectTargetParams( 0, dest ) );
    std::cout << "Dest ::: "<<dest <<endl;
    PtToPt * mup = new PtToPt( p );
    int day = 10;
    
    for(int i=0; i<mup->num_layers ; ++i)
    {
        mup->graphs.push_back( new RLC::Graph(mup->transport, dfa ));
        PtToPt::Dijkstra::ParamType p( RLC::DRegLCParams( mup->graphs[i], day) );
        mup->dij.push_back( new PtToPt::Dijkstra( p ) );
    }
    
    mup->start_nodes.push_back( StartNode( StateFreeNode(0, source), 50000) );
    
    return mup;
}

VisualResult show_point_to_point ( const Transport::Graph* trans, int source, int dest, RLC::DFA dfa )
{
    PtToPt * ptp = point_to_point( trans, source, dest, dfa );
    ptp->run();
    ptp->build_result();
    return ptp->get_result();
}

SharedPath* shared_path ( const Transport::Graph* trans, int src1, int src2, int dest, RLC::DFA dfa )
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
        sp->graphs.push_back( new RLC::Graph(sp->transport, dfa ));
        PtToPt::Dijkstra::ParamType p( RLC::DRegLCParams( sp->graphs[i], day) );
        sp->dij.push_back( new PtToPt::Dijkstra( p ) );
    }
    
    sp->start_nodes.push_back( StartNode( StateFreeNode(0, src1), 50000) );
    sp->start_nodes.push_back( StartNode( StateFreeNode(1, src2), 50000) );
    
    return sp;
}

VisualResult show_shared_path ( const Transport::Graph* trans, int src1, int src2, int dest )
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
        AspectPropagationRuleParams( SumCost, MaxArrival, 2, 0, 1),
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