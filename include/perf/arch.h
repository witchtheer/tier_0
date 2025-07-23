#pragma once

#include <utils/macros.h>
#include <utils/types.h>

#if defined(ARCH_X86)
#include <perf/arch/x86/_counters.h>
#elif ARCH_X86_64
#include <perf/arch/x86_64/_counters.h>
#endif

uint64_t get_cycle_count(void);
uint64_t get_cycle_count_enhanced(void);
