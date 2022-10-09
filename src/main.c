#include <pal.h>
#include <sld.h>

int
main(int argc, char *argv[])
{
    int status = 0;

    pal_initialize();

    sld_context *script = sld_create_context("slides.txt", NULL);
    if (NULL != script)
    {
        status = sld_run_script(script->data, script->size);
    }

    pal_cleanup(status);
    return status;
}
