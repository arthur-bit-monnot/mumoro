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

#include "GraphFactory.h"

#include "Landmark.h"




namespace Transport {
    
    
GraphFactory::GraphFactory ( const int nb_nodes )
{
    initialized = false;
    this->graph = new Graph( nb_nodes );
}

GraphFactory::GraphFactory ( const std::string& filename )
{
    initialized = true;
    this->graph = new Graph( filename );
}

void GraphFactory::add_road_edge ( const int source, const int target, const EdgeMode type, const int duration )
{
    initialized = false;
    graph->add_road_edge( source, target, type, duration );
}

bool GraphFactory::add_public_transport_edge ( const int source, const int target, const DurationType dur_type, const float start, 
                                               const float arrival, const int duration, const std::string& services, 
                                               const EdgeMode type )
{
    initialized = false;
    return graph->add_public_transport_edge( source, target, dur_type, start, arrival, duration, services, type);
}

bool GraphFactory::add_public_transport_edge ( const int source, const int target, const int duration, const EdgeMode type )
{
    initialized = false;
    return graph->add_public_transport_edge( source, target, duration, type );
}

void GraphFactory::set_coord ( const int node, const float lon, const float lat )
{
    graph->set_coord( node, lon, lat );
}

void GraphFactory::set_id ( const std::string id )
{
    graph->set_id( id );
}



const Graph * GraphFactory::get()
{
    
    if( !initialized )
        init();
    initialized = true;
    return graph;
}

void GraphFactory::save ( const std::string& filename ) const
{
    graph->save( filename );
}

void GraphFactory::init()
{
    preproc_car_layer();
    graph->preprocess();
}

/**
 * Removes all edges of type 'type' incoming in 'node'
 * 
 * This function is recursive since modification of the edge list invalidates any iterator on it
 */
void remove_in_edges( Graph_t & g, const int node, const EdgeMode type ) {
    Graph_t::in_edge_iterator ei, end;
    boost::tie(ei, end) = boost::in_edges(node, g);
    for( ; ei != end ; ++ei ) {   
        if(g[*ei].type == type) {
            boost::remove_edge(*ei, g);
            return remove_in_edges(g, node, type);
        }
    }
}

/**
 * Removes all edges of type 'type' going out of 'node'
 * 
 * This function is recursive since modification of the edge list invalidates any iterator on it
 */
void remove_out_edges( Graph_t & g, const int node, const EdgeMode type ) {
    Graph_t::out_edge_iterator ei, end;
    boost::tie(ei, end) = boost::out_edges(node, g);
    for( ; ei != end ; ++ei ) {   
        if(g[*ei].type == type) {
            boost::remove_edge(*ei, g);
            return remove_out_edges(g, node, type);
        }
    }
}

void GraphFactory::preproc_car_layer()
{
    int node, count;
    RLC::Landmark * lm = NULL;
    
    graph->car_accessibility.resize( graph->num_vertices() );
    
    // Selects a landmark from which an important part of the graph is accessible
    do {
        if( lm != NULL ) delete lm;
        
        node = rand() % (graph->num_vertices() / 2);
        lm = RLC::create_car_landmark(graph, node);
        
        count = 0;
        for(int i=0; i<graph->num_vertices() ; ++i) 
            if( lm->forward_reachable(i) )
                count++;
    } while( count < graph->num_vertices()/3 );
    
    int cnt = 0;
    for(int i=0; i< graph->num_vertices(); ++i) {
        if((lm->backward_reachable(i) && !lm->forward_reachable(i)) || (!lm->backward_reachable(i) && lm->forward_reachable(i))) {
            cnt++;
            remove_in_edges(graph->g, i, CarEdge);
            remove_out_edges(graph->g, i, CarEdge);
        } else if( lm->forward_reachable( i ) ) {
            graph->set_car_accessible( i );
        }
    }
    std::cout << cnt << " car dead-ends spoted and removed." << std::endl;
    
    delete lm;
}


    
    
}