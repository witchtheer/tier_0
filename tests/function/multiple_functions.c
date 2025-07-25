#include "perf/instr.h"
#include <tier_0.h>

uint32_t multiply(void)
{
  PROFILE_FUNCTION_START;

  uint32_t result = 0;
  for (uint32_t x = 16; x > 0; x--)
  {
    for (uint32_t y = 16; y > 0; y--)
    {
      result += (x * y);
    }
  }

  PROFILE_HINT_SUCCESSFUL_RETURN;
  PROFILE_FUNCTION_END;

  return result;
}

uint32_t multiply_2(void)
{
  PROFILE_FUNCTION_START;

  uint32_t result = 0;
  for (uint32_t x = 48; x > 0; x--)
  {
    for (uint32_t y = 48; y > 0; y--)
    {
      result += (x * y);
    }
  }

  PROFILE_FUNCTION_END;
  PROFILE_HINT_SUCCESSFUL_RETURN;

  return result;
}

int main(void)
{
  profiler_init();

  for (uint32_t i = 0; i < 96; i++)
  {
    multiply();
    multiply_2();
  }
  profiler_end();
  profiler_print_all();
  return 0;
}
