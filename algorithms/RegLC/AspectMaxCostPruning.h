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

#ifndef ASPECT_MAX_COST_PRUNING_H
#define ASPECT_MAX_COST_PRUNING_H

#include "DRegLC.h"
#include "AspectCount.h"

namespace RLC {
    
struct AspectMaxCostPruningParams {
    AspectMaxCostPruningParams( const int max_cost ) : max_cost(max_cost) {}
    const int max_cost;
};


template<typename Base>
class AspectMaxCostPruning : public Base {
public:    
    typedef LISTPARAM<AspectMaxCostPruningParams, typename Base::ParamType> ParamType;
    
    AspectMaxCostPruning( ParamType parameters ) : Base(parameters.next) {
        AspectMaxCostPruningParams & p = parameters.value;
        max_cost = p.max_cost;
    }
    virtual ~AspectMaxCostPruning() {}
    
    virtual bool insert_node_impl( const Label & label ) override {
        if( (label.cost + label.h) > max_cost )
            return false;
        else
            return Base::insert_node_impl( label );
    }
     
protected:
    int max_cost;
};

}

#endif