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