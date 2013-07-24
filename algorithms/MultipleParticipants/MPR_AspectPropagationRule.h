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
    boost::dynamic_bitset<> * was_inserted = new boost::dynamic_bitset<>(Base::transport->num_vertices());
    
    typedef LISTPARAM<AspectPropagationRuleParams, typename Base::ParamType> ParamType;
    AspectPropagationRule( ParamType p ) : 
    Base(p.next),
    cost_comb(p.value.cost_comb),
    arr_comb(p.value.arr_comb),
    insertion_layer(p.value.insertion_layer),
    condition_layers(p.value.condition_layers)
    {
        
    }
    virtual ~AspectPropagationRule() {
        delete was_inserted;
    }
    
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
    
    virtual void apply_rules( const int node ) override
    {
        if( applicable(node) ) {
            int arr = arrival_in_insertion_layer( node );
            int cost = cost_in_insertion_layer( node );

            if( !was_inserted->test(node) ) {
                if( Base::insert( StateFreeNode(insertion_layer, node), arr, cost ) ) {
                    Base::clear_pred_layers( StateFreeNode(insertion_layer, node) );
                    BOOST_FOREACH( int cond_layer, condition_layers ) 
                        Base::add_pred_layer( StateFreeNode(insertion_layer, node), cond_layer);
                        
                    if( cost_comb == SumCost ) //drop_off
                        Base::num_drop_off++;
                    if( cost_comb == SumPlusWaitCost ) // pick up
                        Base::num_pick_up++;
                    
                    was_inserted->set(node);
                }
            }
        }   
        
        Base::apply_rules( node );
    }
};

} //end namespace MuPaRo
        
        
#endif