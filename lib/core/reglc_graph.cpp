#include "reglc_graph.h"

using namespace std;
using namespace boost;





/*************************** DFA ************************/

DFA::DFA(int start, std::set<int> accepting, DfaEdgeList edges) :
start_state(start), accepting_states(accepting)
{
    DfaEdgeList::iterator it;
    for(it = edges.begin() ; it != edges.end() ; ++it) {
        Edge e;
        e.type = (EdgeMode) it->second;
        e.duration = Duration(1);
        boost::add_edge(it->first.first, it->first.first, e, graph);
    }
    
}

DFA default_dfa()
{
    DfaEdgeList edges;
    edges.push_back(pair<pair<int,int>,EdgeMode>(pair<int,int>(1, 1), FootEdge));
    edges.push_back(pair<pair<int,int>,EdgeMode>(pair<int,int>(1, 1), TransferEdge));
    edges.push_back(pair<pair<int,int>,EdgeMode>(pair<int,int>(1, 1), SubwayEdge));
    edges.push_back(pair<pair<int,int>,EdgeMode>(pair<int,int>(1, 1), FootEdge));
    edges.push_back(pair<pair<int,int>,EdgeMode>(pair<int,int>(1, 1), SubwayEdge));
    edges.push_back(pair<pair<int,int>,EdgeMode>(pair<int,int>(1, 1), TransferEdge));
    set<int> accepting;
    accepting.insert(1);
    
    return DFA(0, accepting, edges);
}



/**************************** RegLCGraph ***********************************/


RegLCGraph::RegLCGraph(Graph g, DFA dfa) : 
g(g), 
dfa(dfa),
length_dfa( ceil(log2((double) num_vertices(dfa.graph))) ),
length_graph( ceil(log2((double) num_vertices(g.g))) ),
_max_vertices( boost::num_vertices(g.g) << length_dfa )
{
    vertice_conv_mask = 0x00;
    for(uint i=0 ; i<length_dfa ; ++i)
        vertice_conv_mask |= (0x01 << i);
    
    BOOST_ASSERT(length_dfa + length_graph <= sizeof(uint));
}
    
RLCVertice RegLCGraph::toVertice(uint composed_vertice) {
    const uint dfa_vertice = composed_vertice & vertice_conv_mask;
    const uint graph_vertice = (composed_vertice - dfa_vertice) >> 4;
    
    BOOST_ASSERT(toInt(RLCVertice(graph_vertice, dfa_vertice)) == composed_vertice);
    return RLCVertice(graph_vertice, dfa_vertice);
}

uint RegLCGraph::toInt(RLCVertice vertice) {
    const uint graph_vertice = vertice.first << 4;
    const uint dfa_vertice = vertice.second;
    uint composed_vertice = graph_vertice | dfa_vertice;
    
    return composed_vertice;
}


RLCVertice RegLCGraph::source(RLCEdge edge) 
{
    return RLCVertice(boost::source(edge.first, g.g), boost::source(edge.second, dfa.graph));
}

RLCVertice RegLCGraph::target(RLCEdge edge)
{
    return RLCVertice(boost::target(edge.first, g.g), boost::target(edge.second, dfa.graph));
}

std::list<RLCEdge> RegLCGraph::out_edges(RLCVertice vertice)
{
    std::list<RLCEdge> edges;
    
    Graph_t::out_edge_iterator g_ei, g_end, dfa_beg, dfa_end, dfa_it;
    tie(g_ei,g_end) = boost::out_edges(vertice.first, g.g);
    tie(dfa_beg,dfa_end) = boost::out_edges(vertice.second, dfa.graph);
    dfa_it = dfa_beg;
    
//     cout <<"-- Out edges : \n";
    
    while(g_ei != g_end && dfa_it != dfa_end) {
//         print_edge(*g_ei, g.g); cout << "\n";
        while(dfa_it != dfa_end) {
//             print_edge(*dfa_it, dfa.graph); cout << "\n";
            if(g.g[*g_ei].type == dfa.graph[*dfa_it].type)
                edges.push_back(RLCEdge(*g_ei, *dfa_it));
            ++dfa_it;
        }
//         cout << "\n";
        ++g_ei;
        dfa_it = dfa_beg;
    }
    return edges;
}

float RegLCGraph::duration(RLCEdge edge, float start_sec, int day)
{
    return g.g[edge.first].duration(start_sec, day);
}



void RLC_test(Graph g)
{
    RegLCGraph rlc(g, default_dfa());
    
    RLCVertice v(42, 0);
    
    Graph_t::out_edge_iterator ei, end;
    tie(ei,end) = out_edges(v.first, g.g);
    
    cout << "\nStreet edges : ";
    for(; ei != end; ei++) {
        Edge e = g.g[*ei];
        cout << "("<< source(*ei, g.g) <<", "<< target(*ei, g.g) <<", "<<e.type<<", "<<e.duration(0,0)<<")   ";
    }
    
    cout << "\n DFA edges : ";
    tie(ei,end) = out_edges(v.second, rlc.dfa.graph);
    for(; ei != end; ei++) {
        Edge e = g.g[*ei];
        cout << "("<< source(*ei, g.g) <<", "<< target(*ei, g.g) <<", "<<e.type<<", "<<e.duration(0,0)<<")   ";
    }    
    
    cout << "\nRLC Edges : ";
    std::list<RLCEdge>::iterator it;
    std::list<RLCEdge> edges = rlc.out_edges(v);
    for(it = edges.begin() ; it != edges.end() ; ++it) {
        cout << "( "<< rlc.source(*it).first <<"-"<< rlc.source(*it).second <<", "
             << rlc.target(*it).first <<"-"<< rlc.target(*it).second <<", "<<rlc.duration(*it, 0,0)<<")   ";
    }
}