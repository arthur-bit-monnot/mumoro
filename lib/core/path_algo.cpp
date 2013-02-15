#include <boost/graph/adjacency_list.hpp>
#include <boost/tuple/tuple.hpp>

#include "path_algo.h"



// using namespace boost::multi_index;
using namespace boost;
// using namespace std;

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
    */
    EdgeList l = g.listEdges(CarEdge);
    EdgeList::iterator it;
    for(it = l.begin(); it != l.end() ; it++) {
        std::cout << edgeType2str(g.mapEdge(*it).type) << " ";
    }
    
    //std::cout << "\nTraversed "<<count<<" edges\n";
    
    return 0;
}
