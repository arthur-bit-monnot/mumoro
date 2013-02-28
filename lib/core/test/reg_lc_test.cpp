#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE RegLC_Graph
#include <boost/test/unit_test.hpp>
 
#include "reglc_graph.h"
#include "path_algo.h"

const char* graph_dump = "/home/arthur/LAAS/mumoro/3d12fc983d92949462bdc2c3c6a65670.dump";

struct Graphs {
    std::list< RLC::Graph > graphs;
    Transport::Graph transport;
    
    Graphs() :
    transport(Transport::Graph(graph_dump))
    {
        std::list< RLC::DFA > dfas;
        dfas.push_back(RLC::foot_subway_dfa());
        dfas.push_back(RLC::all_dfa());
        
        BOOST_FOREACH(RLC::DFA dfa, dfas) {
            graphs.push_back(RLC::Graph(&transport, dfa));
        }
    }
};

 
BOOST_FIXTURE_TEST_SUITE(Dijkstra_on_RegLC, Graphs)
 
BOOST_AUTO_TEST_CASE(Validate_path_length)
{
    float start_sec = 50400.0f;
    int start_day = 233;
    int start_node = 527;
    int dest_node = 58;
    
    BOOST_FOREACH(RLC::Graph g, graphs)
    {
        RLC::Dijkstra dij(&g, start_node, dest_node, start_sec, start_day);
        if(dij.run()) { // path found
            
            /** Check that the path duration is good **/
            EdgeList path = dij.get_transport_path();
            float curr_time = start_sec;
            BOOST_FOREACH(int edge_id, path)
            {
                Edge e = g.transport->mapEdge(edge_id);
                std::pair<bool, int> duration  = e.duration(curr_time, start_day);
                BOOST_ASSERT(duration.first); // It must have traffic
                curr_time = duration.second;
            }
            
            BOOST_CHECK(curr_time == dij.path_arrival);
            
            
            
            /** Check that the path is the same that was found by regular dijkstra  **/
            if(num_vertices( g.dfa.graph ) == 1) // dfa with no constraints 
            {
                
                EdgeList path_a = dijkstra(start_node, dest_node, start_sec, start_day, *g.transport);
                
                EdgeList path_b = dij.get_transport_path();
                
                BOOST_CHECK(path_a.size() == path_b.size());
                
                EdgeList::iterator ita, itb;
                for(ita=path_a.begin(), itb=path_b.begin() ; ita != path_a.end() ; ++ita, ++itb)
                {
                    BOOST_CHECK(*ita == *itb);
                }
            }
        } 
        else 
        {
            BOOST_TEST_MESSAGE( "No path was found for "<<start_node<<" -> "<<dest_node );
        }
        
        
    }
 
}
 
BOOST_AUTO_TEST_SUITE_END()
