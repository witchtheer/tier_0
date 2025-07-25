#pragma once

#include <utils/macros.h>
#include <utils/types.h>
#include <stdio.h>
#include <stdarg.h>

EXTERN_C_START

void log_print(const char* fmt, ...);
void log_println(const char* fmt, ...);

// skips the formating
#define log_u8(val)    printf("%u", (u8)(val))
#define log_u16(val)   printf("%u", (u16)(val))
#define log_u32(val)   printf("%u", (u32)(val))
#define log_u64(val)   printf("%" PRIu64, (u64)(val))

#define log_s8(val)    printf("%d", (s8)(val))
#define log_s16(val)   printf("%d", (s16)(val))
#define log_s32(val)   printf("%d", (s32)(val))
#define log_s64(val)   printf("%" PRId64, (s64)(val))

#define log_f32(val)   printf("%.2f", (f32)(val))
#define log_f64(val)   printf("%.4f", (f64)(val))
#define log_str(val)   printf("%s", (const char*)(val))

#define LOG_INFO(fmt, ...)    do { printf("[INFO] " fmt "\n", __VA_ARGS__); } while(0)
#define LOG_WARN(fmt, ...)    do { printf("[WARN] " fmt "\n", __VA_ARGS__); } while(0)
#define LOG_ERROR(fmt, ...)   do { printf("[ERROR] " fmt "\n", __VA_ARGS__); } while(0)

#ifdef DEBUG
  #define LOG_DEBUG(fmt, ...) do { printf("[DEBUG] %s:%d " fmt "\n", __FILE__, __LINE__, __VA_ARGS__); } while(0)
#else
  #define LOG_DEBUG(fmt, ...) do { } while(0)
#endif

// profiler specific
#define LOG_PROFILE(fmt, ...) do { printf("[PROF] " fmt "\n", __VA_ARGS__); } while(0)

// no formating overhead!
#define LOG_RAW(str)         fputs(str, stdout)
#define LOG_RAW_LN(str)      puts(str)

EXTERN_C_END
