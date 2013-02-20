 %module mumoro
 %include "std_string.i"
 %include "std_vector.i"
 %include "std_list.i"
 %include "std_set.i"

 %{
 #include "graph_wrapper.h"
 #include "path_algo.h"
 #include "reglc_graph.h"
  %}
  
  %template(EdgeList) std::list<int>;

 %rename(RLC_Compare) RLC::Compare;

 // Parse the original header file
 %include "graph_wrapper.h" 
 %include "path_algo.h"
 %include "reglc_graph.h"