#include "debug/cwd_sys.h"
#include "debug/cwd_debug.h"

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
        Edge e;
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

DFA all_dfa()
{
    DfaEdgeList edges;
    edges.push_back(pair<pair<int,int>,EdgeMode>(pair<int,int>(0, 0), FootEdge));
    edges.push_back(pair<pair<int,int>,EdgeMode>(pair<int,int>(0, 0), TransferEdge));
    edges.push_back(pair<pair<int,int>,EdgeMode>(pair<int,int>(0, 0), SubwayEdge));
    set<int> accepting;
    accepting.insert(0);
    
    return DFA(0, accepting, edges);
}


/**************************** RegLCGraph ***********************************/


RegLCGraph::RegLCGraph(Graph transport, DFA dfa) : 
transport(transport), 
dfa(dfa)
{
}


RLCVertice RegLCGraph::source(RLCEdge edge) 
{
    return RLCVertice(boost::source(edge.first, transport.g), boost::source(edge.second, dfa.graph));
}

RLCVertice RegLCGraph::target(RLCEdge edge)
{
    return RLCVertice(boost::target(edge.first, transport.g), boost::target(edge.second, dfa.graph));
}

std::list<RLCEdge> RegLCGraph::out_edges(RLCVertice vertice)
{
    std::list<RLCEdge> edges;
    
    Graph_t::out_edge_iterator g_ei, g_end, dfa_beg, dfa_end, dfa_it;
    tie(g_ei,g_end) = boost::out_edges(vertice.first, transport.g);
    tie(dfa_beg,dfa_end) = boost::out_edges(vertice.second, dfa.graph);
    dfa_it = dfa_beg;
    
    while(g_ei != g_end && dfa_it != dfa_end) {
        while(dfa_it != dfa_end) {
            if(transport.g[*g_ei].type == dfa.graph[*dfa_it].type)
                edges.push_back(RLCEdge(*g_ei, *dfa_it));
            ++dfa_it;
        }
        ++g_ei;
        dfa_it = dfa_beg;
    }
    return edges;
}

float RegLCGraph::duration(RLCEdge edge, float start_sec, int day)
{
    try {
        return transport.g[edge.first].duration(start_sec, day);
    } catch(No_traffic) {
        return -1.0f;
    }
}



void show_edges(RegLCGraph rlc, RLCVertice v)
{   
    try {
    
    Graph_t::out_edge_iterator ei, end;
    tie(ei,end) = out_edges(v.first, rlc.transport.g);
    
    cout << "Street edges : ";
    for(; ei != end; ei++) {
        Edge e = rlc.transport.g[*ei];
        cout << "("<< source(*ei, rlc.transport.g) <<", "<< target(*ei, rlc.transport.g) <<", "<<e.type<<", "<<e.duration(0,0)<<")   ";
    }
    
    cout << "\n DFA edges : ";
    tie(ei,end) = out_edges(v.second, rlc.dfa.graph);
    for(; ei != end; ei++) {
        Edge e = rlc.transport.g[*ei];
        cout << "("<< source(*ei, rlc.transport.g) <<", "<< target(*ei, rlc.transport.g) <<", "<<e.type<<", "<<e.duration(0,0)<<")   ";
    }    
    
    cout << "\nRLC Edges : ";
    std::list<RLCEdge>::iterator it;
    std::list<RLCEdge> edges = rlc.out_edges(v);
    for(it = edges.begin() ; it != edges.end() ; ++it) {
        cout << "( "<< rlc.source(*it).first <<"-"<< rlc.source(*it).second <<", "
             << rlc.target(*it).first <<"-"<< rlc.target(*it).second <<", "<<rlc.duration(*it, 0,0)<<")   ";
    }
    cout <<endl;
                
    
} catch(No_traffic) {}
}





Dijkstra::Dijkstra( RegLCGraph* graph, int source, int dest, float start_sec, int start_day ) :
source(source), dest(dest), start_sec(start_sec), start_day(start_day), path_found(false),
heap(Compare(&(this->arr_times)))
{
    this->graph = graph;
    
    const int g_num_vert = num_vertices(graph->transport.g);
    const int dfa_num_vert = num_vertices(graph->dfa.graph);
    
    arr_times = new float*[dfa_num_vert];
    references = new Heap::handle_type*[dfa_num_vert];
    status = new uint*[dfa_num_vert];
    predecessors = new RLCEdge*[dfa_num_vert];
    for(int i=0 ; i<dfa_num_vert ; ++i) {
        arr_times[i] = new float[g_num_vert];
        references[i] = new Heap::handle_type[g_num_vert];
        status[i] = new uint[g_num_vert];
        predecessors[i] = new RLCEdge[g_num_vert];
        //TODO: use a memset, possibly merge all those arrays
        for(int j=0 ; j<g_num_vert ; ++j)
            status[i][j] = 0;
    }
    
    Dout(dc::notice, "Building Dijkstra object on graph with "<<g_num_vert<<" nodes in Graph & "<<dfa_num_vert<<" nodes in DFA");    
}

Dijkstra::~Dijkstra()
{
    const int dfa_num_vert = num_vertices(graph->dfa.graph);
    
    for(int i=0 ; i<dfa_num_vert ; ++i) {
        delete[] arr_times[i];
        delete[] references[i];
        delete[] status[i];
        delete[] predecessors[i];
    }
    delete[] arr_times;
    delete[] references;
    delete[] status;
    delete[] predecessors;
}
using namespace std;
bool Dijkstra::run()
{
    Dout(dc::notice, "Running Dijkstra on RegLCGraph from node "<<source <<" to node "<<dest);
    RLCVertice rlc_start, rlc_dest;
    rlc_start.first = source;
    rlc_start.second = graph->dfa.start_state;
    rlc_dest.first = dest;
    rlc_dest.second = *(graph->dfa.accepting_states.begin());
    
    set_arrival(rlc_start, start_sec);
    set_gray(rlc_start);
    {
        Graph_t::edge_iterator g_tei, dfa_tei, tend ;
        tie(g_tei,tend) = edges(graph->transport.g);
        tie(dfa_tei,tend) = edges(graph->dfa.graph);
        put_dij_node(rlc_start);
    }
    
    while( !heap.empty() ) 
    {
        Dij_node curr = heap.top();
        heap.pop();
        set_black(curr.v);
        
        Dout(dc::notice, "Current node ("<<curr.v.first<<", "<<curr.v.second<<")");
        
        if( curr.v == rlc_dest)
            break;
        
        list<RLCEdge> n_out_edges = graph->out_edges(curr.v);
        BOOST_FOREACH(RLCEdge e, n_out_edges) 
        {
            RLCVertice target = graph->target(e);
                
            float target_arr = graph->duration(e, arrival(curr.v), start_day);
            
            Dout(dc::notice, " - edge {target: ("<<target.first<<", "<<target.second<<") }");
            
            //TODO: Only useful for debugging 
            //touched_edges.push_back( graph->g.g[e.first].edge_index );
            
            if(target_arr >= 0.0f && white(target))
            {
                Dout(dc::notice, " -- Inserting target into heap : ("<< target.first<<", "<<target.second<<")");
                set_arrival(target, target_arr);
                set_pred(target, e);
                put_dij_node(target);
                set_gray(target);
            }
            else if(target_arr >= 0.0f && target_arr < arrival(target))
            {
                BOOST_ASSERT(!black(target));
                Dout(dc::notice, " -- Updating target in heap : ("<< target.first<<", "<<target.second<<")");
                set_arrival(target, target_arr);
                set_pred(target, e);
                heap.update(handle(target));
            } else if(target_arr < 0.0f) {
                Dout(dc::notice, " -- No traffic on this edge");
            }
            else
            {
                Dout(dc::notice, " -- Edge not interesting");
            }
        }
    }
    
    RLCVertice curr;
    
    if(black(rlc_dest)) 
    {
        path_found = true;
        path_arrival = arrival(rlc_dest);
        for(curr = rlc_dest; curr != rlc_start; curr = graph->source(get_pred(curr))) 
            path.push_front(get_pred(curr));
    } 
    else  // target wasn't reached
    { 
        Dout(dc::notice, "Unable to reach target");
    }
    
    return path_found;
}

EdgeList Dijkstra::get_transport_path()
{
    EdgeList edges;
    BOOST_FOREACH(RLCEdge e, path) 
    {
        edges.push_back(graph->transport.g[e.first].edge_index);
    }
    return edges;
}




} // end RLC namespace
