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

#include <boost/foreach.hpp> 

#include "reglc_graph.h"



using namespace std;
using namespace boost;


namespace RLC {

/*************************** DFA ************************/

DFA::DFA(int start, std::set<int> accepting, DfaEdgeList edges) :
start_state(start), accepting_states(accepting)
{
    DfaEdgeList::iterator it;
    for(it = edges.begin() ; it != edges.end() ; ++it) {
        ::Edge e;
        e.type = (EdgeMode) it->second;
        boost::add_edge(it->first.first, it->first.second, e, graph);
    }
}

DFA foot_subway_dfa()
{
    DfaEdgeList edges;
    edges.push_back(pair<pair<int,int>,EdgeMode>(pair<int,int>(0, 0), FootEdge));
    edges.push_back(pair<pair<int,int>,EdgeMode>(pair<int,int>(0, 0), TransferEdge));
    edges.push_back(pair<pair<int,int>,EdgeMode>(pair<int,int>(0, 1), SubwayEdge));
    edges.push_back(pair<pair<int,int>,EdgeMode>(pair<int,int>(1, 1), FootEdge));
    edges.push_back(pair<pair<int,int>,EdgeMode>(pair<int,int>(1, 1), SubwayEdge));
    edges.push_back(pair<pair<int,int>,EdgeMode>(pair<int,int>(1, 1), TransferEdge));
    set<int> accepting;
    accepting.insert(1);
    
    return DFA(0, accepting, edges);
}

DFA foot_dfa()
{
    DfaEdgeList edges;
    edges.push_back(pair<pair<int,int>,EdgeMode>(pair<int,int>(0, 0), FootEdge));
    set<int> accepting;
    accepting.insert(0);
    
    return DFA(0, accepting, edges);
}

DFA bike_dfa()
{
    DfaEdgeList edges;
    edges.push_back(pair<pair<int,int>,EdgeMode>(pair<int,int>(0, 0), BikeEdge));
    set<int> accepting;
    accepting.insert(0);
    
    return DFA(0, accepting, edges);
}

DFA car_dfa()
{
    DfaEdgeList edges;
    edges.push_back(pair<pair<int,int>,EdgeMode>(pair<int,int>(0, 0), CarEdge));
    set<int> accepting;
    accepting.insert(0);
    
    return DFA(0, accepting, edges);
}

DFA pt_foot_dfa()
{
    DfaEdgeList edges;
    edges.push_back(pair<pair<int,int>,EdgeMode>(pair<int,int>(0, 0), FootEdge));
    edges.push_back(pair<pair<int,int>,EdgeMode>(pair<int,int>(0, 0), TransferEdge));
    edges.push_back(pair<pair<int,int>,EdgeMode>(pair<int,int>(0, 0), SubwayEdge));
    edges.push_back(pair<pair<int,int>,EdgeMode>(pair<int,int>(0, 0), BusEdge));
    edges.push_back(pair<pair<int,int>,EdgeMode>(pair<int,int>(0, 0), TramEdge));
    set<int> accepting;
    accepting.insert(0);
    
    return DFA(0, accepting, edges);
}

DFA pt_car_dfa()
{
    DfaEdgeList edges;
    edges.push_back(pair<pair<int,int>,EdgeMode>(pair<int,int>(0, 0), FootEdge));
    edges.push_back(pair<pair<int,int>,EdgeMode>(pair<int,int>(0, 0), TransferEdge));
    edges.push_back(pair<pair<int,int>,EdgeMode>(pair<int,int>(0, 0), SubwayEdge));
    edges.push_back(pair<pair<int,int>,EdgeMode>(pair<int,int>(0, 0), BusEdge));
    edges.push_back(pair<pair<int,int>,EdgeMode>(pair<int,int>(0, 0), TramEdge));
    edges.push_back(pair<pair<int,int>,EdgeMode>(pair<int,int>(0, 1), CarEdge));
    edges.push_back(pair<pair<int,int>,EdgeMode>(pair<int,int>(1, 1), CarEdge));
    set<int> accepting;
    accepting.insert(1);
    
    return DFA(0, accepting, edges);
}

DFA bike_pt_dfa()
{
    DfaEdgeList edges;
    edges.push_back(pair<pair<int,int>,EdgeMode>(pair<int,int>(0, 1), FootEdge));
    edges.push_back(pair<pair<int,int>,EdgeMode>(pair<int,int>(0, 1), TransferEdge));
    edges.push_back(pair<pair<int,int>,EdgeMode>(pair<int,int>(0, 1), SubwayEdge));
    edges.push_back(pair<pair<int,int>,EdgeMode>(pair<int,int>(0, 1), BusEdge));
    edges.push_back(pair<pair<int,int>,EdgeMode>(pair<int,int>(1, 1), FootEdge));
    edges.push_back(pair<pair<int,int>,EdgeMode>(pair<int,int>(1, 1), SubwayEdge));
    edges.push_back(pair<pair<int,int>,EdgeMode>(pair<int,int>(1, 1), TransferEdge));
    edges.push_back(pair<pair<int,int>,EdgeMode>(pair<int,int>(1, 1), BusEdge));
    edges.push_back(pair<pair<int,int>,EdgeMode>(pair<int,int>(0, 2), BikeEdge));
    edges.push_back(pair<pair<int,int>,EdgeMode>(pair<int,int>(2, 2), BikeEdge));
    set<int> accepting;
    accepting.insert(1);
    accepting.insert(2);
    
    return DFA(0, accepting, edges);
}

DFA pt_dfa()
{
    DfaEdgeList edges;
    edges.push_back(pair<pair<int,int>,EdgeMode>(pair<int,int>(0, 0), FootEdge));
    edges.push_back(pair<pair<int,int>,EdgeMode>(pair<int,int>(0, 0), TransferEdge));
    edges.push_back(pair<pair<int,int>,EdgeMode>(pair<int,int>(0, 0), SubwayEdge));
    edges.push_back(pair<pair<int,int>,EdgeMode>(pair<int,int>(0, 0), BusEdge));
    edges.push_back(pair<pair<int,int>,EdgeMode>(pair<int,int>(0, 1), TransferEdge));
    set<int> accepting;
    accepting.insert(1);
    
    return DFA(0, accepting, edges);
}

/**************************** Graph ***********************************/


Graph::Graph(const Transport::Graph* transport, DFA dfa) : 
AbstractGraph(true, transport),
transport(transport), 
dfa(dfa)
{
}


RLC::Vertice Graph::source( const RLC::Edge & edge) const
{
    return RLC::Vertice(transport->source( edge.first ), boost::source(edge.second, dfa.graph));
}

RLC::Vertice Graph::target( const RLC::Edge & edge) const
{
    return RLC::Vertice(transport->target( edge.first ), boost::target(edge.second, dfa.graph));
}

std::list<RLC::Edge> Graph::out_edges( const RLC::Vertice & vertice) const
{
    std::list<RLC::Edge> edges;
    
    Graph_t::out_edge_iterator g_ei, g_end, dfa_beg, dfa_end, dfa_it;
    tie(g_ei,g_end) = boost::out_edges(vertice.first, transport->g);
    tie(dfa_beg,dfa_end) = boost::out_edges(vertice.second, dfa.graph);
    dfa_it = dfa_beg;
    
    while(g_ei != g_end && dfa_it != dfa_end) {
        while(dfa_it != dfa_end) {
            if(transport->map(*g_ei).type == dfa.graph[*dfa_it].type)
                edges.push_back(RLC::Edge(*g_ei, *dfa_it));
            ++dfa_it;
        }
        ++g_ei;
        dfa_it = dfa_beg;
    }
    return edges;
}

std::pair<bool, int> Graph::duration( const RLC::Edge & edge, const float start_sec, const int day) const
{
    return transport->duration_forward(edge.first, start_sec, day);
}

std::pair<bool, int> Graph::min_duration (const Edge & edge ) const
{
    return transport->min_duration(edge.first);
}

std::set<int> Graph::dfa_start_states() const
{
    std::set<int> states;
    states.insert(dfa.start_state);
    return states;
}

std::set<int> Graph::dfa_accepting_states() const
{
    return dfa.accepting_states;
}

int Graph::num_transport_vertices() const
{
    return transport->num_vertices();
}

int Graph::num_dfa_vertices() const
{
    return num_vertices(dfa.graph);
}


/****************** Backward Graph ************************/

BackwardGraph::BackwardGraph ( Graph* forward_graph ) :
AbstractGraph(false, forward_graph->transport),
forward_graph(forward_graph)
{
}

Vertice BackwardGraph::source ( const Edge & edge ) const
{
    return forward_graph->target( edge );
}

Vertice BackwardGraph::target ( const Edge & edge ) const
{
    return forward_graph->source( edge );
}

list< Edge > BackwardGraph::out_edges ( const Vertice & vertice ) const
{
    std::list<RLC::Edge> edges;
    
    Graph_t::in_edge_iterator g_ei, g_end, dfa_beg, dfa_end, dfa_it;
    tie(g_ei,g_end) = boost::in_edges(vertice.first, forward_graph->transport->g);
    tie(dfa_beg,dfa_end) = boost::in_edges(vertice.second, forward_graph->dfa.graph);
    dfa_it = dfa_beg;
    
    while(g_ei != g_end && dfa_it != dfa_end) {
        while(dfa_it != dfa_end) {
            if(forward_graph->transport->map(*g_ei).type == forward_graph->dfa.graph[*dfa_it].type)
                edges.push_back(RLC::Edge(*g_ei, *dfa_it));
            ++dfa_it;
        }
        ++g_ei;
        dfa_it = dfa_beg;
    }
    return edges;
}

std::pair<bool, int> BackwardGraph::duration ( const Edge & edge, const float start_sec, const int day ) const
{
    return forward_graph->transport->duration_backward(edge.first, start_sec, day);
}

std::pair<bool, int> BackwardGraph::min_duration ( const Edge & edge ) const
{
    return forward_graph->transport->min_duration(edge.first);
}

std::set< int > BackwardGraph::dfa_start_states() const
{
    return forward_graph->dfa_accepting_states();
}

std::set< int > BackwardGraph::dfa_accepting_states() const
{
    return forward_graph->dfa_start_states();
}

int BackwardGraph::num_transport_vertices() const
{
    return forward_graph->num_transport_vertices();
}

int BackwardGraph::num_dfa_vertices() const
{
    return forward_graph->num_dfa_vertices();
}










} // end RLC namespace
