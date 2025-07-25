#include <perf/arch.h>
#include <utils/types.h>

uint64_t x86_get_rdtsc_counter(void) {
    uint32_t lo, hi;
    __asm__ __volatile__ (
        "rdtsc\n"
        : "=a"(lo), "=d"(hi)
    );
    return ((uint64_t)hi << 32) | lo;
}

uint64_t x86_get_rdtscp_counter(void) {
    uint32_t lo, hi;
    __asm__ __volatile__ (
        "rdtscp\n"
        : "=a"(lo), "=d"(hi)
        : /* no inputs */
        : "rcx"  /* rdtscp clobbers RCX on x86_64 */
    );
    return ((uint64_t)hi << 32) | lo;
}

uint64_t x86_get_rdtsc_counter_serialized(void) {
    uint32_t lo, hi;
    __asm__ __volatile__ (
        "cpuid\n\t"
        "rdtsc\n"
        : "=a"(lo), "=d"(hi)
        : "a"(0)          /* cpuid leaf 0 */
        : "rbx", "rcx"    /* cpuid clobbers RBX, RCX */
    );
    return ((uint64_t)hi << 32) | lo;
}

#define x86_64_get_rdtsc_counter            x86_get_rdtsc_counter
#define x86_64_get_rdtscp_counter           x86_get_rdtscp_counter
#define x86_64_get_rdtsc_counter_serialized x86_get_rdtsc_counter_serialized

uint64_t get_cycle_count(void) {
#if defined(ARCH_X86) || defined(ARCH_X86_64)
    return x86_get_rdtsc_counter();
#else
# error "Unsupported architecture"
#endif
}

uint64_t get_cycle_count_enhanced(void) {
#if defined(ARCH_X86) || defined(ARCH_X86_64)
    return x86_get_rdtscp_counter();
#else
# error "Unsupported architecture"
#endif
}

uint64_t get_cycle_count_serialized(void) {
#if defined(ARCH_X86) || defined(ARCH_X86_64)
    return x86_get_rdtsc_counter_serialized();
#else
# error "Unsupported architecture"
#endif
}

