#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pal.h>
#include <sld.h>

#include "resource.h"

int
main(int argc, char *argv[])
{
    int status = EXIT_SUCCESS;

    pal_initialize(argc, argv);

    sld_context *script = sld_create_context("slides.txt", NULL);
    if (NULL == script)
    {
        char msg[80];
        pal_load_string(IDS_NOEXECCTX, msg, sizeof(msg));
        pal_alert(msg, errno);
        status = EXIT_FAILURE;
        goto end;
    }

    sld_run(script);
    sld_enter_context(script);
    while ((SLD_STATE_STOP != script->state) && (0 <= script->state))
    {
        sld_handle();
    }

    if ((0 > script->state) && (SLD_QUIT != script->state))
    {
        pal_alert(script->message, (SLD_SYSERR == script->state) ? errno : 0);
        status = EXIT_FAILURE;
    }

    sld_close_context(script);

end:
    pal_cleanup();

    return status;
}
