#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE RegLC_Graph
#include <boost/test/unit_test.hpp>
 
#include "reglc_graph.h"
#include "path_algo.h"

const char* graph_dump = "/home/arthur/LAAS/mumoro/3d12fc983d92949462bdc2c3c6a65670.dump";

struct Graphs {
    std::list< RLC::RegLCGraph > graphs;
    Graph transport;
    
    Graphs() :
    transport(Graph(graph_dump))
    {
        std::list< RLC::DFA > dfas;
        dfas.push_back(RLC::foot_subway_dfa());
        dfas.push_back(RLC::all_dfa());
        
        BOOST_FOREACH(RLC::DFA dfa, dfas) {
            graphs.push_back(RLC::RegLCGraph(transport, dfa));
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
    
    BOOST_FOREACH(RLC::RegLCGraph g, graphs)
    {
        RLC::Dijkstra dij(&g, start_node, dest_node, start_sec, start_day);
        if(dij.run()) { // path found
            
            /** Check that the path duration is good **/
            EdgeList path = dij.get_transport_path();
            float curr_time = start_sec;
            BOOST_FOREACH(int edge_id, path)
            {
                Edge e = g.transport.mapEdge(edge_id);
                curr_time = e.duration(curr_time, start_day);
            }
            
            BOOST_CHECK(curr_time == dij.path_arrival);
            
            
            
            /** Check that the path is the same that was found by regular dijkstra  **/
            if(num_vertices( g.dfa.graph ) == 1) // dfa with no constraints 
            {
                
                EdgeList path_a = dijkstra(start_node, dest_node, start_sec, start_day, g.transport);
                
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
