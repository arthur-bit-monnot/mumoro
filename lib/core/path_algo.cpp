#include "debug/cwd_sys.h"

#include <boost/graph/adjacency_list.hpp>
#include <boost/heap/fibonacci_heap.hpp>
#include <boost/tuple/tuple.hpp>
#include <time.h>


#include "path_algo.h"
//#include "reglc_graph.h"



typedef boost::heap::fibonacci_heap<node_ptr, boost::heap::compare<Compare> > Heap;


// using namespace boost::multi_index;
using namespace boost;
using namespace std;


EdgeList dijkstra(int start, int destination, float dep_sec, int dep_day, Transport::Graph g) {
    
    clock_t start_cpu_time, end_cpu_time;
    double cpu_time_used;
     
    start_cpu_time = clock();
    
    Graph_t::edge_iterator tei, tend;
    tie(tei,tend) = edges(g.g);
    
    //Vecteur de distance
    vector<float> distance(num_vertices(g.g), std::numeric_limits<float>::max());
    
    // Vecteur des prédcesseurs
    vector<edge_t> predecessor(num_vertices(g.g), *tei);
    
    // Vecteurs des references (objets stockés dans le tas)
    std::vector<Heap::handle_type> references;
    
    Compare comp(distance);
    
    Heap heap(comp);
    
    for(uint i=0 ; i<num_vertices(g.g) ; ++i)
        references.push_back(heap.push(node_ptr(i)));
    
    distance[start] = dep_sec;
    predecessor[start] = *tei;
    heap.update(references[start]); 
    
    while (!heap.empty())
    {
        node_ptr np = heap.top();
        heap.pop();
        
        std::cout << "Current node : "<< np.index << " "<<distance[np.index]<<" -- ";

        if (np.index == destination)
            break;

        Graph_t::out_edge_iterator ei, end;
        tie(ei,end) = out_edges(np.index, g.g);
        for(; ei != end; ei++) {
            Edge e = g.g[*ei];
            if(e.type == FootEdge || e.type == TransferEdge || e.type == SubwayEdge) 
            {
                cout << "("<<e.edge_index;
                int target = boost::target(*ei, g.g);
                
                bool has_traffic;
                int dist;
                boost::tie(has_traffic, dist) = e.duration(distance[np.index], dep_day);
                cout << ", "<<target<<", "<<dist<<", "<<") ";
                
                if(dist < distance[target])
                {
                    distance[target] = dist;
                    predecessor[target] = *ei;
                    heap.update(references[target]);
                }
            }
        }
        cout << "\n";
    }
    
    end_cpu_time = clock();
    cpu_time_used = ((double) (end_cpu_time - start_cpu_time)) / CLOCKS_PER_SEC;
    
    EdgeList path;
    
    
    for(int i = destination; i != start; i = source(predecessor[i], g.g))
        path.push_front(g.g[predecessor[i]].edge_index);
    
    std::cout << start << " -> " << destination << " \n\nPath found in " 
              << cpu_time_used <<" cup sec. Duration : "<< distance[destination]-distance[start] << "\n\n";
              
    EdgeList::iterator it;
    for(it = path.begin(); it != path.end() ; it++) {
        std::cout << g.sourceNode(*it) <<" ";
        
    cout << "\n";
    }
    
    return path;
}


MeetPoint::MeetPoint ( int startA, int startB, int dep_sec, int dep_day, Transport::Graph * g ) :
startA(startA), startB(startB), dep_sec(dep_sec), dep_day(dep_day), g(g),
dfaA(RLC::pt_foot_dfa()), dfaB(RLC::pt_dfa()),
rlcA(RLC::Graph(g, dfaA)), rlcB(RLC::Graph(g, dfaB)),
dijA(RLC::Dijkstra(&rlcA, startA, -1, (float) dep_sec, dep_day)),
dijB(RLC::Dijkstra(&rlcB, startB, -1, (float) dep_sec, dep_day)),
nf(cap_jj_nf(g->g)),
vres(g)
{
}

bool MeetPoint::run()
{
    for(int i=0 ; i< 3000 ; ++i)
    {
        int next_arr_a = dijA.arrival( dijA.heap.top().v );
        int next_arr_b = dijB.arrival( dijB.heap.top().v );
        
        RLC::Vertice last;
        if(next_arr_a < next_arr_b) {
            last = dijA.treat_next();
            if(last.first != startA)
                vres.edges.push_back( g->edgeIndex( dijA.get_pred(last).first ) );
            
            if(dfaA.accepting_states.find(last.second) != dfaA.accepting_states.end()) {
                // node is in an accepting state
                BOOST_FOREACH(int dfa_b_acc, dfaB.accepting_states) {
                    RLC::Vertice other;
                    other.first = last.first;
                    other.second = dfa_b_acc;
                    if(dijB.black(other)) {
                        cout << "Connection A ";
                        if(nf.isIn(last.first))
                            vres.b_nodes.push_back(last.first);
                        else
                            vres.c_nodes.push_back(last.first);
                    }
                }
            }
        }
        else {
            last = dijB.treat_next();
            if(last.first != startB)
                vres.edges.push_back( g->edgeIndex( dijB.get_pred(last).first ) );
            
            if(dfaB.accepting_states.find(last.second) != dfaB.accepting_states.end()) {
                // node is in an accepting state
                BOOST_FOREACH(int dfa_a_acc, dfaA.accepting_states) {
                    RLC::Vertice other;
                    other.first = last.first;
                    other.second = dfa_a_acc;
                    if(dijA.black(other)) {
                        cout << "Connection B ";
                        if(nf.isIn(last.first))
                            vres.b_nodes.push_back(last.first);
                        else
                            vres.c_nodes.push_back(last.first);
                    }
                }
            }
        }
    }
    return false;
}

VisualResult MeetPoint::get_result() const
{
    return vres;
}

SharedPath::SharedPath ( int startA, int startB, int dest, int dep_sec, int dep_day, Transport::Graph * g ) :
startA(startA), startB(startB), //dest(dest), startA(109256), startB(109246), 
dest(172807), 
dep_sec(dep_sec), dep_day(dep_day), g(g),
dfaA(RLC::foot_dfa()), dfaB(RLC::bike_pt_dfa()), dfaC(RLC::bike_pt_dfa()),
rlcA(RLC::Graph(g, dfaA)), rlcB(RLC::Graph(g, dfaB)), rlcC(RLC::Graph(g, dfaC)),
dijA(RLC::Dijkstra(&rlcA, this->startA, -1, (float) dep_sec, dep_day)),
dijB(RLC::Dijkstra(&rlcB, this->startB, -1, (float) dep_sec, dep_day)),
dijC(RLC::Dijkstra(&rlcC, -1, this->dest, (float) dep_sec, dep_day)),
nf(cap_jj_nf(g->g)),
vres(g)
{
    states = new VerticeState* [3];
    for(int i=0; i<3 ; ++i) {
        states[i] = new VerticeState[num_vertices(g->g)];
        memset(states[i], 0, sizeof(VerticeState) * num_vertices(g->g));
    }
}

bool SharedPath::run()
{
    int current_layer;
    RLC::Vertice curr_node;
    int count = 0;
    
    cerr<< dijA.heap.top().v.first <<" "<<dijA.heap.top().v.second << endl;
    cerr<< dijB.heap.top().v.first <<" "<<dijB.heap.top().v.second << endl;
    
    while( !dijC.path_found )
    {
        if( dijC.heap.empty()
            || dijA.arrival( dijA.heap.top().v ) < dijC.arrival( dijC.heap.top().v ) 
            || dijB.arrival( dijB.heap.top().v ) < dijC.arrival( dijC.heap.top().v )) 
        {
            if( dijA.arrival( dijA.heap.top().v ) < dijB.arrival( dijB.heap.top().v ) ) {
                current_layer = 0; // A
            }
            else { 
                current_layer = 1; // B
            }
        }
        else {
            current_layer = 2; // C
        }
                
        if(current_layer == 0)
        {
            curr_node = dijA.treat_next();
            if(rlcA.is_accepting(curr_node) && !states[current_layer][curr_node.first].set_and_accepting) {
                states[current_layer][curr_node.first].set_and_accepting = true;
                states[current_layer][curr_node.first].dfa_state = curr_node.second;
                states[current_layer][curr_node.first].arrival = dijA.arrival(curr_node);
                
                if( states[1][curr_node.first].set_and_accepting ) {
                    int max_arr;
                    if(states[0][curr_node.first].arrival > states[1][curr_node.first].arrival)
                        max_arr = states[0][curr_node.first].arrival;
                    else 
                        max_arr = states[1][curr_node.first].arrival;
                    
                    insert_node_in_C(curr_node.first, max_arr);
                }
            }
        }
        else if(current_layer == 1)
        {
            curr_node = dijB.treat_next();
            if(rlcB.is_accepting(curr_node) && !states[current_layer][curr_node.first].set_and_accepting) {
                states[current_layer][curr_node.first].set_and_accepting = true;
                states[current_layer][curr_node.first].dfa_state = curr_node.second;
                states[current_layer][curr_node.first].arrival = dijB.arrival(curr_node);
                
                if( states[0][curr_node.first].set_and_accepting ) {
                    int max_arr;
                    if(states[0][curr_node.first].arrival > states[1][curr_node.first].arrival)
                        max_arr = states[0][curr_node.first].arrival;
                    else 
                        max_arr = states[1][curr_node.first].arrival;
                    
                    insert_node_in_C(curr_node.first, max_arr);
                }
                
                
            }
        }
        else
        {
            curr_node = dijC.treat_next();
        }
    }
    cout << "Finished\n";
    BOOST_ASSERT(current_layer == 2);
    build_result(curr_node);
    
    return true;
}

void SharedPath::build_result(RLC::Vertice rlc_dest)
{
    BOOST_ASSERT(dijC.black(rlc_dest));
    BOOST_ASSERT(rlcC.is_accepting(rlc_dest));

    RLC::Vertice curr;
    vres.a_nodes.push_back(rlc_dest.first);
    
    for(curr = rlc_dest ; dijC.has_pred(curr) ; curr = rlcC.source(dijC.get_pred(curr)))
        vres.edges.push_back(g->edgeIndex( dijC.get_pred(curr).first ));
    
    vres.b_nodes.push_back(curr.first);
    
    int m = curr.first;
    curr.first = m;
    curr.second = states[0][m].dfa_state;
    
    for(; dijA.has_pred(curr) ; curr = rlcA.source(dijA.get_pred(curr))) 
        vres.edges.push_back(g->edgeIndex( dijA.get_pred(curr).first ));
    
    vres.c_nodes.push_back(curr.first);
    
    curr.first = m;
    curr.second = states[1][m].dfa_state;
    
    for(; dijB.has_pred(curr) ; curr = rlcB.source(dijB.get_pred(curr))) 
        vres.edges.push_back(g->edgeIndex( dijB.get_pred(curr).first ));
    
    vres.c_nodes.push_back(curr.first);
}

VisualResult SharedPath::get_result() const
{
    return vres;
}


void SharedPath::insert_node_in_C ( int node, int arr )
{
    BOOST_FOREACH(int dfa_start, rlcC.dfa_start_states()) 
    {
        RLC::Vertice rlc_node;
        rlc_node.first = node;
        rlc_node.second = dfa_start;
        
        if(dijC.white(rlc_node))
        {
    //         Dout(dc::notice, " -- Inserting rlc_node into heap : ("<< rlc_node.first<<", "<<rlc_node.second<<")");
            dijC.set_arrival(rlc_node, arr);
            dijC.put_dij_node(rlc_node);
            dijC.set_gray(rlc_node);
//             vres.a_nodes.push_back(node);
        }
        else if( arr < dijC.arrival(rlc_node) )
        {
    //         Dout(dc::notice, " -- Updating rlc_node in heap : ("<< rlc_node.first<<", "<<rlc_node.second<<") new: "<<rlc_node_arr
    //                             << " old: "<<arrival(rlc_node)                );
            BOOST_ASSERT(!dijC.black(rlc_node));
            
            dijC.set_arrival(rlc_node, arr);
            dijC.clear_pred(rlc_node);
            dijC.heap.update(dijC.handle(rlc_node));
//             vres.b_nodes.push_back(node);
        }
    }
    
}



