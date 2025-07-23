#pragma once

#include <utils/types.h>
#include <utils/macros.h>

EXTERN_C_START

uint64_t x86_get_rdtsc_counter(void);
uint64_t x86_get_rdtscp_counter(void);

EXTERN_C_END
