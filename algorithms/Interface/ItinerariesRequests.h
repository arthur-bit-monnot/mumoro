#ifndef INTERFACE_ITINERARIES_REQUEST_H
#define INTERFACE_ITINERARIES_REQUEST_H

#include "Path.h"

#include "graph_wrapper.h"
#include "reglc_graph.h"

Path point_to_point( const Transport::Graph * trans, const int source, const int dest, const int departure_time, const int day, const RLC::DFA dfa = RLC::pt_foot_dfa() );


#endif