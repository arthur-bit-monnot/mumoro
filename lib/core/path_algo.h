#ifndef PATH_ALGO_H
#define PATH_ALGO_H

#include "graph_wrapper.h"
#include "reglc_graph.h"
#include "nodes_filter.h"

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

class MeetPoint 
{
public:
    MeetPoint(int startA, int startB, int dep_sec, int dep_day, Transport::Graph * g);
    
    bool run();
    
    VisualResult get_result() const;
    
private:
    int startA;
    int startB;
    int dep_sec;
    int dep_day;
    Transport::Graph * g;
    
    
    RLC::DFA dfaA, dfaB;
    RLC::Graph rlcA, rlcB;
    RLC::Dijkstra dijA, dijB;
    BBNodeFilter nf;
    
    VisualResult vres;
};

struct VerticeState
{
    bool set_and_accepting;
    int dfa_state;
    int arrival;
};

class SharedPath 
{
public:
    SharedPath(int startA, int startB, int dest, int dep_sec, int dep_day, Transport::Graph * g);
    
    bool run();
    
    VisualResult get_result() const;
    
private:
    int startA;
    int startB;
    int dest;
    int dep_sec;
    int dep_day;
    Transport::Graph * g;
    
    
    RLC::DFA dfaA, dfaB, dfaC;
    RLC::Graph rlcA, rlcB, rlcC;
    RLC::Dijkstra dijA, dijB, dijC;
    BBNodeFilter nf;
    
    VerticeState **states;
    
    VisualResult vres;
    
    void insert_node_in_C(int node, int arr);
    void build_result(RLC::Vertice rlc_dest);
};



#endif