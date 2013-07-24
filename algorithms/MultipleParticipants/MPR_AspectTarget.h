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

#ifndef MPR_ASPECT_TARGET_H
#define MPR_ASPECT_TARGET_H

#include "muparo.h"


namespace MuPaRo {

struct AspectTargetParams {
    AspectTargetParams( const int target_layer, const int target_node ) : target_layer(target_layer), target_node(target_node) {}
    const int target_layer;
    const int target_node;
};


template<typename Base>
class AspectTarget : public Base {
public:    
    typedef LISTPARAM<AspectTargetParams, typename Base::ParamType> ParamType;
    AspectTarget( ParamType p ) : Base(p.next) {
        goal = StateFreeNode(p.value.target_layer, p.value.target_node);
    }
    virtual ~AspectTarget() {};
    
    StateFreeNode goal;
    
    virtual bool finished() const override {
        if( Base::finished() )
            return true;
        
        if( Base::is_node_set( goal ) )
            return true;
        
        return false;
    }
    
    virtual void init_result_queue( std::list< CompleteNode > & queue ) override {
        /*
        if( Base::is_node_set( goal ) ) {
            queue.push_back( CompleteNode(goal.first, RLC::Vertice(goal.second, Base::flags[goal.first][goal.second].dfa_state )));
        }
        */
    };
    
    virtual int solution_cost() const override {
        return Base::get_cost( goal );
    }
};

} //end namespace MuPaRo
        
        
#endif