#include <pal.h>

extern char _binary_obj_version_txt_start[];

const char *
pal_get_version_string(void)
{
    return _binary_obj_version_txt_start;
}
