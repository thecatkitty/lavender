#include <stdio.h>

#include <pal.h>

static const char *OPEN_COMMAND[] = {
    "xdg-open",
    "x-www-browser",
    "sensible-browser",
    "gio open",
};

void
pal_open_url(const char *url)
{
    LOG("entry, '%s'", url);

    char cmd[PATH_MAX];

    for (int i = 0; i < lengthof(OPEN_COMMAND); i++)
    {
        LOG("trying %s", OPEN_COMMAND[i]);
        snprintf(cmd, PATH_MAX, "%s \"%s\"", OPEN_COMMAND[i], url);
        if (0 == system(cmd))
        {
            LOG("exit");
            return;
        }
    }

    LOG("exit, failed");
}
