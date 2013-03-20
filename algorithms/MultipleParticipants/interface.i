%module "mumoro::muparo"

%{
 #include "muparo.h"
 #include "run_configurations.h"
%}

// Parse the original header file
%include "muparo.h"
%include "run_configurations.h"