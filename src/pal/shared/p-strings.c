#include <string.h>

#include <nls.h>
#include <pal.h>

extern nls_locstr STRINGS[];

int
pal_load_string(unsigned id, char *buffer, int max_length)
{
    LOG("entry, id: %u, buffer: %p, max_length: %d", id, buffer, max_length);

    nls_locstr *it = STRINGS;
    while (UINT_MAX != it->id)
    {
        if (it->id == id)
        {
            strncpy(buffer, it->str, max_length);
            break;
        }

        it++;
    }

    if (UINT_MAX == it->id)
    {
        const char msg[] = "!!! string missing !!!";
        strncpy(buffer, msg, max_length);

        LOG("exit, string missing");
    }

    LOG("exit, '%s'", buffer);
    return strlen(buffer);
}
