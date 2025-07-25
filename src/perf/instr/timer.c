#define _POSIX_C_SOURCE 200809L
#include <perf/timer.h>

uint64_t get_nanoseconds(void)
{ 
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
  return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}
