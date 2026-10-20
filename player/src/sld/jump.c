#include "sld_impl.h"

int
__sld_load_conditional(const char *str, sld_entry *out)
{
    const char *cur = str;
    int         length = 0;

    length = __sld_loadu(cur, &out->posy);
    if (0 > length)
    {
        __sld_errmsgcpy(out, IDS_INVALIDCMPVAL);
        return SLD_ARGERR;
    }
    cur += length;

    __sld_try_load(__sld_load_content, cur, out);

    return cur - str;
}
