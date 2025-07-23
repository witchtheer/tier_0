#include <utils/macros.h>

#if PLATFORM_MACOS

void init(void) {}

#else
#error "linux/impl.c included in non-Linux build!"
#endif
