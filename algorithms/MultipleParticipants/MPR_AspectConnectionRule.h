#ifndef MPR_ASPECT_CONNECTION_RULE_H
#define MPR_ASPECT_CONNECTION_RULE_H

#include "muparo.h"
#include "MPR_AspectPropagationRule.h"
#include "AspectTarget.h"


namespace MuPaRo {

struct AspectConnectionRuleParams {
    AspectConnectionRuleParams( CostCombination cost, ArrivalCombination arr, RLC::AbstractGraph * graph, 
                                int cond1, int cond2, int target ) :
    cost_comb(cost), arr_comb(arr), graph(graph), target(target)
    {
        condition_layers.push_back(cond1);
        condition_layers.push_back(cond2);
    }
    CostCombination cost_comb;
    ArrivalCombination arr_comb;
    std::vector<int> condition_layers;
    RLC::AbstractGraph * graph;
    const int target;
};


template<typename Base>
class AspectConnectionRule : public Base {
public:    
    typedef RLC::AspectTarget<RLC::DRegLC> Dij;
    
    CostCombination cost_comb;
    ArrivalCombination arr_comb;
    int insertion_layer;
    vector<int> condition_layers;
    RLC::AbstractGraph * graph;
    int target;
    
    Dij * conn_dij;
    
    int best_cost = std::numeric_limits< int >::max();
    
    
    typedef LISTPARAM<AspectConnectionRuleParams, typename Base::ParamType> ParamType;
    AspectConnectionRule( ParamType p ) : 
    Base(p.next),
    cost_comb(p.value.cost_comb),
    arr_comb(p.value.arr_comb),
    condition_layers(p.value.condition_layers),
    graph(p.value.graph),
    target(p.value.target)
    {
        int day = 10;
        Dij::ParamType dij_params( 
            RLC::DRegLCParams(graph, day),
            RLC::AspectTargetParams( target )
        );
        conn_dij = new Dij( dij_params );
    }
    virtual ~AspectConnectionRule() {}
    
    virtual bool finished() const override {
        int lower_cost = std::numeric_limits< int >::max();
        for(int i=0 ; i<Base::num_layers ; ++i) {
            if( !Base::dij[i]->finished() && ( Base::dij[i]->cost( Base::dij[i]->heap->top() ) <= lower_cost ) ) {
                lower_cost = Base::dij[i]->cost( Base::dij[i]->heap->top() );
            }
        }
        return Base::finished() || lower_cost > best_cost;
    }
    
    
    int arrival_in_insertion_layer( const int node ) const
    {
        if(arr_comb == MaxArrival) {
            int arr = -99999999;
            BOOST_FOREACH( int layer, condition_layers ) {
                if( Base::arrival( StateFreeNode(layer, node) ) > arr )
                    arr = Base::arrival( StateFreeNode(layer, node) );
            }
            return arr;
        } else if(arr_comb == FirstLayerArrival) {
            return Base::arrival( StateFreeNode(condition_layers[0], node) );
        } else if(arr_comb == SecondLayerArrival) {
            return Base::arrival( StateFreeNode(condition_layers[1], node) );
        }
        
        throw new Invalid_Operation(); // This should never be reached;
    }
    
    int cost_in_insertion_layer( const int node ) const
    {
        int cost = 0;
        if( cost_comb == SumCost ){
            BOOST_FOREACH( int layer, condition_layers ) {
                cost += Base::get_cost( StateFreeNode(layer, node) );
            }
        } else if( cost_comb == SumPlusWaitCost ) {
            const int last_arr = arrival_in_insertion_layer( node );
            int max_wait = 0;
            
            BOOST_FOREACH( int layer, condition_layers ) {
                cost += Base::get_cost( StateFreeNode(layer, node) );
                if(last_arr - Base::arrival( StateFreeNode(layer, node) ) > max_wait)
                    max_wait = last_arr - Base::arrival( StateFreeNode(layer, node) );
            }
            
            cost += max_wait;
        } else {
            throw Invalid_Operation();
        }
        return cost;  
    }
    
    bool applicable(const int node) const
    {
        BOOST_FOREACH( int layer, condition_layers ) {
            if( !Base::is_node_set( StateFreeNode(layer, node) ) ) {
                return false;
            }
        }
        return true;
    }
    
    virtual void apply_rules( const int node ) override
    {
        static int count = 0;
        if( applicable(node) ) {
            int arr = arrival_in_insertion_layer( node );
            int cost = cost_in_insertion_layer( node );

            conn_dij->clear();
            conn_dij->insert_node( RLC::Vertice( node, 0 ), arr, cost );
            conn_dij->run();
            count++;
            if( conn_dij->success && conn_dij->get_path_cost() < best_cost ) {
                best_cost = conn_dij->get_path_cost();
            }
            
        }   
        
        Base::apply_rules( node );
    }
};

} //end namespace MuPaRo
        
        
#endif