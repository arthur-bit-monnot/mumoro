%module "mumoro::core"


%{
 #include "graph_wrapper.h"
 #include "nodes_filter.h"
%}
  
%template(EdgeList) std::list<int>;

// Parse the original header file
%include "graph_wrapper.h" 
%include "nodes_filter.h"