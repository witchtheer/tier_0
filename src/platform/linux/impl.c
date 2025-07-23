#include <utils/macros.h>

#if PLATFORM_LINUX

void init(void) {}

#else
#error "linux/impl.c included in non-Linux build!"
#endif
