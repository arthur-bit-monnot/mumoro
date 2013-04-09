#ifndef MPR_ASPECT_PROPAGATION_RULE_H
#define MPR_ASPECT_PROPAGATION_RULE_H

#include "muparo.h"


namespace MuPaRo {

typedef enum { SumCost, MaxCost, SumPlusWaitCost } CostCombination;
typedef enum { MaxArrival, FirstLayerArrival, SecondLayerArrival } ArrivalCombination;

struct AspectPropagationRuleParams {
    AspectPropagationRuleParams( CostCombination cost, ArrivalCombination arr, int insertion_layer, int cond1, int cond2 ) :
    cost_comb(cost), arr_comb(arr), insertion_layer(insertion_layer) 
    {
        condition_layers.push_back(cond1);
        condition_layers.push_back(cond2);
    }
    CostCombination cost_comb;
    ArrivalCombination arr_comb;
    std::vector<int> condition_layers;
    int insertion_layer;
};


template<typename Base>
class AspectPropagationRule : public Base {
public:    
    typedef LISTPARAM<AspectPropagationRuleParams, typename Base::ParamType> ParamType;
    AspectPropagationRule( ParamType p ) : 
    Base(p.next),
    cost_comb(p.value.cost_comb),
    arr_comb(p.value.arr_comb),
    insertion_layer(p.value.insertion_layer),
    condition_layers(p.value.condition_layers)
    {

    }
    virtual ~AspectPropagationRule() {}
    
    CostCombination cost_comb;
    ArrivalCombination arr_comb;
    int insertion_layer;
    vector<int> condition_layers;
    
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
    
    void apply(const int node);
    
    virtual void apply_rules( const int node )
    {
        if( applicable(node) ) {
            int arr = arrival_in_insertion_layer( node );
            int cost = cost_in_insertion_layer( node );

            if( Base::insert( StateFreeNode(insertion_layer, node), arr, cost ) ) {
                Base::clear_pred_layers( StateFreeNode(insertion_layer, node) );
                BOOST_FOREACH( int cond_layer, condition_layers ) 
                    Base::add_pred_layer( StateFreeNode(insertion_layer, node), cond_layer);
            }
        }   
        
        Base::apply_rules( node );
    }
};

} //end namespace MuPaRo
        
        
#endif