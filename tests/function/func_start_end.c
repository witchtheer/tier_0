#include "utils/log.h"
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

  PROFILE_FUNCTION_END;

  return result;
}

int main(void)
{
  profiler_init();
  multiply();
  log_println("running!");

  profiler_end();
  profiler_print_all();
  return 0;
}
