#ifndef PATH_ALGO_H
#define PATH_ALGO_H

#include "graph_wrapper.h"

struct Compare;


struct node_ptr
{
  int index;    // index of the string

  explicit node_ptr(int idx): index(idx) {}
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

EdgeList dijkstra(int start, int destination, float dep_sec, int dep_day, Transport::Graph g);

#endif