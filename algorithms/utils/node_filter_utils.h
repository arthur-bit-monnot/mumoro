#ifndef NODE_FILTER_UTILS_H
#define NODE_FILTER_UTILS_H

#include "nodes_filter.h"
#include "reglc_graph.h"

NodeSet * isochrone( Transport::Graph * trans, RLC::DFA dfa, const int node, const int max_time );

BBNodeFilter * rectangle_containing( Transport::Graph * trans, const int node1, const int node2, const float margin );






#endif

