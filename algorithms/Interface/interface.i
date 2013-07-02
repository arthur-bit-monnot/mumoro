%module "mumoro::interface"

%{
#include "ItinerariesRequests.h"
#include "Path.h"
%}

// Parse the original header file
%include "ItinerariesRequests.h"
%include "Path.h"
