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

#ifndef ASPECT_TARGET_H
#define ASPECT_TARGET_H

#include "DRegLC.h"
#include "AspectCount.h"
#include "AspectMaxCostPruning.h"

namespace RLC {
    
struct AspectTargetParams {
    AspectTargetParams( const int target ) : target(target) {}
    const int target;
};


template<typename Base = DRegLC>
class AspectTarget : public Base 
{
public:    
    typedef LISTPARAM<AspectTargetParams, typename Base::ParamType> ParamType;
    
    std::set<RLC::Vertice> dest_vertices;
    int target_cost = -1;
    
    AspectTarget( ParamType parameters ) : Base(parameters.next) {
        AspectTargetParams & p = parameters.value;
        
        BOOST_FOREACH( const int v_dfa, Base::graph->dfa_accepting_states() ) {
            dest_vertices.insert(Vertice(p.target, v_dfa));
        }
    }
    virtual ~AspectTarget() {}

    
    
    virtual bool check_termination( const RLC::Label & lab ) override { 
        if( !(dest_vertices.find(lab.node) != dest_vertices.end()) ) {
            return false;
        } else {
            target_cost = lab.cost;
            return true;
        }
    }
    
    virtual bool finished() const override {
        return Base::finished() || Base::success;
    }
    
    std::vector<int> get_path() const {
        std::list<int> path;
        RLC::Vertice curr;
        
        BOOST_FOREACH( RLC::Vertice v, dest_vertices ) {
            if(Base::black(v))
                curr = v;
        }
        
        path.push_front(curr.first);
        while( Base::has_pred(curr) ) {
            curr = Base::g->source(Base::get_pred( curr ));
            path.push_front(curr.first);
        }
        
        return path;
    }
    
    int get_path_cost() const {
//         BOOST_FOREACH( RLC::Vertice v, dest_vertices ) {
//             if(Base::black(v))
//                 return Base::cost( v );
//         }
        return target_cost;
    }
};


}


#endif
