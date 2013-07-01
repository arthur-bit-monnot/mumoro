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
        has_predecessor = new boost::dynamic_bitset<>*[Base::dfa_num_vert];
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
    
    virtual bool insert_node_with_predecessor(const Vertice & vert, const int arrival, const int cost, const RLC::Edge & pred, const int source) override
    {
        bool was_inserted = Base::insert_node_with_predecessor(vert, arrival, cost, pred, source);
        if( was_inserted )
            set_pred( vert, pred );
        return was_inserted;
    }
    
    virtual Path get_path_to( const int node ) const { 
        Path p; 
        p.end_node = node;
        
        Vertice curr_node(node, 0);
        
        while( has_pred(curr_node) ) {
            cout << curr_node.first << endl;
            p.edges.push_back(this->graph->transport->edgeIndex( this->get_pred(curr_node).first ));
            curr_node = this->graph->source( this->get_pred( curr_node ) );
        }
        
        p.start_node = curr_node.first;
        
        return p;
    }

private:
    boost::dynamic_bitset<> ** has_predecessor;
    RLC::Edge **predecessors;
};

}

#endif