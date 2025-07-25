#include <perf/instr.h>
#include <utils/log.h>
#include <stdlib.h>
#include <string.h>

// Define typical cache sizes (these are illustrative and may vary by CPU)
#define L1_CACHE_SIZE   (32 * 1024)    // 32 KB
#define L2_CACHE_SIZE   (256 * 1024)   // 256 KB
#define L3_CACHE_SIZE   (8 * 1024 * 1024) // 8 MB

// A buffer size larger than L1, but fitting in L2
#define L2_TEST_BUFFER_SIZE (L1_CACHE_SIZE * 4) // 128 KB, should hit L2

// A buffer size larger than L2, but fitting in L3
#define L3_TEST_BUFFER_SIZE (L2_CACHE_SIZE * 4) // 1 MB, should hit L3

// A buffer size larger than L3, should spill into DRAM
#define DRAM_TEST_BUFFER_SIZE (L3_CACHE_SIZE * 2) // 16 MB, should hit DRAM

void test_l2_cache(void)
{
    PROFILE_FUNCTION_START;
    log_println("\nTesting L2");

    uint8_t *l2_buf = malloc(L2_TEST_BUFFER_SIZE);
    if (!l2_buf) {
        log_println("Failed to allocate L2 test buffer!");
        PROFILE_FUNCTION_END;
        return;
    }
    memset(l2_buf, 0xBB, L2_TEST_BUFFER_SIZE);

    const uint64_t iterations = 1000 * 25; // Fewer iterations for larger buffers

    for (uint64_t i = 0; i < iterations; ++i)
    {
        PROFILE_CACHE_START(l2_access_test);
        for (size_t j = 0; j < L2_TEST_BUFFER_SIZE; j += 64)
        { // Stride by cache line size (64 bytes typical)
            l2_buf[j]++;
        }
        PROFILE_CACHE_END(l2_access_test);
    }

    free(l2_buf);
    PROFILE_HINT_SUCCESSFUL_RETURN;
    PROFILE_FUNCTION_END;
}

void test_l3_cache(void)
{
    PROFILE_FUNCTION_START;
    log_println("\nTesting L3");

    uint8_t *l3_buf = malloc(L3_TEST_BUFFER_SIZE);
    if (!l3_buf) {
        log_println("Failed to allocate L3 test buffer!");
        PROFILE_FUNCTION_END;
        return;
    }
    memset(l3_buf, 0xCC, L3_TEST_BUFFER_SIZE);

    const uint64_t iterations = 100 * 25; 

    for (uint64_t i = 0; i < iterations; ++i)
    {
        PROFILE_CACHE_START(l3_access_test);
        for (size_t j = 0; j < L3_TEST_BUFFER_SIZE; j += 64) {
            l3_buf[j]++;
        }
        PROFILE_CACHE_END(l3_access_test);
    }

    free(l3_buf);
    PROFILE_HINT_SUCCESSFUL_RETURN;
    PROFILE_FUNCTION_END;
}

void test_dram_access(void)
{
    PROFILE_FUNCTION_START;
    log_println("\nTesting DRAM");

    uint8_t *dram_buf = malloc(DRAM_TEST_BUFFER_SIZE);
    if (!dram_buf) {
        log_println("Failed to allocate DRAM test buffer!");
        PROFILE_FUNCTION_END;
        return;
    }
    memset(dram_buf, 0xDD, DRAM_TEST_BUFFER_SIZE);

    const uint64_t iterations = 10 * 25; // Very few iterations as DRAM access is slow

    for (uint64_t i = 0; i < iterations; ++i) {
        PROFILE_CACHE_START(dram_access_test);
        for (size_t j = 0; j < DRAM_TEST_BUFFER_SIZE; j += 64) {
            dram_buf[j]++;
        }
        PROFILE_CACHE_END(dram_access_test);
    }

    free(dram_buf);
    PROFILE_HINT_SUCCESSFUL_RETURN;
    PROFILE_FUNCTION_END;
}


int main(void)
{
    profiler_init();
    
    cache_latency_profile_t* cache_profile = profiler_get_cache_profile();
    if (cache_profile) {
        log_println("Using calibrated cache latencies:");
        log_println("L1 latencies {}", cache_profile->l1_latency_cycles);
        log_println("L2 latencies {}", cache_profile->l2_latency_cycles);
        log_println("L3 latencies {}", cache_profile->l3_latency_cycles);
        log_println("Dram latencies {}", cache_profile->dram_latency_cycles);
    } else {
        log_println("Cache latency profiling not available.");
    }
    
    PROFILE_FUNCTION_START;
    log_println("\nTesting L1");
    uint8_t *l1_buf = malloc(L1_CACHE_SIZE);
    if (!l1_buf)
    {
        return -1;
    }
    memset(l1_buf, 0xAA, L1_CACHE_SIZE);  
    const uint64_t l1_iterations = 250 * 4 * 1000;
    for (uint64_t i = 0; i < l1_iterations; ++i)
    {
        PROFILE_CACHE_START(l1_access_main);
        for (size_t j = 0; j < L1_CACHE_SIZE; ++j)
        {
            l1_buf[j]++;
        }
        PROFILE_CACHE_END(l1_access_main);
    }
    free(l1_buf);
    PROFILE_HINT_SUCCESSFUL_RETURN;
    PROFILE_FUNCTION_END;

    test_l2_cache();
    test_l3_cache();
    test_dram_access();

    profiler_end();
    profiler_print_all();
    return 0;
}
