#ifndef MUPARO_RUN_CONFIGURATIONS
#define MUPARO_RUN_CONFIGURATIONS

#include "muparo.h"

namespace MuPaRo {

Muparo * point_to_point(Transport::Graph * trans, int source, int dest);
Muparo * bi_point_to_point(Transport::Graph * trans, int source, int dest);
Muparo * covoiturage(Transport::Graph * trans, int source1, int source2, int dest1, int dest2,
                     RLC::DFA dfa1, RLC::DFA dfa2, int limit = -1);
Muparo * time_dep_covoiturage(Transport::Graph * trans, int source1, int source2, int dest1, int dest2,
                              RLC::DFA dfa_pass, RLC::DFA dfa_car, int limit = -1);

Muparo * conv_time_dep_covoiturage(Transport::Graph * trans, int source1, int source2, int dest1, int dest2,
                              RLC::DFA dfa_pass, RLC::DFA dfa_car);
}


#endif
