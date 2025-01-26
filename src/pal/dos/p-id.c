#include <pal.h>

bool
pal_get_machine_id(uint8_t *mid)
{
    // No known reliable way to identify the machine under DOS
    return false;
}
