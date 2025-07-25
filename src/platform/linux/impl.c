#include <platform/platform.h>

#if PLATFORM_LINUX

_cpu_cores impl_hw_get_cpu_cores(void)
{
    NOT_IMPLEMENTED_RETURN_VAL_DETAILED(0);
}

_cpu_threads impl_hw_get_cpu_threads(void)
{
    NOT_IMPLEMENTED_RETURN_VAL_DETAILED(0);
}

_mem_size impl_hw_get_total_memory(void)
{
    NOT_IMPLEMENTED_RETURN_VAL_DETAILED(0);
}

_mem_size impl_hw_get_free_memory(void)
{
    NOT_IMPLEMENTED_RETURN_VAL_DETAILED(0);
}

_mem_size impl_hw_get_used_memory(void)
{
    NOT_IMPLEMENTED_RETURN_VAL_DETAILED(0);
}

#else
#error "linux/impl.c included in non-Linux build!"
#endif
