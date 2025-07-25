#pragma once

#include <assert.h>

#include <utils/macros.h>
#include <utils/types.h>

#include <perf/arch.h>
#include <perf/timer.h>

#ifndef MAX_FUNCTIONS
#define MAX_FUNCTIONS 128
#endif

#define PROFILER_RESERVED 32
#define STARTING_INDEX PROFILER_RESERVED

#define TOTAL_FUNCTIONS (MAX_FUNCTIONS + PROFILER_RESERVED)

#define EST_L1_MAX_CYCLES   25
#define EST_L2_MAX_CYCLES   35
#define EST_L3_MAX_CYCLES   90

typedef bool      _in_use;
typedef uint64_t  _index;

typedef struct ALIGNAS(8) prof_stat_head_t
{
  _index _id;
  char  _file_name[144];
  char  _func_name[96];
  u16   _line;

  char padding[6];
} prof_stat_head_t;

typedef struct ALIGNAS(8) prof_cpu_stat_t
{
  u64   _total_cycles;
  u64   _cycles_min;
  u64   _cycles_max;
  f64   _avg_cycles;
} prof_cpu_stat_t;

typedef struct ALIGNAS(8) prof_time_stat_t
{
  f64   _total_time;
  f64   _max_time;
  f64   _min_time;
  f64   _avg_time;
} prof_time_stat_t;

typedef struct ALIGNAS(8) prof_call_stat_t
{
  u64   _total_calls;
  u64   _early_condition_return;
  u64   _successful_return;
  u64   _failed_return;
} prof_call_stat_t;

typedef struct  ALIGNAS(8) prof_cache_stat_t //estimated
{
  u64 _l1_access_total;
  u64 _l2_access_total;
  u64 _l3_access_total;
  u64 _dram_access_total;

  u64 _l1_misses;
  u64 _l2_misses;
  u64 _l3_misses;

  char padding[8]; // why did i decide? idk
} prof_cache_stat_t;

typedef struct cache_latency_profile_t {
    uint64_t l1_latency_cycles;
    uint64_t l2_latency_cycles; 
    uint64_t l3_latency_cycles;
    uint64_t dram_latency_cycles;
    bool calibrated;
} cache_latency_profile_t;

enum prof_add 
{
  // cpu 
  pa_cycles_total,
  pa_cycles_min,
  pa_cycles_max,

  // time 
  pa_time_total,
  pa_time_min,
  pa_time_max,

  // func_state
  pa_total_calls,
  pa_early_condition_return,
  pa_successful_return,
  pa_failed_return,

  // cache
  pa_cache_access,
}; 

enum prof_output
{
  pa_cycles_avg,
  pa_time_avg,

  pa_l1_misses,
  pa_l2_misses,
  pa_l3_misses,
};

void profiler_init(void);
void profiler_end(void);

uint64_t profiler_add_function(const char* file_name,const char* func_name,u16 line_number);

void profiler_add(enum prof_add type,uint64_t value,_index index);
u64 profiler_output(enum prof_output type,_index index);

void profiler_print_all(void);

void profiler_calibrate_cache_latency(void);
cache_latency_profile_t* profiler_get_cache_profile(void);

static_assert(sizeof(prof_stat_head_t) == 256,  "prof_stat_head_t   isnt 256 bytes!");
static_assert(sizeof(prof_cpu_stat_t) == 32,    "prof_cpu_stat_t    isnt 32 bytes!");
static_assert(sizeof(prof_time_stat_t) == 32,   "prof_time_stat_t   isnt 32 bytes!");
static_assert(sizeof(prof_call_stat_t) == 32,   "prof_call_stat_t   isnt 32 bytes!");
static_assert(sizeof(prof_cache_stat_t) == 64,  "prof_cache_stat_t  isnt 64 bytes");

#define PROFILE_FUNCTION_START                \
  static bool     _initialized_ = false;      \
  static _index   _function_index_ = 0;       \
  uint64_t _prof_start_cycles = get_cycle_count_enhanced(); \
  uint64_t _prof_start_time   = get_nanoseconds();          \
  if (_initialized_ == false)                               \
  {                                                         \
    _function_index_ = profiler_add_function(__FILE__, FUNCTION_NAME, __LINE__);  \
    _initialized_ = true;                                                         \
  }                                                                               \
  profiler_add(pa_total_calls,1,_function_index_);                              

#define PROFILE_FUNCTION_END                                                \
  uint64_t _prof_end_cycles = get_cycle_count_enhanced();                   \
  uint64_t _prof_end_time   = get_nanoseconds();                            \
  uint64_t _prof_cycles_total   = _prof_end_cycles - _prof_start_cycles;    \
  uint64_t _prof_time_total     = _prof_end_time - _prof_start_time;        \
  profiler_add(pa_cycles_total,_prof_cycles_total,_function_index_);        \
  profiler_add(pa_cycles_min,_prof_cycles_total,_function_index_);          \
  profiler_add(pa_cycles_max,_prof_cycles_total,_function_index_);          \
  profiler_add(pa_time_total,_prof_time_total,_function_index_);            \
  profiler_add(pa_time_min,_prof_time_total,_function_index_);              \
  profiler_add(pa_time_max,_prof_time_total,_function_index_);              \

#define PROFILE_HINT_SUCCESSFUL_RETURN                      \
  profiler_add(pa_successful_return,1,_function_index_);    \

#define PROFILE_CACHE_START(name)                           \
  uint64_t name##_cache_start = get_cycle_count_enhanced(); \

#define PROFILE_CACHE_END(name)                                     \
  uint64_t name##_cache_end = get_cycle_count_enhanced();           \
  uint64_t name##_cache_delta = name##_cache_end - name##_cache_start;    \
  profiler_add(pa_cache_access,name##_cache_delta,_function_index_)   \
