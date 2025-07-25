# tier_0 - Instrumenting Profiling Library

A lightweight C library for low-level performance analysis and profiling.

## Features

- Precise Timings: Hardware cycle counting using RDTSC/RDTSCP instructions (also serialized equivalent using CPUID)
- Cache Analysis: Runtime profiling for L1/L2/L3/DRAM access patterns (Linux support, rough estimation)
- Function Profiling: Automatic timing and call statistics

Note: Cache analysis is currently WIP with partial Linux support.

## Building

Requires premake5 and a C99 compiler.

```bash
# Generate build files
premake5 ninja    # for Ninja
premake5 gmake2   # for GNU Make  
premake5 vs2022   # for Visual Studio 2022

# Build
ninja             # if using Ninja
make              # if using Make
```

## Usage

```c
#include <tier_0.h>

int my_function() {
    PROFILE_FUNCTION_START;
    
    // Your code here
    
    PROFILE_FUNCTION_END;
    PROFILE_HINT_SUCCESSFUL_RETURN;
    return 0;
}

int main() {
    profiler_init();
    
    my_function();
    
    profiler_end();
    profiler_print_all();
    return 0;
}
```

## Cache Profiling (Linux only)

```c
PROFILE_CACHE_START(memory_op);
// Memory intensive code
PROFILE_CACHE_END(memory_op);
```

## Structure

- `perf/` - Core profiling functionality
- `platform/` - Platform specific code
- `utils/` - Utilities and logging

## Why? 
- this started as a module in my Game Engine and this module 
  got bigger than the Engine i was developing so i made it a separate library (and rewritten it :) )
