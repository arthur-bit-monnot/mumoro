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

#include <boost/foreach.hpp>

#include "MultipleParticipants/muparo.h"
#include "MultipleParticipants/run_configurations.h"
#include "utils/node_filter_utils.h"
#include "utils/Area.h"

#include "RegLC/AspectTarget.h"
#include "RegLC/AlgoTypedefs.h"
#include "RegLC/AspectTargetLandmark.h"
#include "RegLC/AspectTargetAreaLandmark.h"
#include "RegLC/LandmarkSet.h"

#include "DataStructures/GraphFactory.h"

#include "MultiObjectives/Martins.h"

using namespace RLC;

int main() 
{
//     std::string file( "/home/arthur/LAAS/Data/Graphs/sud-ouest.dump" );
    std::string file( "/home/arthur/LAAS/Data/Graphs/toulouse.dump" );
    Transport::GraphFactory gf( file, true );
    
    const Transport::Graph * g = gf.get();
    
    RLC::Graph rlc(g, RLC::foot_dfa());
    RLC::BackwardGraph back_rlc( &rlc );
    
//     Area * toulouse = toulouse_area(g);
//     Area * bordeaux = bordeaux_area(g);
    
    int src1 = 10;
    int src2 = 644;
    int dest = 235;//20; //693;
    
    int time = 50000;
    int day = 10;
    
    {
        typedef AspectPropagationRule<Muparo<DRegLC>> AlgoMulti;
        
        AlgoMulti::ParamType p(
            MuparoParams( g, 3),
            AspectPropagationRuleParams( SumPlusWaitCost, MaxArrival, 2, 0, 1 )
        );
        
        AlgoMulti algo( p );
        
        for(int i=0; i<2 ; ++i)
        {
            algo.graphs.push_back( new RLC::Graph(algo.transport, RLC::car_dfa() ));
            DRegLC::ParamType p( RLC::DRegLCParams( algo.graphs[i], day) );
            algo.dij.push_back( new DRegLC( p ) );
        }
        
        algo.graphs.push_back( new RLC::Graph(algo.transport, pt_foot_dfa() ));
        Martins * m = new Martins(algo.graphs[2], dest, day);
        algo.dij.push_back( m );
        
        algo.insert( StateFreeNode(0, src1), time, 0);
        algo.insert( StateFreeNode(1, src2), time, 0);
        algo.run();
        
        if( m->success ) {
            cout << "Stats : "<< m->undominated_iter <<" " << m->total_iter <<endl;
        }
    }
    
}