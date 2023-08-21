#include <pal.h>

void
__pal_log_time(void)
{
    uint32_t counter = pal_get_counter();
    fprintf(stderr, "[%4u.%03u] ", counter / 1000, counter % 1000);
}
