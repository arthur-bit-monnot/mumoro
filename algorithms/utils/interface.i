%module "mumoro::utils"


%{
 #include "node_filter_utils.h"
%}

// Parse the original header file
%include "node_filter_utils.h"