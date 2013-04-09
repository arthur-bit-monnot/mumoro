#ifndef NODE_FILTER_UTILS_H
#define NODE_FILTER_UTILS_H

#include "nodes_filter.h"
#include "reglc_graph.h"

/**
 * Builds an isochrone filter of maximum cost 'max_time' around 'center'
 */
NodeSet * isochrone ( RLC::AbstractGraph * g, const int center, const int max_time );
VisualResult show_isochrone ( RLC::AbstractGraph * g, const int center, const int max_time );


BBNodeFilter * rectangle_containing( Transport::Graph * trans, const int node1, const int node2, const float margin );






#endif

