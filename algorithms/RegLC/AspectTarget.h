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
    AspectTarget( ParamType parameters ) : Base(parameters.next) {
        AspectTargetParams & p = parameters.value;
        
        BOOST_FOREACH( const int v_dfa, Base::graph->dfa_accepting_states() ) {
            dest_vertices.insert(Vertice(p.target, v_dfa));
        }
    }
    virtual ~AspectTarget() {}

    std::set<RLC::Vertice> dest_vertices;
    
    virtual bool check_termination( const RLC::Vertice & vert ) const { 
        return dest_vertices.find(vert) != dest_vertices.end();
    }
    
    virtual bool finished() const {
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
};


}


#endif
