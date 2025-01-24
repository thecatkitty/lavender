#include <pal.h>

extern char _binary_obj_version_txt_start[];

bool
pal_get_machine_id(uint8_t *mid)
{
    // No known reliable way to identify the machine under DOS
    return false;
}

const char *
pal_get_version_string(void)
{
    return _binary_obj_version_txt_start;
}
