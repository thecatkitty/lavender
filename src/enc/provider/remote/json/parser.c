#include <string.h>

#define JSMN_HEADER
#include "jsmn/jsmn.h"

#define MAX_TOKENS 32

const char *
encjson_get_child_string(const char *str,
                         size_t      str_len,
                         const char *key,
                         size_t     *child_len)
{
    jsmn_parser jsmn;
    jsmntok_t   tokens[MAX_TOKENS];
    int         i, count;

    jsmn_init(&jsmn);
    count = jsmn_parse(&jsmn, str, str_len, tokens, MAX_TOKENS);
    if (0 > count)
    {
        return NULL;
    }

    if (JSMN_OBJECT != tokens[0].type)
    {
        return NULL;
    }

    i = 1;
    while (i < count - 1)
    {
        const char *start = str + tokens[i].start;
        const char *end = str + tokens[i].end;

        if (JSMN_STRING != tokens[i].type)
        {
            return NULL;
        }

        if (0 == memcmp(key, start, end - start))
        {
            break;
        }

        i += 2 + tokens[i].size;
    }

    if (JSMN_STRING != tokens[i + 1].type)
    {
        return NULL;
    }

    i++;
    *child_len = (size_t)(tokens[i].end - tokens[i].start);
    return str + tokens[i].start;
}
