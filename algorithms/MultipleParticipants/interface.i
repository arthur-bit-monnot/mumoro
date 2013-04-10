%module "mumoro::muparo"

%{
 #include "muparo.h"
 #include "MPR_AspectTarget.h"
 #include "run_configurations.h"
 #include "PotentialMeetingPoints.h"
%}

// Parse the original header file
%include "muparo.h"
%include "MPR_AspectTarget.h"
%include "run_configurations.h"
%include "PotentialMeetingPoints.h"