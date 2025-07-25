#pragma once

#include <utils/macros.h>
#include <utils/types.h>

typedef uint32_t  _cpu_cores;
typedef uint32_t  _cpu_threads;

typedef uint64_t  _mem_size;
typedef uint8_t   _percentage;

typedef struct ALIGNAS(32) Hardware_Specifications
{
  _cpu_cores   cpu_cores;
  _cpu_threads cpu_threads;

  _mem_size    total_memory;
  _mem_size    free_memory;
  _mem_size    used_memory;

  _percentage  memory_usage_percentage;
} Hardware_Specifications;

_cpu_cores    impl_hw_get_cpu_cores(void);
_cpu_threads  impl_hw_get_cpu_threads(void);

_mem_size      impl_hw_get_total_memory(void);
_mem_size      impl_hw_get_free_memory(void);
_mem_size      impl_hw_get_used_memory(void);

// shared helpers:
// calculate and update percentage inside the struct
_percentage    shared_calc_mem_usage_s(Hardware_Specifications *hw_specs);

// calculate by given total/free memory (pure calculation)
_percentage    shared_calc_mem_usage_v(_mem_size total,_mem_size free);

// getter to the static struct
Hardware_Specifications *shared_get_hw_specs(void);
