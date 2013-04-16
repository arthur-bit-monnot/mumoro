#ifndef NODE_FILTER_UTILS_H
#define NODE_FILTER_UTILS_H

#include "nodes_filter.h"
#include "reglc_graph.h"

/**
 * Builds an isochrone filter of maximum cost 'max_time' around 'center'
 */
NodeSet * isochrone ( const RLC::AbstractGraph * g, const int center, const int max_time );
VisualResult show_isochrone ( const RLC::AbstractGraph * g, const int center, const int max_time );


BBNodeFilter * rectangle_containing( const Transport::Graph * trans, const int node1, const int node2, const float margin );

BBNodeFilter * bordeaux_bb( const Transport::Graph * trans );

BBNodeFilter * toulouse_bb( const Transport::Graph * trans );



#endif

