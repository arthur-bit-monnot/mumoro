#include "debug/cwd_sys.h"
#include "debug/cwd_debug.h"

#include <boost/foreach.hpp> 

#include "reglc_dijkstra.h"


namespace RLC {

Dijkstra::Dijkstra( AbstractGraph* graph, int source, int dest, float start_sec, int start_day, DijkstraParameters * params ) :
params(params), source( source ), dest( dest ), 
start_sec( start_sec ), start_day( start_day ), 
path_found( false ),
visited_nodes( 0 ),
heap( Compare(&(this->costs)) ),
trans_num_vert( graph->num_transport_vertices() ),
dfa_num_vert( graph->num_dfa_vertices() )
{
    if(params == NULL)
        this->params = new DijkstraParameters();
    
    this->graph = graph;
    
    arr_times = new float*[dfa_num_vert];
    costs = new int*[dfa_num_vert];
    references = new Heap::handle_type*[dfa_num_vert];
    status = new uint*[dfa_num_vert];
    has_predecessor = new boost::dynamic_bitset<>*[dfa_num_vert];
    predecessors = new RLC::Edge*[dfa_num_vert];
    for(int i=0 ; i<dfa_num_vert ; ++i) {
        arr_times[i] = new float[trans_num_vert];
        costs[i] = new int[trans_num_vert];
        references[i] = new Heap::handle_type[trans_num_vert];
        status[i] = new uint[trans_num_vert];
        has_predecessor[i] = new boost::dynamic_bitset<>(trans_num_vert);
        predecessors[i] = (RLC::Edge *) malloc(trans_num_vert * sizeof(RLC::Edge));

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
        Predecessor pred; pred.has_pred = false; //empty predecessor object
        insert_node(rlc_start, start_sec, 0, pred);
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
        delete has_predecessor[i];
        delete[] predecessors[i];
    }
    delete[] arr_times;
    delete[] costs;
    delete[] references;
    delete[] status;
    delete[] has_predecessor;
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
    } else {
        visited_nodes += 1;
    }
    
    list<RLC::Edge> n_out_edges = graph->out_edges(curr.v);
    BOOST_FOREACH(RLC::Edge e, n_out_edges) 
    {
        RLC::Vertice target = graph->target(e);
        
        bool has_traffic;
        int edge_cost;
        if(params->use_cost_lower_bounds) {
            boost::tie(has_traffic, edge_cost) = graph->min_duration(e);
        } else {
            boost::tie(has_traffic, edge_cost) = graph->duration(e, arrival(curr.v), start_day);
        }
        int target_cost = cost(curr.v) + edge_cost * params->cost_factor;
        float target_arr;
        if(graph->forward)
            target_arr = arrival(curr.v) + edge_cost;
        else 
            target_arr = arrival(curr.v) - edge_cost;
        
        Dout(dc::notice, " - edge {target: ("<<target.first<<", "<<target.second<<") }");   
        
        if(has_traffic) {
//             BOOST_ASSERT(edge_cost >= 0);
            //TODO remove inconsistent data from generated graph
            if(edge_cost >= 0) {
                Predecessor pred;
                pred.has_pred = true;
                pred.pred = e;
                insert_node(target, target_arr, target_cost, pred);
            }
        }
        
        
    }
    return curr.v;
}

bool Dijkstra::insert_node ( const Vertice & vert, const int arr, const int vert_cost, const Predecessor & pred )
{
    // check if we are above the cost limit
    if( params->cost_limit && vert_cost > params->cost_limit_value )
        return false;
    
    // Check if the node is in the restricted graph
    if( params->filter_nodes && !params->filter->isIn( vert.first ) )
        return false;
    
    if( white(vert) )
    {
        Dout(dc::notice, " -- Inserting target into heap : ("<< vert.first<<", "<<vert.second<<")");
        set_arrival(vert, arr);
        set_cost(vert, vert_cost);
        if(pred.has_pred)
            set_pred(vert, pred.pred);
        put_dij_node(vert);
        set_gray(vert);
        
        if( params->save_touched_nodes )
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
        if(pred.has_pred)
            set_pred(vert, pred.pred);
        heap.update(handle(vert));
        
        if( params->save_touched_nodes )
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

} // end namespace RLC