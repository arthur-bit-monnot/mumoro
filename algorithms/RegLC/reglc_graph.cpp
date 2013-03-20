#include "debug/cwd_sys.h"
#include "debug/cwd_debug.h"

#include <boost/heap/fibonacci_heap.hpp>
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

std::pair<bool, int> Graph::min_duration (const Edge edge ) const
{
    return transport->g[edge.first].duration.min_duration();
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

std::pair<bool, int> BackwardGraph::min_duration ( const Edge edge ) const
{
    return forward_graph->transport->g[edge.first].duration.min_duration();
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





Dijkstra::Dijkstra( AbstractGraph* graph, int source, int dest, float start_sec, int start_day, DijkstraParameters params ) :
params(params), source( source ), dest( dest ), 
start_sec( start_sec ), start_day( start_day ), 
path_found( false ),
heap( Compare(&(this->costs)) ),
trans_num_vert( graph->num_transport_vertices() ),
dfa_num_vert( graph->num_dfa_vertices() )
{
    this->graph = graph;
    
    arr_times = new float*[dfa_num_vert];
    costs = new int*[dfa_num_vert];
    references = new Heap::handle_type*[dfa_num_vert];
    status = new uint*[dfa_num_vert];
    predecessors = new Predecessor*[dfa_num_vert];
    for(int i=0 ; i<dfa_num_vert ; ++i) {
        arr_times[i] = new float[trans_num_vert];
        costs[i] = new int[trans_num_vert];
        references[i] = new Heap::handle_type[trans_num_vert];
        status[i] = new uint[trans_num_vert];
        predecessors[i] = new Predecessor[trans_num_vert];
        
        // all vertices are white
        memset(status[i], 0, trans_num_vert * sizeof(status[0][0]));
    }
    
    // in some case, sources might be added while running
    if(source != -1) {
        BOOST_FOREACH(int v_dfa, graph->dfa_start_states()) {
            source_vertices.insert(Vertice(source, v_dfa));
        }
    }
    
    // if dest == -1 there is no goals, the set is empty
    if(dest != -1) {
        BOOST_FOREACH(int v_dfa, graph->dfa_accepting_states()) {
            dest_vertices.insert(Vertice(dest, v_dfa));
        }
    }
    
    // insert start vertices in heap
    BOOST_FOREACH(RLC::Vertice rlc_start, source_vertices) {
        set_arrival(rlc_start, start_sec);
        set_cost(rlc_start, 0);
        set_gray(rlc_start);

        put_dij_node(rlc_start);
    }
    
    Dout(dc::notice, "Building Dijkstra object on graph with "<<trans_num_vert<<" nodes in Graph & "<<dfa_num_vert<<" nodes in DFA");    
}

Dijkstra::~Dijkstra()
{
    for(int i=0 ; i<dfa_num_vert ; ++i) {
        delete[] arr_times[i];
        delete[] costs[i];
        delete[] references[i];
        delete[] status[i];
        delete[] predecessors[i];
    }
    delete[] arr_times;
    delete[] costs;
    delete[] references;
    delete[] status;
    delete[] predecessors;
}
using namespace std;
bool Dijkstra::run()
{
    Dout(dc::notice, "Running Dijkstra on RegLCGraph from node "<<source <<" to node "<<dest);
    
    while( !heap.empty() && !path_found ) 
    {
       treat_next();
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
        path_cost = cost(rlc_dest);
        for(curr = rlc_dest ; has_pred(curr) ; curr = graph->source(get_pred(curr))) 
            path.push_front(get_pred(curr));
    } 
    else  // target wasn't reached
    { 
        Dout(dc::notice, "Unable to reach target");
    }
    
    return path_found;
}

Vertice Dijkstra::treat_next() 
{
    Dij_node curr = heap.top();
    heap.pop();
    set_black(curr.v);
    
    Dout(dc::notice, "Current node ("<<curr.v.first<<", "<<curr.v.second<<") "<<arrival(curr.v));
    
    if( dest_vertices.find(curr.v) != dest_vertices.end()) {
        path_found = true;
        return curr.v;
    }
    
    list<RLC::Edge> n_out_edges = graph->out_edges(curr.v);
    BOOST_FOREACH(RLC::Edge e, n_out_edges) 
    {
        RLC::Vertice target = graph->target(e);
        
        bool has_traffic;
        int edge_cost;
        if(params.use_cost_lower_bounds) {
            boost::tie(has_traffic, edge_cost) = graph->min_duration(e);
        } else {
            boost::tie(has_traffic, edge_cost) = graph->duration(e, arrival(curr.v), start_day);
        }
        int target_cost = cost(curr.v) + edge_cost;
        float target_arr;
        if(graph->forward)
            target_arr = arrival(curr.v) + edge_cost;
        else 
            target_arr = arrival(curr.v) - edge_cost;
        
        Dout(dc::notice, " - edge {target: ("<<target.first<<", "<<target.second<<") }");   
        
        if(has_traffic) {
            BOOST_ASSERT(edge_cost >= 0);
            insert_node(target, target_arr, target_cost, Predecessor(e));
        }
        
        
    }
    return curr.v;
}

bool Dijkstra::insert_node ( Vertice vert, int arr, int vert_cost, Predecessor pred )
{
    // check if we are above the cost limit
    if( params.cost_limit && vert_cost > params.cost_limit_value )
        return false;
    
    if( white(vert) )
    {
        Dout(dc::notice, " -- Inserting target into heap : ("<< vert.first<<", "<<vert.second<<")");
        set_arrival(vert, arr);
        set_cost(vert, vert_cost);
        set_pred(vert, pred);
        put_dij_node(vert);
        set_gray(vert);
        
        if( params.save_touched_nodes )
            touched_nodes.push_back(vert);
        
        return true;
    }
    else if( vert_cost < cost(vert) )
    {
        Dout(dc::notice, " -- Updating target in heap : ("<< vert.first<<", "<<vert.second<<") new: "<<arr
                            << " old: "<<arrival(vert)                );

        BOOST_ASSERT(!black(vert));
        
        set_arrival(vert, arr);
        set_cost(vert, vert_cost);
        set_pred(vert, pred);
        heap.update(handle(vert));
        
        if( params.save_touched_nodes )
            touched_nodes.push_back(vert);
        
        return true;
    }
    else
    {
        Dout(dc::notice, " -- Edge not interesting");
        return false;
    }
}



VisualResult Dijkstra::get_result()
{
    VisualResult res(graph->transport) ;
    BOOST_FOREACH(RLC::Edge e, path) 
    {
        res.edges.push_back(graph->transport->g[e.first].edge_index);
    }
    res.a_nodes.push_back(source);
    res.b_nodes.push_back(dest);
    return res;
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
