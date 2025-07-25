#include <utils/log.h>

#include <inttypes.h>
#include <string.h>

EXTERN_C_START

static u16 log_parse_and_print(const char* fmt, va_list args)
{
    const char* p = fmt;
    
    while (*p) {
        if (*p == '{' && *(p + 1) != '\0') {
            p++; // skip '{'
            
            // check for type specifiers
            if (*p == 'u')
            {
                p++;
                if (*p == '8' && *(p + 1) == '}') {
                    log_u8(va_arg(args, int));
                    p += 2;
                }
                else if (*p == '1' && *(p + 1) == '6' && *(p + 2) == '}')
                {
                    log_u16(va_arg(args, int));
                    p += 3;
                } 
                else if (*p == '3' && *(p + 1) == '2' && *(p + 2) == '}') 
                {
                    log_u32(va_arg(args, u32));
                    p += 3;
                } 
                else if (*p == '6' && *(p + 1) == '4' && *(p + 2) == '}')
                {
                    log_u64(va_arg(args, u64));
                    p += 3;
                } 
                else 
                {
                    putchar('{');
                    putchar('u');
                }
            }
            else if(*p == 's')
            {
                p++;
                if (*p == '8' && *(p + 1) == '}') 
                {
                    log_s8(va_arg(args, int));
                    p += 2;
                } 
                else if (*p == '1' && *(p + 1) == '6' && *(p + 2) == '}')
                {
                    log_s16(va_arg(args, int));
                    p += 3;
                } 
                else if (*p == '3' && *(p + 1) == '2' && *(p + 2) == '}')
                {
                    log_s32(va_arg(args, s32));
                    p += 3;
                } 
                else if (*p == '6' && *(p + 1) == '4' && *(p + 2) == '}')
                {
                    log_s64(va_arg(args, s64));
                    p += 3;
                } 
                else if (strncmp(p, "tr}", 3) == 0)
                {
                    log_str(va_arg(args, const char*));
                    p += 3;
                } 
                else 
                {
                    putchar('{');
                    putchar('s');
                }
            }
            else if (*p == 'f') 
            {
                p++;
                if (*p == '3' && *(p + 1) == '2' && *(p + 2) == '}') {
                    log_f32(va_arg(args, double));
                    p += 3;
                } else if (*p == '6' && *(p + 1) == '4' && *(p + 2) == '}') {
                    log_f64(va_arg(args, double));
                    p += 3;
                } else {
                    putchar('{');
                    putchar('f');
                }
            }
            else if (*p == '}') 
            {
                // Default to int for bare {}
                printf("%d", va_arg(args, int));
                p++;
            }
            else 
            {
                // Unknown format, just print it
                putchar('{');
            }
        }
        else 
        {
            putchar(*p);
            p++;
        }
    }
    
    return 0;
}

void log_print(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    log_parse_and_print(fmt, args);
    va_end(args);
}

void log_println(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    log_parse_and_print(fmt, args);
    va_end(args);
    putchar('\n');
}

EXTERN_C_END
