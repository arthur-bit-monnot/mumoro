#include "debug/cwd_sys.h"
#include "debug/cwd_debug.h"

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
        e.duration = Duration(1);
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
    return RLC::Vertice(boost::source(edge.first, transport->g), boost::source(edge.second, dfa.graph));
}

RLC::Vertice Graph::target( const RLC::Edge & edge) const
{
    return RLC::Vertice(boost::target(edge.first, transport->g), boost::target(edge.second, dfa.graph));
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
            if(transport->g[*g_ei].type == dfa.graph[*dfa_it].type)
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
    return transport->g[edge.first].duration(start_sec, day);
}

std::pair<bool, int> Graph::min_duration (const Edge & edge ) const
{
    return transport->g[edge.first].duration.min_duration();
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
    return num_vertices(transport->g);
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
            if(forward_graph->transport->g[*g_ei].type == forward_graph->dfa.graph[*dfa_it].type)
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
    return forward_graph->transport->g[edge.first].duration(start_sec, day, true);
}

std::pair<bool, int> BackwardGraph::min_duration ( const Edge & edge ) const
{
    return forward_graph->transport->g[edge.first].duration.min_duration();
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
