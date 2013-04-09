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
    
    virtual bool finished() const {
        if( Base::finished() )
            return true;
        
        if( Base::is_node_set( goal ) )
            return true;
        
        return false;
    }
    
    virtual void init_result_queue( std::list< CompleteNode > & queue ) {
        if( Base::is_node_set( goal ) ) {
            queue.push_back( CompleteNode(goal.first, RLC::Vertice(goal.second, Base::flags[goal.first][goal.second].dfa_state )));
        }
    };
};

} //end namespace MuPaRo
        
        
#endif