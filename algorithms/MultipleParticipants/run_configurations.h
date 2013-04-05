#ifndef MUPARO_RUN_CONFIGURATIONS
#define MUPARO_RUN_CONFIGURATIONS

#include "muparo.h"

namespace MuPaRo {

Muparo * point_to_point(Transport::Graph * trans, int source, int dest);
Muparo * bi_point_to_point(Transport::Graph * trans, int source, int dest);
Muparo * bidir_covoiturage(Transport::Graph * trans, int source1, int source2, int dest1, int dest2,
                     RLC::DFA dfa1, RLC::DFA dfa2, int limit = -1);
Muparo * time_dep_covoiturage(Transport::Graph * trans, int source1, int source2, int dest1, int dest2,
                              RLC::DFA dfa_pass, RLC::DFA dfa_car, int limit = -1);

Muparo * conv_time_dep_covoiturage(Transport::Graph * trans, int source1, int source2, int dest1, int dest2,
                              RLC::DFA dfa_pass, RLC::DFA dfa_car);

Muparo * covoiturage(Transport::Graph * trans, int source1, int source2, int dest1, int dest2,
                              RLC::DFA dfa_pass, RLC::DFA dfa_car );


/**
 * Those functions allow to add further restrictions on a Muparo instance for car-sharing
 */
void add_rectangle_restriction_on_car(Muparo * mup, Transport::Graph* trans, int source, int dest, float margin);
void add_isochrone_restriction_on_passenger(Muparo * mup, Transport::Graph* trans, int source, int dest, float max_time);
}


#endif
