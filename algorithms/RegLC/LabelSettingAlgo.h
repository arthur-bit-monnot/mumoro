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

#ifndef LABEL_SETTING_ALGO_H
#define LABEL_SETTING_ALGO_H

#include <iostream>

#include "reglc_graph.h"
#include "../Interface/Path.h"

namespace RLC {
    
    
struct Label {
  Label( RLC::Vertice node, int time, int cost, int source = -1) : node(node), cost(cost), h(0), time(time), source(source) {
      BOOST_ASSERT( valid() );
  }
  
  Label() : node(RLC::Vertice(-1, -1)), cost(-1), h(0), time(-1), source(-1) {}
  
  /**
   * The node (vertex in transport graph and state in NFA) this label applies to
   */
  RLC::Vertice node;
  
  /**
   * Actual cost of the node
   */
  int cost;
  
  /**
   * Evaluation of the distance to the goal
   */
  int h;
  
  /**
   * Arrival time at this node
   */
  int time;
  
  /**
   * 
   */
  int source;
  
  
  bool operator<(const Label & rhs) const { return cost + h > rhs.cost + rhs.h; }
  
  bool dominated_by(const Label & other) const { 
      return (cost >= other.cost && time >= other.time);
//           || ((cost >= other.cost) && (cost - other.cost > other.time - time));
  }
  
  bool valid() const { 
      BOOST_ASSERT( this->cost >= 0 );
      BOOST_ASSERT( this->node.first >= 0 );
      BOOST_ASSERT( this->node.second >= 0 );
      BOOST_ASSERT( this->h >= 0 );
      return true;
  }
};




class LabelSettingAlgo 
{
public:
    LabelSettingAlgo() : count(0) {}
    virtual ~LabelSettingAlgo() {}
    
    virtual bool finished() const = 0;
    virtual bool run() = 0;
    virtual Label treat_next() = 0;
    bool add_source_node( const Vertice & vert, const int arrival, const int vert_cost ) {
        return insert_node(vert, arrival, vert_cost, vert.first );
    }
    virtual bool insert_node(const Vertice & vert, const int arrival, const int vert_cost, const int source ) = 0;
    
//     virtual float arrival(const RLC::Vertice v) const = 0;
//     virtual float cost(const RLC::Vertice v) const = 0;
    
//     virtual RLC::Edge get_pred(const RLC::Vertice v) const = 0;
//     virtual bool has_pred(const RLC::Vertice v) const = 0;
    
    virtual int best_cost_in_heap() = 0;
    
    int count;
    
    virtual Path get_path_to( const int node ) const { return Path(); };
};






} // end namespace RLC

std::ostream & operator<<(std::ostream & os, const RLC::Label l);

#endif



