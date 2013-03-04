#include "debug/cwd_sys.h"
#include "debug/cwd_debug.h"

#include "reglc_graph.h"



using namespace std;
using namespace boost;


namespace RLC {
    
void show_edges(Graph *rlc, RLC::Vertice v);


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

DFA pt_foot_dfa()
{
    DfaEdgeList edges;
    edges.push_back(pair<pair<int,int>,EdgeMode>(pair<int,int>(0, 0), FootEdge));
    edges.push_back(pair<pair<int,int>,EdgeMode>(pair<int,int>(0, 0), TransferEdge));
    edges.push_back(pair<pair<int,int>,EdgeMode>(pair<int,int>(0, 0), SubwayEdge));
    edges.push_back(pair<pair<int,int>,EdgeMode>(pair<int,int>(0, 0), BusEdge));
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

/**************************** Graph ***********************************/


Graph::Graph(Transport::Graph* transport, DFA dfa) : 
AbstractGraph(true, transport),
transport(transport), 
dfa(dfa)
{
}


RLC::Vertice Graph::source(RLC::Edge edge) 
{
    return RLC::Vertice(boost::source(edge.first, transport->g), boost::source(edge.second, dfa.graph));
}

RLC::Vertice Graph::target(RLC::Edge edge)
{
    return RLC::Vertice(boost::target(edge.first, transport->g), boost::target(edge.second, dfa.graph));
}

std::list<RLC::Edge> Graph::out_edges(RLC::Vertice vertice)
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

std::pair<bool, int> Graph::duration(RLC::Edge edge, float start_sec, int day)
{
    return transport->g[edge.first].duration(start_sec, day);
}

std::set<int> Graph::dfa_start_states()
{
    std::set<int> states;
    states.insert(dfa.start_state);
    return states;
}

std::set<int> Graph::dfa_accepting_states()
{
    return dfa.accepting_states;
}

int Graph::num_transport_vertices()
{
    return num_vertices(transport->g);
}

int Graph::num_dfa_vertices()
{
    return num_vertices(dfa.graph);
}


/****************** Backward Graph ************************/

BackwardGraph::BackwardGraph ( Graph* forward_graph ) :
AbstractGraph(false, forward_graph->transport),
forward_graph(forward_graph)
{
}

Vertice BackwardGraph::source ( Edge edge )
{
    return forward_graph->target( edge );
}

Vertice BackwardGraph::target ( Edge edge )
{
    return forward_graph->source( edge );
}

list< Edge > BackwardGraph::out_edges ( Vertice vertice )
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

std::pair<bool, int> BackwardGraph::duration ( Edge edge, float start_sec, int day )
{
    return forward_graph->transport->g[edge.first].duration(start_sec, day, true);
}


std::set< int > BackwardGraph::dfa_start_states()
{
    return forward_graph->dfa_accepting_states();
}

std::set< int > BackwardGraph::dfa_accepting_states()
{
    return forward_graph->dfa_start_states();
}

int BackwardGraph::num_transport_vertices()
{
    return forward_graph->num_transport_vertices();
}

int BackwardGraph::num_dfa_vertices()
{
    return forward_graph->num_dfa_vertices();
}



void show_edges(Graph *rlc, RLC::Vertice v)
{   
    try {
    
    Graph_t::out_edge_iterator ei, end;
    tie(ei,end) = out_edges(v.first, rlc->transport->g);
    
    cout << "Street edges : ";
    for(; ei != end; ei++) {
        ::Edge e = rlc->transport->g[*ei];
        cout << "("<< source(*ei, rlc->transport->g) <<", "<< target(*ei, rlc->transport->g) <<", "<<e.type<<", "<<e.duration(0,0).second<<")   ";
    }
    
    cout << "\n DFA edges : ";
    tie(ei,end) = out_edges(v.second, rlc->dfa.graph);
    for(; ei != end; ei++) {
        ::Edge e = rlc->transport->g[*ei];
        cout << "("<< source(*ei, rlc->transport->g) <<", "<< target(*ei, rlc->transport->g) <<", "<<e.type<<", "<<e.duration(0,0).second<<")   ";
    }    
    
    cout << "\nRLC Edges : ";
    std::list<RLC::Edge>::iterator it;
    std::list<RLC::Edge> edges = rlc->out_edges(v);
    for(it = edges.begin() ; it != edges.end() ; ++it) {
        cout << "( "<< rlc->source(*it).first <<"-"<< rlc->source(*it).second <<", "
             << rlc->target(*it).first <<"-"<< rlc->target(*it).second <<", "<<rlc->duration(*it, 0,0).second<<")   ";
    }
    cout <<endl;
                
    
} catch(No_traffic) {}
}





Dijkstra::Dijkstra( AbstractGraph* graph, int source, int dest, float start_sec, int start_day ) :
source( source ), dest( dest ), 
start_sec( start_sec ), start_day( start_day ), 
path_found( false ),
heap( Compare(graph->forward, &(this->arr_times)) ),
trans_num_vert( graph->num_transport_vertices() ),
dfa_num_vert( graph->num_dfa_vertices() )
{
    this->graph = graph;
    
    arr_times = new float*[dfa_num_vert];
    references = new Heap::handle_type*[dfa_num_vert];
    status = new uint*[dfa_num_vert];
    predecessors = new RLC::Edge*[dfa_num_vert];
    for(int i=0 ; i<dfa_num_vert ; ++i) {
        arr_times[i] = new float[trans_num_vert];
        references[i] = new Heap::handle_type[trans_num_vert];
        status[i] = new uint[trans_num_vert];
        predecessors[i] = new RLC::Edge[trans_num_vert];
        //TODO: use a memset, possibly merge all those arrays
        for(int j=0 ; j<trans_num_vert ; ++j)
            status[i][j] = 0;
    }
    
    BOOST_FOREACH(int v_dfa, graph->dfa_start_states()) {
        source_vertices.insert(Vertice(source, v_dfa));
    }
    BOOST_FOREACH(int v_dfa, graph->dfa_accepting_states()) {
        dest_vertices.insert(Vertice(dest, v_dfa));
    }
    
    Dout(dc::notice, "Building Dijkstra object on graph with "<<g_num_vert<<" nodes in Graph & "<<dfa_num_vert<<" nodes in DFA");    
}

Dijkstra::~Dijkstra()
{    
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
    std::cout << "Running Dijkstra on RegLCGraph from node "<<source <<" to node "<<dest<<"on time ("<<start_sec<<" "<<start_day<<")\n";
    
    BOOST_FOREACH(RLC::Vertice rlc_start, source_vertices) {
        set_arrival(rlc_start, start_sec);
        set_gray(rlc_start);

        put_dij_node(rlc_start);
    }
    
    while( !heap.empty() ) 
    {
        Dij_node curr = heap.top();
        heap.pop();
        set_black(curr.v);
        
        Dout(dc::notice, "Current node ("<<curr.v.first<<", "<<curr.v.second<<") "<<arrival(curr.v));
        
        if( dest_vertices.find(curr.v) != dest_vertices.end())
            break;
        
        list<RLC::Edge> n_out_edges = graph->out_edges(curr.v);
        BOOST_FOREACH(RLC::Edge e, n_out_edges) 
        {
            RLC::Vertice target = graph->target(e);
            
            bool has_traffic;
            int target_arr;
            boost::tie(has_traffic, target_arr) = graph->duration(e, arrival(curr.v), start_day);
            
            Dout(dc::notice, " - edge {target: ("<<target.first<<", "<<target.second<<") }");
            
            //TODO: Only useful for debugging 
            //touched_edges.push_back( graph->g.g[e.first].edge_index );
            
            if(has_traffic && white(target))
            {
                Dout(dc::notice, " -- Inserting target into heap : ("<< target.first<<", "<<target.second<<")");
                set_arrival(target, target_arr);
                set_pred(target, e);
                put_dij_node(target);
                set_gray(target);
            }
            else if(has_traffic && ( ( graph->forward && (target_arr < arrival(target)) ) 
                                           || ( (!graph->forward) && (target_arr > arrival(target)) )))
            {
                Dout(dc::notice, " -- Updating target in heap : ("<< target.first<<", "<<target.second<<") new: "<<target_arr
                                  << " old: "<<arrival(target)                );
                BOOST_ASSERT(!black(target));
                
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
    
    
    
    std::set<Vertice>::iterator dest_cur = dest_vertices.begin();
    while(dest_cur != dest_vertices.end() && !black(*dest_cur))
        dest_cur++;
    
    if(dest_cur != dest_vertices.end()) 
    {
        RLC::Vertice curr, rlc_dest;
        rlc_dest = *dest_cur;
        path_found = true;
        path_arrival = arrival(rlc_dest);
        for(curr = rlc_dest; source_vertices.find(curr) == source_vertices.end(); curr = graph->source(get_pred(curr))) 
            path.push_front(get_pred(curr));
        
        cout << "Path found : duration = " << path_arrival - start_sec << endl;
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
    BOOST_FOREACH(RLC::Edge e, path) 
    {
        edges.push_back(graph->transport->g[e.first].edge_index);
    }
    return edges;
}




} // end RLC namespace
