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

#ifndef ASPECT_TARGET_LANDMARK_H
#define ASPECT_TARGET_LANDMARK_H

#include "DRegLC.h"
#include "AspectTarget.h"
#include "Landmark.h"

namespace RLC {
    
template<typename H = Landmark>
struct AspectTargetLandmarkParams {
    AspectTargetLandmarkParams( const int target, const H * h ) : target(target), h(h) {}
    const int target;
    const H * h;
};


/**
 * Aspect implementing an A* search towards a single target.
 * 
 * It needs a heuristic H (default is landmark) to provide a lower bound of the distance 
 * between two vertices in the graph.
 */
template<typename Base = DRegLC, typename H = Landmark>
class AspectTargetLandmark : public AspectTarget<Base> 
{
    /**
     * Provide a heuristic giving lower bound of distance to target from any point in the graph.
     * Needs to be admissible.
     */
    const H * h = NULL;
    
    /** 
     * Node we're willing to reach
     */
    int target;
        
public:    
    typedef LISTPARAM<AspectTargetLandmarkParams<H>, typename Base::ParamType> ParamType;
    
    AspectTargetLandmark( ParamType parameters ) : 
    AspectTarget<Base>(typename AspectTarget<Base>::ParamType(
        typename Base::ParamType( parameters.next ),
        AspectTargetParams( parameters.value.target )  )) 
    {
        target = parameters.value.target;
        h = parameters.value.h;
    }
    
    virtual ~AspectTargetLandmark() {}
    
    virtual Label label(RLC::Vertice vert, int time, int cost, int source = -1) const override {
        Label l = Base::label(vert, time, cost, source);
        l.h = h->dist_lb( vert.first, target, Base::graph->forward ) * Base::cost_factor;
        
        BOOST_ASSERT( l.valid() );
        return l;
    }

};


}


#endif
