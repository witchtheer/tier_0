#include <perf/arch.h>

#include <stdio.h>

uint64_t get_cycle_count(void)
{
  #if defined(ARCH_X86)
  return x86_get_rdtsc_counter();
  #elif defined(ARCH_X86_64)
  return x86_64_get_rdtsc_counter();
  #endif
}

uint64_t get_cycle_count_enhanced(void)
{
  #if defined(ARCH_X86)
  return x86_get_rdtscp_counter();
  #elif defined(ARCH_X86_64)
  return x86_64_get_rdtscp_counter();
  #endif
}
