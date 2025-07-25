#include <perf/instr.h>
#include <utils/log.h>
#include <stdlib.h>
#include <string.h>

#define L1_SIZE 32 * 1024 

int main(void)
{
    profiler_init();
    
    cache_latency_profile_t* cache_profile = profiler_get_cache_profile();
    log_println("Using calibrated cache latencies:");
    log_println("L1 latencies {}",cache_profile->l1_latency_cycles);
    log_println("L2 latencies {}",cache_profile->l2_latency_cycles);
    log_println("L3 latencies {}",cache_profile->l3_latency_cycles);
    log_println("Dram latencies {}",cache_profile->dram_latency_cycles);
    
    PROFILE_FUNCTION_START;

    uint8_t *l1_buf = malloc(L1_SIZE);
    if (!l1_buf) {
        return -1;
    }

    memset(l1_buf, 0xAA, L1_SIZE);  

    const uint64_t iterations = 250 * 4 * 100;

    for (uint64_t i = 0; i < iterations; ++i) 
    {
        PROFILE_CACHE_START(l1_access);

        // This will keep the entire L1 buffer hot repeatedly
        for (size_t j = 0; j < L1_SIZE; ++j)
        {
            l1_buf[j]++;
        }

        PROFILE_CACHE_END(l1_access);
    }

    free(l1_buf);

    PROFILE_FUNCTION_END;
    PROFILE_HINT_SUCCESSFUL_RETURN;

    profiler_end();
    profiler_print_all();

    return 0;
}
