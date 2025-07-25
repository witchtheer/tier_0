#if defined(__linux__)
#define _GNU_SOURCE
#endif

#include <perf/instr.h>
#include <utils/log.h>

#include <stdlib.h>
#include <string.h>

#ifdef __linux__
#include <linux/perf_event.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#endif

static prof_stat_head_t     g_prof_stat_head[TOTAL_FUNCTIONS];
static prof_cpu_stat_t      g_prof_cpu_stat[TOTAL_FUNCTIONS];
static prof_time_stat_t     g_prof_time_stat[TOTAL_FUNCTIONS];
static prof_call_stat_t     g_prof_call_stat[TOTAL_FUNCTIONS];
static prof_cache_stat_t    g_prof_cache_stat[TOTAL_FUNCTIONS];
static _in_use              g_prof_in_use[TOTAL_FUNCTIONS];

static _index               g_current_free_index = 0;
static cache_latency_profile_t g_cache_profile = {0};

#ifdef __linux__
// Hardware performance counter context
typedef struct 
{
    int l1_miss_fd;
    int l1_access_fd;
    int l2_miss_fd;
    int l2_access_fd; 
    int l3_miss_fd;
    int l3_access_fd; 
    int llc_miss_fd; 
    int cycles_fd;
    int instructions_fd;
    bool hw_counters_available;
} hw_perf_context_t;


static hw_perf_context_t g_hw_perf = {0};

// Linux perf_event_open syscall wrapper
static long perf_event_open(struct perf_event_attr *hw_event, pid_t pid,
                           int cpu, int group_fd, unsigned long flags) {
    return syscall(__NR_perf_event_open, hw_event, pid, cpu, group_fd, flags);
}

static bool init_hardware_performance_counters(void)
{
    struct perf_event_attr pe = {0};
    
    pe.type = PERF_TYPE_HW_CACHE;
    pe.size = sizeof(struct perf_event_attr);
    pe.disabled = 1;
    pe.exclude_kernel = 1;
    pe.exclude_hv = 1;
    pe.exclude_idle = 1;

    // L1 data cache load misses
    pe.config = (PERF_COUNT_HW_CACHE_L1D) |
                (PERF_COUNT_HW_CACHE_OP_READ << 8) |
                (PERF_COUNT_HW_CACHE_RESULT_MISS << 16);
    
    g_hw_perf.l1_miss_fd = perf_event_open(&pe, 0, -1, -1, 0);
    if (g_hw_perf.l1_miss_fd == -1)
    {
        log_println("Failed to open L1 data cache miss counter, falling back to estimation");
        // Don't return false here immediately if other counters might be useful
    }

    // L1 data cache load accesses (NEW COUNTER)
    // This will count all L1D accesses from reads
    pe.config = (PERF_COUNT_HW_CACHE_L1D) |
                (PERF_COUNT_HW_CACHE_OP_READ << 8) |
                (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16); // Use RESULT_ACCESS
    
    g_hw_perf.l1_access_fd = perf_event_open(&pe, 0, -1, -1, 0);
    if (g_hw_perf.l1_access_fd == -1) {
        log_println("Failed to open L1 data cache access counter");
    }

    // Last Level Cache (LLC) misses
    // Removed the invalid L1_DCACHE_ACCESSES << 24
    pe.config = (PERF_COUNT_HW_CACHE_LL) |
                (PERF_COUNT_HW_CACHE_OP_READ << 8) |
                (PERF_COUNT_HW_CACHE_RESULT_MISS << 16);
    
    g_hw_perf.llc_miss_fd = perf_event_open(&pe, 0, -1, -1, 0);
    if (g_hw_perf.llc_miss_fd == -1)
    {
        log_println("Failed to open Last Level Cache miss counter");
    }
    
    // CPU cycles counter
    pe.type = PERF_TYPE_HARDWARE; // Reset type for hardware events
    pe.config = PERF_COUNT_HW_CPU_CYCLES;
    g_hw_perf.cycles_fd = perf_event_open(&pe, 0, -1, -1, 0);
    if (g_hw_perf.cycles_fd == -1) 
    {
        log_println("Failed to open CPU cycles counter");
    }
    
    // Instructions counter    
    pe.config = PERF_COUNT_HW_INSTRUCTIONS;
    g_hw_perf.instructions_fd = perf_event_open(&pe, 0, -1, -1, 0);
    if (g_hw_perf.instructions_fd == -1) {
        log_println("Failed to open Instructions counter");
    }
    
    // Check if at least some critical counters are available
    if (g_hw_perf.l1_miss_fd != -1 || g_hw_perf.l1_access_fd != -1 ||
        g_hw_perf.llc_miss_fd != -1 || g_hw_perf.cycles_fd != -1 ||
        g_hw_perf.instructions_fd != -1)
    {
        g_hw_perf.hw_counters_available = true;
        log_println("Hardware performance counters initialized successfully");
        return true;
    }
    
    log_println("No hardware performance counters could be initialized.");
    return false;
}

static uint64_t read_hw_counter(int fd) {
    uint64_t count = 0;
    if (fd != -1 && read(fd, &count, sizeof(count)) == sizeof(count)) 
    {
        return count;
    }
    return 0;
}

static void enable_hw_counters(void) 
{
    if (!g_hw_perf.hw_counters_available) return;
    
    if (g_hw_perf.l1_miss_fd != -1) 
    {
        ioctl(g_hw_perf.l1_miss_fd, PERF_EVENT_IOC_ENABLE, 0);
    }
    if (g_hw_perf.l1_access_fd != -1)
    {
        ioctl(g_hw_perf.l1_access_fd, PERF_EVENT_IOC_ENABLE, 0); 
    }
    if (g_hw_perf.llc_miss_fd != -1)
    {
        ioctl(g_hw_perf.llc_miss_fd, PERF_EVENT_IOC_ENABLE, 0);
    }
    if (g_hw_perf.cycles_fd != -1)
    {
        ioctl(g_hw_perf.cycles_fd, PERF_EVENT_IOC_ENABLE, 0);
    }
    if (g_hw_perf.instructions_fd != -1)
    {
        ioctl(g_hw_perf.instructions_fd, PERF_EVENT_IOC_ENABLE, 0);
    }
}

static void disable_hw_counters(void)
{
    if (!g_hw_perf.hw_counters_available) return;
    
    if (g_hw_perf.l1_miss_fd != -1)
    {
        ioctl(g_hw_perf.l1_miss_fd, PERF_EVENT_IOC_DISABLE, 0);
    }
    if (g_hw_perf.l1_access_fd != -1)
    {
        ioctl(g_hw_perf.l1_access_fd, PERF_EVENT_IOC_DISABLE, 0);
    }
    
    if (g_hw_perf.llc_miss_fd != -1)
    {
        ioctl(g_hw_perf.llc_miss_fd, PERF_EVENT_IOC_DISABLE, 0);
    }
    if (g_hw_perf.cycles_fd != -1)
    {
        ioctl(g_hw_perf.cycles_fd, PERF_EVENT_IOC_DISABLE, 0);
    }
    if (g_hw_perf.instructions_fd != -1)
    {
        ioctl(g_hw_perf.instructions_fd, PERF_EVENT_IOC_DISABLE, 0);
    }
}

static void cleanup_hw_counters(void) {
    if (g_hw_perf.l1_miss_fd != -1) close(g_hw_perf.l1_miss_fd);
    if (g_hw_perf.l2_miss_fd != -1) close(g_hw_perf.l2_miss_fd);
    if (g_hw_perf.l3_miss_fd != -1) close(g_hw_perf.l3_miss_fd);
    if (g_hw_perf.llc_miss_fd != -1) close(g_hw_perf.llc_miss_fd);
    if (g_hw_perf.cycles_fd != -1) close(g_hw_perf.cycles_fd);
    if (g_hw_perf.instructions_fd != -1) close(g_hw_perf.instructions_fd);
}

#endif // __linux__

/*static _in_use _check_if_index_is_in_use(_index index)
{
  return g_prof_in_use[index];
}*/

static _index _find_free_index(void)
{
    for (_index i = 0; i < TOTAL_FUNCTIONS; i++)
    {
        if (!g_prof_in_use[i])
        {
            // reserve it:
            g_prof_in_use[i] = true;
            // bump our "used slots" count if needed
            if (i + 1 > g_current_free_index)
            {
                g_current_free_index = i + 1;
            }
            return i;
        }
    }
    return (_index)-1;  // no slots left
}

// Cache calibration implementation
static uint64_t measure_pointer_chase_latency(void* buffer, size_t buffer_size, int iterations)
{
    // Create a random pointer chain through the buffer
    size_t* ptrs = (size_t*)buffer;
    size_t num_ptrs = buffer_size / sizeof(size_t);
    
    // Initialize sequential chain first
    for (size_t i = 0; i < num_ptrs - 1; i++) 
    {
        ptrs[i] = (size_t)&ptrs[i + 1];
    }
    ptrs[num_ptrs - 1] = (size_t)&ptrs[0];  // Close the loop
    
    // Fisher-Yates shuffle to randomize the chain
    for (size_t i = num_ptrs - 1; i > 0; i--)
    {
        size_t j = rand() % (i + 1);
        size_t temp = ptrs[i];
        ptrs[i] = ptrs[j];
        ptrs[j] = temp;
    }
    
    volatile size_t* current = (volatile size_t*)ptrs;
    uint64_t start_cycles = get_cycle_count_serialized();
    
    for (int i = 0; i < iterations; i++)
    {
        current = (volatile size_t*)*current;
    }
    
    uint64_t end_cycles = get_cycle_count_serialized();
    
    // Prevent optimization
    (void)current;
    
    return (end_cycles - start_cycles) / iterations;
}

void profiler_calibrate_cache_latency(void)
{
    if (g_cache_profile.calibrated)
    {
        return;  // Already calibrated
    }
    
    log_println("Starting cache latency calibration...");
    
    // L1 cache test (typically 32KB)
    size_t l1_size = 16 * 1024;  // Conservative estimate
    void* l1_buffer = malloc(l1_size);
    if (!l1_buffer) 
    {
        log_println("Failed to allocate L1 test buffer");
        return;
    }
    
    g_cache_profile.l1_latency_cycles = measure_pointer_chase_latency(l1_buffer, l1_size, 10000);
    free(l1_buffer);
    
    // L2 cache test (typically 256KB-1MB)
    size_t l2_size = 512 * 1024;
    void* l2_buffer = malloc(l2_size);
    if (!l2_buffer)
    {
        log_println("Failed to allocate L2 test buffer");
        return;
    }
    
    g_cache_profile.l2_latency_cycles = measure_pointer_chase_latency(l2_buffer, l2_size, 5000);
    free(l2_buffer);
    
    // L3 cache test (typically 8MB-32MB)
    size_t l3_size = 32 * 1024 * 1024;
    void* l3_buffer = malloc(l3_size);
    if (!l3_buffer)
    {
        log_println("Failed to allocate L3 test buffer");
        return;
    }
    
    g_cache_profile.l3_latency_cycles = measure_pointer_chase_latency(l3_buffer, l3_size, 2000);
    free(l3_buffer);
    
    // DRAM test (large buffer that won't fit in cache)
    size_t dram_size = 128 * 1024 * 1024;  // 128MB
    void* dram_buffer = malloc(dram_size);
    if (!dram_buffer) {
        log_println("Failed to allocate DRAM test buffer");
        return;
    }
    
    g_cache_profile.dram_latency_cycles = measure_pointer_chase_latency(dram_buffer, dram_size, 1000);
    free(dram_buffer);
    
    g_cache_profile.calibrated = true;
    
    log_println("Cache latency calibration complete:");
    log_println("L1 latency: {}" , g_cache_profile.l1_latency_cycles);
    log_println("L2 latency: {}" , g_cache_profile.l2_latency_cycles);
    log_println("L3 latency: {}" , g_cache_profile.l3_latency_cycles);
    log_println("DRAM latency: {}" , g_cache_profile.dram_latency_cycles);
}

cache_latency_profile_t* profiler_get_cache_profile(void) {
    return &g_cache_profile;
}

static void _cache_cycle_estimation(_index index, uint64_t cycles)
{
    if (!g_cache_profile.calibrated)
    {
        // Use old static estimates if not calibrated
        if (cycles <= EST_L1_MAX_CYCLES) 
        {
            g_prof_cache_stat[index]._l1_access_total++;
        } 
        else if (cycles <= EST_L2_MAX_CYCLES) 
        {
            g_prof_cache_stat[index]._l2_access_total++;
            g_prof_cache_stat[index]._l1_misses++;
        } 
        else if (cycles <= EST_L3_MAX_CYCLES) {
            g_prof_cache_stat[index]._l3_access_total++;
            g_prof_cache_stat[index]._l2_misses++;
        } 
        else
        {
            g_prof_cache_stat[index]._dram_access_total++;
            g_prof_cache_stat[index]._l3_misses++;
        }
        return;
    }
    
    // Use dynamically measured latencies with tolerance
    uint64_t l1_threshold = g_cache_profile.l1_latency_cycles + 
                           (g_cache_profile.l1_latency_cycles >> 3); // +12.5%
    uint64_t l2_threshold = g_cache_profile.l2_latency_cycles + 
                           (g_cache_profile.l2_latency_cycles >> 2); // +25%
    uint64_t l3_threshold = g_cache_profile.l3_latency_cycles + 
                           (g_cache_profile.l3_latency_cycles >> 2); // +25%
    
    if (cycles <= l1_threshold) 
    {
        g_prof_cache_stat[index]._l1_access_total++;
    }
    else if (cycles <= l2_threshold) 
    {
        g_prof_cache_stat[index]._l2_access_total++;
        g_prof_cache_stat[index]._l1_misses++;
    }
    else if (cycles <= l3_threshold)
    {
        g_prof_cache_stat[index]._l3_access_total++;
        g_prof_cache_stat[index]._l2_misses++;
    }
    else 
    {
        g_prof_cache_stat[index]._dram_access_total++;
        g_prof_cache_stat[index]._l3_misses++;
    }
}

#ifdef __linux__
// Hardware-based cache miss tracking (when available)
static void _cache_hw_measurement(_index index)
{
    if (!g_hw_perf.hw_counters_available) return;
    
    uint64_t l1_misses = read_hw_counter(g_hw_perf.l1_miss_fd);
    uint64_t llc_misses = read_hw_counter(g_hw_perf.llc_miss_fd);
    uint64_t l1_accesses = read_hw_counter(g_hw_perf.l1_access_fd);
    
    // Update hardware-measured cache statistics
    g_prof_cache_stat[index]._l1_misses += l1_misses;
    g_prof_cache_stat[index]._l3_misses += llc_misses; // LLC is usually L3
    g_prof_cache_stat[index]._l1_access_total += l1_accesses;
    
    // Reset counters for next measurement
    if (g_hw_perf.l1_miss_fd != -1) 
    {
        ioctl(g_hw_perf.l1_miss_fd, PERF_EVENT_IOC_RESET, 0);
    }
    if (g_hw_perf.llc_miss_fd != -1) {
        ioctl(g_hw_perf.llc_miss_fd, PERF_EVENT_IOC_RESET, 0);
    }
    if (g_hw_perf.l1_access_fd != -1) 
    {
        ioctl(g_hw_perf.l1_access_fd, PERF_EVENT_IOC_RESET, 0);
    }
}
#endif

static void _handle_prof_add_event(enum prof_add type,_index index,uint64_t value)
{
  switch (type)
  {
    case pa_cycles_total:
      g_prof_cpu_stat[index]._total_cycles += value;
      break;

    case pa_cycles_min:
      if (g_prof_cpu_stat[index]._cycles_min == 0 || value < g_prof_cpu_stat[index]._cycles_min)
      {
        g_prof_cpu_stat[index]._cycles_min = value;
      }
      break;

    case pa_cycles_max:
      if (value > g_prof_cpu_stat[index]._cycles_max)
      {
        g_prof_cpu_stat[index]._cycles_max = value;
      }
      break;

    case pa_time_total: 
      g_prof_time_stat[index]._total_time += value;
      break;

    case pa_time_min:
      if (g_prof_time_stat[index]._min_time == 0 || value < g_prof_time_stat[index]._min_time)
      {
        g_prof_time_stat[index]._min_time = value;
      }
      break;

    case pa_time_max:
      if (value > g_prof_time_stat[index]._max_time)
      {
        g_prof_time_stat[index]._max_time = value;
      }
      break;

    case pa_total_calls: 
      g_prof_call_stat[index]._total_calls += value;
      break;

    case pa_early_condition_return:
      g_prof_call_stat[index]._early_condition_return += value;
      break;

    case pa_successful_return: 
      g_prof_call_stat[index]._successful_return += value;
      break;

    case pa_failed_return:
      g_prof_call_stat[index]._failed_return += value;
      break;

    case pa_cache_access:
#ifdef __linux__
      if (g_hw_perf.hw_counters_available) {
        _cache_hw_measurement(index);
      } else {
        _cache_cycle_estimation(index, value);
      }
#else
      _cache_cycle_estimation(index, value);
#endif
      break;

    default: 
      log_println("unknown enum passed! {u64}", (uint64_t)type); 
  }
}

static uint64_t _handle_prof_output_event(enum prof_output type, _index index)
{
  switch (type)
  {
    case pa_l1_misses:
      /*g_prof_cache_stat[index]._l1_misses = (g_prof_cache_stat[index]._l2_access_total 
        + g_prof_cache_stat[index]._l3_access_total + g_prof_cache_stat[index]._dram_access_total);*/
      return g_prof_cache_stat[index]._l1_misses;

    case pa_l2_misses:
      /*g_prof_cache_stat[index]._l2_misses = (g_prof_cache_stat[index]._l3_access_total + g_prof_cache_stat[index]._dram_access_total);*/
      return g_prof_cache_stat[index]._l2_misses;

    case pa_l3_misses:
      /*g_prof_cache_stat[index]._l3_misses = (g_prof_cache_stat[index]._dram_access_total); */
      return g_prof_cache_stat[index]._l3_misses;

    case pa_cycles_avg: 
      g_prof_cpu_stat[index]._avg_cycles = ((f64) g_prof_cpu_stat[index]._total_cycles / (f64)g_prof_call_stat[index]._total_calls);
      return g_prof_cpu_stat[index]._avg_cycles;

    case pa_time_avg:
      g_prof_time_stat[index]._avg_time = ((f64) g_prof_time_stat[index]._total_time / (f64) g_prof_call_stat[index]._total_calls);
      return g_prof_time_stat[index]._avg_time;

    default: 
      log_println("unknown enum passed! {u64}", (uint64_t)type);
      return 0;
  }
}

static void _prof_print(_index index)
{
  log_println("Index {u64}", g_prof_stat_head[index]._id);
  log_println("File name: {str}",g_prof_stat_head[index]._file_name);
  log_println("Function Name: {str}" , g_prof_stat_head[index]._func_name);
  log_println("Line number: {u16}", g_prof_stat_head[index]._line);

  log_println("Total Cycles: {u64} ", g_prof_cpu_stat[index]._total_cycles);
  log_println("Minimum Cycles: {u64}", g_prof_cpu_stat[index]._cycles_min);
  log_println("Maximum Cycles: {u64}", g_prof_cpu_stat[index]._cycles_max);
  log_println("Average Cycles: {f64}", g_prof_cpu_stat[index]._avg_cycles);

  log_println("Total time: {f64}", g_prof_time_stat[index]._total_time);
  log_println("Minimum time: {f64}", g_prof_time_stat[index]._min_time);
  log_println("Maximum time: {f64}", g_prof_time_stat[index]._max_time);
  log_println("Average time: {f64}", g_prof_time_stat[index]._avg_time);

  log_println("Total calls: {u64}",g_prof_call_stat[index]._total_calls);
  log_println("Total early condition exits: {u64}",g_prof_call_stat[index]._early_condition_return);
  log_println("Failed returns: {u64}",g_prof_call_stat[index]._failed_return);
  log_println("Successful returns {u64}",g_prof_call_stat[index]._successful_return);

  log_println("Total L1 Accesses: {u64}",g_prof_cache_stat[index]._l1_access_total);
  log_println("Total L2 Accesses: {u64}",g_prof_cache_stat[index]._l2_access_total);
  log_println("Total L3 Accesses: {u64}",g_prof_cache_stat[index]._l3_access_total);
  log_println("Total DRAM Accesses: {u64}",g_prof_cache_stat[index]._dram_access_total);

  log_println("Total L1 Misses: {u64}",g_prof_cache_stat[index]._l1_misses);
  log_println("Total L2 Misses: {u64}",g_prof_cache_stat[index]._l2_misses);
  log_println("Total L3 Misses: {u64}",g_prof_cache_stat[index]._l3_misses);

  log_println("------------------------------------------------------------");
}

void profiler_init(void)
{
    memset(g_prof_stat_head, 0, sizeof(g_prof_stat_head));
    memset(g_prof_cpu_stat, 0, sizeof(g_prof_cpu_stat));
    memset(g_prof_time_stat, 0, sizeof(g_prof_time_stat));
    memset(g_prof_call_stat, 0, sizeof(g_prof_call_stat));
    memset(g_prof_cache_stat, 0, sizeof(g_prof_cache_stat));
    memset(g_prof_in_use, 0, sizeof(g_prof_in_use));
    
    // Initialize hardware performance counters if available
#ifdef __linux__
    init_hardware_performance_counters();
    if (g_hw_perf.hw_counters_available) {
        enable_hw_counters();
    }
#endif
    
    // Calibrate cache latencies
    profiler_calibrate_cache_latency();
}

void profiler_end(void)
{
#ifdef __linux__
    if (g_hw_perf.hw_counters_available) {
        disable_hw_counters();
        cleanup_hw_counters();
    }
#endif
  /*for (_index i = 0; i < g_current_free_index; i++)
  {
    _handle_prof_output_event(pa_l1_misses, i);
    _handle_prof_output_event(pa_l2_misses, i);
    _handle_prof_output_event(pa_l3_misses, i);

    _handle_prof_output_event(pa_time_avg, i);
    _handle_prof_output_event(pa_cycles_avg, i);
  }*/
}

void profiler_print_all(void)
{
  for (_index i = 0; i < g_current_free_index; i++)
  {
    _prof_print(i);
  }
}

uint64_t profiler_add_function(const char* file_name,const char* func_name,u16 line_number)
{
  _index assigned = _find_free_index();
  g_prof_in_use[assigned] = true; // we mark the index as "in use" (true)
  
  g_prof_stat_head[assigned]._id = assigned;

  strncpy(g_prof_stat_head[assigned]._file_name, file_name, sizeof(g_prof_stat_head[assigned]._file_name) - 1);
  g_prof_stat_head[assigned]._file_name[sizeof(g_prof_stat_head[assigned]._file_name) - 1] = '\0';

  strncpy(g_prof_stat_head[assigned]._func_name, func_name, sizeof(g_prof_stat_head[assigned]._func_name) -1);
  g_prof_stat_head[assigned]._func_name[sizeof(g_prof_stat_head[assigned]._func_name) - 1] = '\0';

  g_prof_stat_head[assigned]._line = line_number;

  return assigned;
}

void profiler_add(enum prof_add type,uint64_t value,_index index)
{
  _handle_prof_add_event(type,index,value);
}

u64 profiler_output(enum prof_output type,_index index)
{
  return _handle_prof_output_event(type,index);
}
