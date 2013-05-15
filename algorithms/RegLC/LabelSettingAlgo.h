#ifndef LABEL_SETTING_ALGO_H
#define LABEL_SETTING_ALGO_H

#include "reglc_graph.h"

namespace RLC {
    
    
struct Label {
  Label( RLC::Vertice node, int time, int cost, int source = -1) : node(node), cost(cost), h(0), time(time), source(source) {}
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
      return (cost >= other.cost && time >= other.time)
          || ((cost >= other.cost) && (cost - other.cost > other.time - time));
  }
};




class LabelSettingAlgo 
{
public:
    virtual ~LabelSettingAlgo() {}
    
    virtual bool finished() const = 0;
    virtual bool run() = 0;
    virtual Label treat_next() = 0;
    virtual bool insert_node(const Vertice & vert, const int arrival, const int vert_cost) = 0;
    
//     virtual float arrival(const RLC::Vertice v) const = 0;
//     virtual float cost(const RLC::Vertice v) const = 0;
    
//     virtual RLC::Edge get_pred(const RLC::Vertice v) const = 0;
//     virtual bool has_pred(const RLC::Vertice v) const = 0;
    
    virtual int best_cost_in_heap() const = 0;
    
    int count;
    
};






} // end namespace RLC


#endif



