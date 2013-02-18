#ifndef PATH_ALGO_H
#define PATH_ALGO_H

#include "graph_wrapper.h"

struct Compare;


struct node_ptr
{
  int index;    // index of the string
//   float time; // current distance

  explicit node_ptr(int idx): index(idx) {}
//   void operator= (float t) { time = t; }

//   bool operator< (node_ptr const& np) const { return time > np.time; }
};

struct Compare
{
    const std::vector<float> & vec;
    Compare(const std::vector<float> & dist) : vec(dist) {}
    bool operator()(node_ptr a, node_ptr b) const
    {
        return vec[a.index] > vec[b.index];
    }
};

EdgeList dijkstra(int start, int destination, float dep_sec, int dep_day, Graph g);



#endif