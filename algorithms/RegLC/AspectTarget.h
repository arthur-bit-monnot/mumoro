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


template<typename Base>
class AspectTarget : public Base {
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

};
/* TODO clean  up
DRegLC * dreg_with_target(AbstractGraph * g, const int source, const int target) {
    typedef AspectMaxCostPruning<AspectCount<AspectTarget<DRegLC> > > Algo;
    Algo::ParamType p(
        DRegLCParams( g, 10 ), AspectTargetParams( 306 ), AspectMaxCostPruningParams( 1000 )
    );
    Algo * dreg = new Algo(p);

    dreg->insert_node(Vertice(source, 0), 0, 0);

    return dreg;
}
*/

}


#endif
