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
    
    virtual bool insert_node( const Vertice & node, const int arrival, const int cost ) {
        if(cost > max_cost)
            return false;
        else
            return Base::insert_node( node, arrival, cost );
    }
    
private:
    int max_cost;
};

}

#endif