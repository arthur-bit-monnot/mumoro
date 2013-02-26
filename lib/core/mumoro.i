 %module mumoro
 %include "std_string.i"
 %include "std_vector.i"
 %include "std_list.i"
 %include "std_set.i"
%include pointer.i

 %{
 #include "graph_wrapper.h"
 #include "path_algo.h"
 #include "reglc_graph.h"
  %}
  
  %template(EdgeList) std::list<int>;

 %rename(RLC_Compare) RLC::Compare;
 %rename(RLC_Vertice) RLC::Vertice;
 %rename(RLC_Edge)    RLC::Edge;
 %rename(RLC_Graph)   RLC::Graph;

 // Parse the original header file
 %include "graph_wrapper.h" 
 %include "path_algo.h"
 %include "reglc_graph.h"