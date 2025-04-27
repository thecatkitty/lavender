#include <stdlib.h>
#include <string.h>

#include <nls.h>
#include <pal.h>

#ifdef CONFIG_COMPACT
extern nls_locstr STRINGS[];
#else
extern nls_locstr  STRINGS_CS_CZ[];
extern nls_locstr  STRINGS_EN_US[];
extern nls_locstr  STRINGS_PL_PL[];
static nls_locstr *STRINGS = NULL;

typedef struct
{
    const char *name;
    nls_locstr *strings;
} nls_mapping;

static const nls_mapping LANGUAGES[] = {
    {"cs_CZ", STRINGS_CS_CZ},
    {"en_US", STRINGS_EN_US},
    {"pl_PL", STRINGS_PL_PL},
};

static const char *ENV[] = {"LC_ALL", "LC_MESSAGES", "LANG"};

static nls_locstr *
match_language(const char *name)
{
    int   i, length;
    char *end;

    if (NULL == name)
    {
        return NULL;
    }

    LOG("entry, name: %s", name);

    // matching with the sublanguage
    end = strchr(name, '.');
    length = (NULL == end) ? strlen(name) : (end - name);
    for (i = 0; i < lengthof(LANGUAGES); i++)
    {
        if (0 == strncasecmp(name, LANGUAGES[i].name, length) &&
            (0 == LANGUAGES[i].name[length]))
        {
            LOG("exit, matched %s", LANGUAGES[i].name);
            return LANGUAGES[i].strings;
        }
    }

    // matching only the language
    end = memchr(name, '_', length);
    length = (NULL == end) ? length : (end - name);
    for (i = 0; i < lengthof(LANGUAGES); i++)
    {
        if ((0 == strncasecmp(name, LANGUAGES[i].name, length)) &&
            ((0 == LANGUAGES[i].name[length]) ||
             ('_' == LANGUAGES[i].name[length])))
        {
            LOG("exit, matched %s", LANGUAGES[i].name);
            return LANGUAGES[i].strings;
        }
    }

    LOG("exit, no match!");
    return NULL;
}

static void
select_language(void)
{
    int i;

    if (NULL != STRINGS)
    {
        return;
    }

    LOG("entry");

    for (i = 0; i < lengthof(ENV); i++)
    {
        LOG("checking %s", ENV[i]);
        if (NULL != (STRINGS = match_language(getenv(ENV[i]))))
        {
            LOG("exit, matched");
            return;
        }
    }

    LOG("exit, falling back to US English");
    STRINGS = STRINGS_EN_US;
}
#endif

int
pal_load_string(unsigned id, char *buffer, int max_length)
{
    LOG("entry, id: %u, buffer: %p, max_length: %d", id, buffer, max_length);

    nls_locstr *it;
#ifndef CONFIG_COMPACT
    select_language();
#endif
    it = STRINGS;

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
