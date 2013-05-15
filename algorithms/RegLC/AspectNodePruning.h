#ifndef ASPECT_NODE_PRUNING_H
#define ASPECT_NODE_PRUNING_H

#include "DRegLC.h"
#include "nodes_filter.h"

namespace RLC {
    
struct AspectNodePruningParams {
    AspectNodePruningParams( const NodeFilter * nf ) : nf(nf) {}
    const NodeFilter * nf;
};


template<typename Base>
class AspectNodePruning : public Base {
public:    
    typedef LISTPARAM<AspectNodePruningParams, typename Base::ParamType> ParamType;
    
    AspectNodePruning( ParamType parameters ) : Base(parameters.next) {
        AspectNodePruningParams & p = parameters.value;
        nf = p.nf;
    }
    virtual ~AspectNodePruning() {}
    
    virtual bool insert_node( const Vertice & node, const int arrival, const int cost ) override {
        if( ! nf->isIn(node.first) )
            return false;
        else
            return Base::insert_node( node, arrival, cost );
    }
    
private:
    const NodeFilter * nf;
};

}

#endif