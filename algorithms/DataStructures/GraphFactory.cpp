#include "GraphFactory.h"

#include "Landmark.h"




namespace Transport {
    
    
GraphFactory::GraphFactory ( const int nb_nodes )
{
    initialized = false;
    this->graph = new Graph( nb_nodes );
}

GraphFactory::GraphFactory ( const std::string& filename, bool from_bin )
{
    initialized = true;
    this->graph = new Graph( filename, from_bin );
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

void GraphFactory::save_to_bin ( const std::string& filename ) const
{
    graph->save_to_bin( filename );
}

void GraphFactory::save_to_txt ( const std::string& filename ) const
{
    graph->save_to_txt( filename );
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