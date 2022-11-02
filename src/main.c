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
    int status = EXIT_SUCCESS;

    pal_initialize(argc, argv);

    sld_context *script = sld_create_context("slides.txt", NULL);
    if (NULL == script)
    {
        puterrno();
        status = EXIT_FAILURE;
        goto end;
    }

    sld_run(script);
    sld_enter_context(script);
    while ((SLD_STATE_STOP != script->state) && (0 <= script->state))
    {
        sld_handle();
    }

    if (0 > script->state)
    {
        status = EXIT_FAILURE;
        puts("\n=====");
        puts(script->message);
        if (SLD_SYSERR == script->state)
        {
            puterrno();
        }
    }

    sld_close_context(script);

end:
    if (EXIT_FAILURE == status)
    {
        while (!pal_get_keystroke())
            ;
    }

    pal_cleanup();

    return status;
}
