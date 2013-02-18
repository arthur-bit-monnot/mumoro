 %module mumoro
 %include "std_string.i"
 %include "std_vector.i"
 %include "std_list.i"
 %include "std_set.i"

 %{
 #include "martins.h"
 #include "graph_wrapper.h"
 #include "path_algo.h"
 #include "reglc_graph.h"
  %}

 %template(Paths) std::vector<Path>;
 %template(Costs) std::vector<float>;
 %template(Nodes) std::list<int>;



 // Parse the original header file
 %include "martins.h"
 %include "graph_wrapper.h" 
 %include "path_algo.h"
 %include "reglc_graph.h"