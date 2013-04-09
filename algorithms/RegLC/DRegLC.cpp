#include "DRegLC.h"

using namespace std;

namespace RLC {

DRegLC::DRegLC( ParamType parameters ) :
heap( DRegCompare(&(this->costs)) ),
success( false )
{
    DRegLCParams & p = parameters.value;
    
    this->graph = p.graph;
    day = p.day;
    cost_factor = p.cost_factor;
    
    trans_num_vert = graph->num_transport_vertices();
    dfa_num_vert = graph->num_dfa_vertices();
    
    arr_times = new float*[dfa_num_vert];
    costs = new int*[dfa_num_vert];
    references = new DRegHeap::handle_type*[dfa_num_vert];
    status = new uint*[dfa_num_vert];
    has_predecessor = new boost::dynamic_bitset<>*[dfa_num_vert];
    predecessors = new RLC::Edge*[dfa_num_vert];
    for(int i=0 ; i<dfa_num_vert ; ++i) {
        arr_times[i] = new float[trans_num_vert];
        costs[i] = new int[trans_num_vert];
        references[i] = new DRegHeap::handle_type[trans_num_vert];
        status[i] = new uint[trans_num_vert];
        has_predecessor[i] = new boost::dynamic_bitset<>(trans_num_vert);
        predecessors[i] = (RLC::Edge *) malloc(trans_num_vert * sizeof(RLC::Edge));

        // all vertices are white
        memset(status[i], 0, trans_num_vert * sizeof(status[0][0]));
    }
}

DRegLC::~DRegLC()
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

bool DRegLC::run()
{    
    while( !finished() ) 
    {
       treat_next();
    }
        
    return success;
}

bool DRegLC::finished() const
{
    return heap.empty();
}


Vertice DRegLC::treat_next() 
{
    RLC::Vertice curr = heap.top();
    heap.pop();
    set_black(curr);
    
    if( check_termination(curr) ) {
        success = true;
        return curr;
    }
    
    list<RLC::Edge> n_out_edges = graph->out_edges(curr);
    BOOST_FOREACH(RLC::Edge e, n_out_edges) 
    {
        RLC::Vertice target = graph->target(e);
        
        bool has_traffic;
        int edge_cost;
        boost::tie(has_traffic, edge_cost) = duration(e, arrival(curr), day);

        int target_cost = cost(curr) + edge_cost * cost_factor;
        float target_arr;
        if(graph->forward)
            target_arr = arrival(curr) + edge_cost;
        else 
            target_arr = arrival(curr) - edge_cost;
        
        if(has_traffic) {
//             BOOST_ASSERT(edge_cost >= 0);
//             TODO remove inconsistent data from generated graph
            if(edge_cost >= 0) {
                insert_node_with_predecessor(target, target_arr, target_cost, e);
            }
        }
        
        
    }
    return curr;
}

bool DRegLC::insert_node_with_predecessor ( const Vertice & vert, const int arr, const int vert_cost, const RLC::Edge & pred )
{
    bool was_inserted = insert_node( vert, arr, vert_cost );
    if( was_inserted )
        set_pred( vert, pred );
    return was_inserted;
}

bool DRegLC::insert_node ( const Vertice & vert, const int arr, const int vert_cost )
{
    if( white(vert) )
    {
        set_arrival(vert, arr);
        set_cost(vert, vert_cost);
        put_dij_node(vert);
        set_gray(vert);
        
        return true;
    }
    else if( vert_cost < cost(vert) )
    {
        BOOST_ASSERT(!black(vert));
        
        set_arrival(vert, arr);
        set_cost(vert, vert_cost);
        heap.update(handle(vert));

        return true;
    }
    else
    {
        return false;
    }
}

}
