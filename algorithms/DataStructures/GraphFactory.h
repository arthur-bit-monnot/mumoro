#ifndef GRAPH_FACTORY_H
#define GRAPH_FACTORY_H

#include "graph_wrapper.h"

namespace Transport {

    
class GraphFactory {
    
    Graph * graph = NULL;
    bool initialized = false;

public:
    /**
     * Initializes the graph with a given number of nodes
     */
    GraphFactory(const int nb_nodes);
    
    /**
     * Loads the graph from an archive
     */
    GraphFactory(const std::string & filename, bool from_bin);
    
    /**
     * Insert an edge from *source* to *target* in the graph
     */
    void add_road_edge(const int source, const int target, const EdgeMode type, const int duration);
    
    /**
     * Adds public transport information to the given edge
     */
    bool add_public_transport_edge(const int source,const  int target, const DurationType dur_type, const float start, const float arrival, 
                                      const int duration, const std::string & services, const EdgeMode type);
    bool add_public_transport_edge(const int source,const  int target, const int duration, const EdgeMode type);
    
    /**
     * Sets longitude and latitude to the node
     */
    void set_coord(const int node, const float lon, const float lat);
    
    void set_id(const std::string id);

    const Graph * get();
    
    void save_to_bin(const std::string & filename) const;
    void save_to_txt(const std::string & filename) const;
    
private:
    void init();
    
    /**
     * This method is used to remove edges that make some nodes sinks from where it is impossible to go back.
     * There is typically car-labeled incoming edges but no car-labeled outgoing edges. It is mainly a dataset
     * problem in OSM.
     * 
     * This method therefore spot the dead-end nodes and removes any outgoing/incoming car-labeled edges from them.
     * 
     * It also stores in the graph information about which nodes are accessible by car
     */
    void preproc_car_layer();
};


}



#endif