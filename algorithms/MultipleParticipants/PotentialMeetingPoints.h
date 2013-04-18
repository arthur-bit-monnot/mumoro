#ifndef POTENTIAL_MEETING_POINTS_H
#define POTENTIAL_MEETING_POINTS_H

#include "MuparoTypedefs.h"

class Area;



VisualResult show_meeting_points( const Transport::Graph * g, int source, int time = 50000 );

NodeSet * meeting_points( const Transport::Graph * g, int source, Area * area );




#endif