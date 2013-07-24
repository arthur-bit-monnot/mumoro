/** Copyright : Arthur Bit-Monnot (2013)  arthur.bit-monnot@laas.fr

This software is a computer program whose purpose is to [describe
functionalities and technical features of your software].

This software is governed by the CeCILL-B license under French law and
abiding by the rules of distribution of free software.  You can  use, 
modify and/ or redistribute the software under the terms of the CeCILL-B
license as circulated by CEA, CNRS and INRIA at the following URL
"http://www.cecill.info". 

As a counterpart to the access to the source code and  rights to copy,
modify and redistribute granted by the license, users are provided only
with a limited warranty  and the software's author,  the holder of the
economic rights,  and the successive licensors  have only  limited
liability. 

In this respect, the user's attention is drawn to the risks associated
with loading,  using,  modifying and/or developing or reproducing the
software by the user in light of its specific status of free software,
that may mean  that it is complicated to manipulate,  and  that  also
therefore means  that it is reserved for developers  and  experienced
professionals having in-depth computer knowledge. Users are therefore
encouraged to load and test the software's suitability as regards their
requirements in conditions enabling the security of their systems and/or 
data to be ensured and,  more generally, to use and operate it in the 
same conditions as regards security. 

The fact that you are presently reading this means that you have had
knowledge of the CeCILL-B license and that you accept its terms. 
*/

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