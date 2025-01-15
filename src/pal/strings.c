#include <string.h>

#include <fmt/exe.h>
#include <pal.h>

#if defined(__ia16__) || defined(__linux__)
extern char __w32_rsrc_start[];

int
pal_load_string(unsigned id, char *buffer, int max_length)
{
    LOG("entry, id: %u, buffer: %p, max_length: %d", id, buffer, max_length);

    int length = exe_pe_load_string(__w32_rsrc_start, id, buffer, max_length);
    if (0 > length)
    {
        const char msg[] = "!!! string missing !!!";
        strncpy(buffer, msg, max_length);

        LOG("exit, string missing");
        return sizeof(msg) - 1;
    }

    LOG("exit, '%s'", buffer);
    return length;
}
#endif
