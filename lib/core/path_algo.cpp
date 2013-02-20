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


EdgeList dijkstra(int start, int destination, float dep_sec, int dep_day, Graph g) {
    
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
            try {
            if(e.type == FootEdge || e.type == TransferEdge || e.type == SubwayEdge) 
            {
                cout << "("<<e.edge_index;
                int target = boost::target(*ei, g.g);
                
                float dist = e.duration(distance[np.index], dep_day);
                cout << ", "<<target<<", "<<dist<<", "<<") ";
                
                if(dist < distance[target])
                {
                    distance[target] = dist;
                    predecessor[target] = *ei;
                    heap.update(references[target]);
                }
            }
            } catch(No_traffic) {}
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


/*
int main() {
    
    Graph g("/home/arthur/LAAS/mumoro/3d12fc983d92949462bdc2c3c6a65670.dump");

    EdgeList edges = dijkstra(174, 92, 0, 0, g);
    EdgeList::iterator it;
    for(it = edges.begin(); it != edges.end() ; it++) {
        std::cout << "(" << g.sourceNode(*it) <<" "<<*it<<" "<<g.targetNode(*it) <<") ";
    }
    
    RLC_test(g);
    RegLCGraph rlc(g, default_dfa());
    
    RLCVertice orig(13763823, 2);
    uint conv = rlc.toInt(orig);
    RLCVertice fin = rlc.toVertice(conv);
    
    cerr << orig.first <<" "<< fin.first <<" "<<orig.second <<" "<< fin.second <<" \n";
    BOOST_ASSERT(orig.first == fin.first);
    BOOST_ASSERT(orig.second == fin.second);

  
    return 0;
}*/
