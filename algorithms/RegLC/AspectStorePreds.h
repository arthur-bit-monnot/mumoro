#ifndef ASPECT_STORE_PREDS_H
#define ASPECT_STORE_PREDS_H

#include "DRegLC.h"

namespace RLC {

    
/**
 * This aspect is used to store information about the predecessors of vertices.
 * 
 * It is  no longer included in DRegLC, since some applications don't need it and it creates some stress on the cache.
 */
template<typename Base>
class AspectStorePreds : public Base {
public:    
    typedef typename Base::ParamType ParamType;
    AspectStorePreds( ParamType parameters ) : Base(parameters) { 
        has_predecessor = new boost::dynamic_bitset<>*[dfa_num_vert];
        predecessors = new RLC::Edge*[Base::dfa_num_vert];
        for(int i=0 ; i<Base::dfa_num_vert ; ++i) {
            has_predecessor[i] = new boost::dynamic_bitset<>(Base::trans_num_vert);
            predecessors[i] = (RLC::Edge *) malloc(Base::trans_num_vert * sizeof(RLC::Edge));
        }
    }
    virtual ~AspectStorePreds() {
        for(int i=0 ; i<Base::dfa_num_vert ; ++i) {
            delete has_predecessor[i];
            free( predecessors[i] );
        }
        delete[] has_predecessor;
        delete[] predecessors;
    }
    
    inline void clear_pred(const RLC::Vertice v) { has_predecessor[v.second]->reset(v.first); }
    inline void set_pred(const RLC::Vertice v, const RLC::Edge & pred) { 
        has_predecessor[v.second]->set(v.first);
        predecessors[v.second][v.first] = pred; 
    }

    inline RLC::Edge get_pred(const RLC::Vertice v) const { return predecessors[v.second][v.first]; }
    inline bool has_pred(const RLC::Vertice v) const { return has_predecessor[v.second]->test(v.first); }
    
    virtual void clear() override {
        for(int i=0 ; i<Base::dfa_num_vert ; ++i) {
            has_predecessor[i]->reset();
        }
        Base::clear();
    }
    
    virtual bool insert_node_with_predecessor(const Vertice & vert, const int arrival, const int cost, const RLC::Edge & pred) override
    {
        bool was_inserted = Base::insert_node_with_predecessor(vert, arrival, cost, pred);
        if( was_inserted )
            set_pred( vert, pred );
        return was_inserted;
    }
    
    virtual RLC::Label treat_next() override {
        Base::count++;
        return Base::treat_next();
    }
    
private:
    boost::dynamic_bitset<> ** has_predecessor;
    RLC::Edge **predecessors;
};

}

#endif