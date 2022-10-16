#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pal.h>
#include <sld.h>

void
puterrno(void)
{
    char msg[80] = "errno ";
    itoa(errno, msg + strlen(msg), 10);
    puts(msg);
}

int
main(int argc, char *argv[])
{
    int status = 0;

    pal_initialize();

    sld_context *script = sld_create_context("slides.txt", NULL);
    if (NULL != script)
    {
        status = sld_run_script(script);
        sld_close_context(script);
    }

    pal_cleanup();

    if (0 > status)
    {
        puts(script->message);
        if (SLD_SYSERR == status)
        {
            puterrno();
        }
    }

    return status;
}
