#include <platform/platform.h>

static Hardware_Specifications Hardware_specs = {0};

percentage shared_calc_mem_usage_v(mem_size total, mem_size free)
{
    if (total == 0) return 0;
    return (percentage)(((total - free) * 100) / total);
}

percentage shared_calc_mem_usage_s(Hardware_Specifications *hw_specs)
{
    hw_specs->memory_usage_percentage = shared_calc_mem_usage_v(hw_specs->total_memory, hw_specs->free_memory);
    return hw_specs->memory_usage_percentage;
}

Hardware_Specifications *shared_get_hw_specs(void)
{
    return &Hardware_specs;
}
