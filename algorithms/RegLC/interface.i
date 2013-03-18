%module "mumoro::reglc"


%{
 #include "reglc_graph.h"
%}

%rename(RLC_Compare) RLC::Compare;
%rename(RLC_Vertice) RLC::Vertice;
%rename(RLC_Edge)    RLC::Edge;
%rename(RLC_Graph)   RLC::Graph;

// Parse the original header file
%include "reglc_graph.h"