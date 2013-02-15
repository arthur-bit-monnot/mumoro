#include <boost/graph/adjacency_list.hpp>
#include <boost/heap/fibonacci_heap.hpp>
#include <boost/tuple/tuple.hpp>

#include "path_algo.h"


typedef boost::heap::fibonacci_heap<node_ptr, boost::heap::compare<Compare> > Heap;


// using namespace boost::multi_index;
using namespace boost;
using namespace std;

const char* edgeType2str(EdgeType type) {
    if(type == FootEdge)
        return "Foot";//"Foot";
    else if(type == BikeEdge)
        return "Bike";
    else if(type == CarEdge)
        return "Car";//;"Car";
    else if(type == BusEdge)
        return "Bus";
    else if(type == TramEdge)
        return "Tram";
    else if(type == SubwayEdge)
        return "Subway";
    else if(type == TransferEdge)
        return "--Transfer--";
    else
        return "------ Unknown -------";
}

void dijkstra(int start, int destination, Graph g) {
    
    Graph_t::edge_iterator tei, tend;
    tie(tei,tend) = edges(g.g);
    
    
    vector<float> distance(num_vertices(g.g), 999999999999999.0);
    vector<edge_t> predecessor(num_vertices(g.g), *tei);
    std::vector<Heap::handle_type> references;
    
    Compare comp(distance);
    
    Heap heap(comp);
    
    for(int i=0 ; i<750 ; ++i)
        references.push_back(heap.push(node_ptr(i)));
    
    //references[start]->time = 0.0f;
    
    
    distance[start] = 0.0f;
    predecessor[start] = *tei;
    heap.update(references[start]); 
    
    while (!heap.empty())
    {
        int dist;

        node_ptr np = heap.top();
        heap.pop();
        
        std::cout << np.index << " "<<distance[start]<<" "<<distance[np.index]<<"\n";

        if (np.index == destination)
            break;

        Graph_t::out_edge_iterator ei, end;
        tie(ei,end) = out_edges(np.index, g.g);
        for(; ei != end; ei++) {
            Edge e = g.g[*ei];
            if(e.type == FootEdge) {
            
                int target = boost::target(*ei, g.g);
                float dist = np.time + e.duration(np.time, 0);
    //             if(e.type == FootEdge)
                    cout << "("<<target<<", "<<dist<<", "<<e.duration(np.time, 0)<<") ";
                if(dist < distance[target])// && e.type == FootEdge) 
                {
                    distance[target] = dist;
                    predecessor[target] = *ei;
                    heap.update(references[target]);//, dist);
                }
            }
        }
        cout << "\n";
    }
    /*
    for(int i = destination; i != start; i = source(predecessor[i], g.g))
            std::cout << i << " - ";
        std::cout << "\n";
    */
}

int main() {
    
    Graph g("/home/arthur/LAAS/mumoro/3d12fc983d92949462bdc2c3c6a65670.dump");
    /*
    std::cout << "Loaded " << num_vertices(g.g);
    int count = 0;
    Graph_t::vertex_iterator vi, vi_end, next;
    tie(vi, vi_end) = vertices(g.g);
    for (next = vi; vi != vi_end; vi = next) {
        Graph_t::out_edge_iterator ei, end;
        tie(ei,end) = out_edges(*vi, g.g);
        for(; ei != end; ei++) {
            std::cout << edgeType2str(g.g[*ei].type) << " ";
            count++;
        }
        std::cout << std::endl;
        ++next;
    }
   
    EdgeList l = g.listEdges(CarEdge);
    EdgeList::iterator it;
    for(it = l.begin(); it != l.end() ; it++) {
        std::cout << edgeType2str(g.mapEdge(*it).type) << " ";
    }
    */
    //std::cout << "\nTraversed "<<count<<" edges\n";
    
    dijkstra(234, 238, g);
    
    return 0;
}
