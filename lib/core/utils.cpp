#include <unistd.h>
#include <stdlib.h>
#include <sys/times.h>
#include <ctime>

#include "utils.h"

double get_run_time_sec() {

  struct tms usage;
  static int clock_ticks = sysconf(_SC_CLK_TCK);
  times(&usage);
  double df=((double)usage.tms_utime+(double)usage.tms_stime)/clock_ticks;

  return df;
}
