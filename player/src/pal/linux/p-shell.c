#include <spawn.h>
#include <string.h>
#include <sys/wait.h>

#include <pal.h>

#define MAX_ARGC 3

static const char *const OPEN_COMMAND[][MAX_ARGC] = {
    {"xdg-open"},
    {"x-www-browser"},
    {"sensible-browser"},
    {"gio", "open"},
};

extern char **environ;

void
pal_open_url(const char *url)
{
    LOG("entry, '%s'", url);

    for (int i = 0; i < lengthof(OPEN_COMMAND); i++)
    {
        LOG("trying %s", OPEN_COMMAND[i][0]);

        char *argv[MAX_ARGC + 1] = {NULL};
        memcpy(argv, OPEN_COMMAND[i], sizeof(OPEN_COMMAND[0]));

        for (int arg = 0; arg < MAX_ARGC; arg++)
        {
            if (NULL == argv[arg])
            {
                argv[arg] = (char *)url;
                break;
            }
        }

        pid_t child;
        if (0 != posix_spawnp(&child, argv[0], NULL, NULL, argv, environ))
        {
            continue;
        }

        int   status;
        pid_t waited;

        do
        {
            waited = waitpid(child, &status, 0);
        } while ((-1 == waited) && (errno == EINTR));

        if ((waited == child) && WIFEXITED(status) &&
            (0 == WEXITSTATUS(status)))
        {
            LOG("exit");
            return;
        }
    }

    LOG("exit, failed");
}
