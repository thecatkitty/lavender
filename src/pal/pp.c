#include <pal.h>

uint32_t
palpp_get_counter(void)
{
    return pal_get_counter();
}

uint32_t
palpp_get_ticks(unsigned ms)
{
    return pal_get_ticks(ms);
}
