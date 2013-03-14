#include "debug/cwd_sys.h"

#include <boost/graph/adjacency_list.hpp>
#include <boost/heap/fibonacci_heap.hpp>
#include <boost/tuple/tuple.hpp>
#include <time.h>


#include "path_algo.h"
//#include "reglc_graph.h"



typedef boost::heap::fibonacci_heap<node_ptr, boost::heap::compare<Compare> > Heap;

using namespace boost;
using namespace std;

//FIXME this is algorithm wasn't updated after modifyng costs definition
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



