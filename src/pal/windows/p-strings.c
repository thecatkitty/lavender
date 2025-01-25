#include <arch/windows.h>
#include <pal.h>

#include "impl.h"

int
pal_load_string(unsigned id, char *buffer, int max_length)
{
    LPWSTR wbuffer;
    int    length, mb_length;

    LOG("entry, id: %u, buffer: %p, max_length: %d", id, buffer, max_length);

    wbuffer = (PWSTR)malloc(max_length * sizeof(WCHAR));
    if (NULL == wbuffer)
    {
        return -1;
    }

    length = LoadStringW(NULL, id, wbuffer, max_length);
    if (0 > length)
    {
        const char msg[] = "!!! string missing !!!";
        strncpy(buffer, msg, max_length);
        buffer[max_length - 1] = 0;

        LOG("exit, string missing");
        return sizeof(msg) - 1;
    }

    mb_length = WideCharToMultiByte(CP_UTF8, 0, wbuffer, length, buffer,
                                    max_length, NULL, NULL);
    buffer[mb_length] = 0;
    free(wbuffer);

    LOG("exit, '%s'", buffer);
    return length;
}
