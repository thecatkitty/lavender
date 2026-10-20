#include <stdlib.h>
#include <unistd.h>

#include <andrea.h>

#include <arch/dos.h>
#include <pal.h>

#define MAX_DRIVERS 4

typedef int ddcall (*pf_drvinit)(void);
typedef int ddcall (*pf_drvdeinit)(void);

uint16_t
dos_load_driver(const char *name)
{
    char path[_MAX_PATH];
    if (0 > pal_extract_asset(name, path))
    {
        return 0;
    }

    andrea_module module = andrea_load(path);
    if (0 == module)
    {
        return 0;
    }

    pf_drvinit drv_init = (pf_drvinit)andrea_get_procedure(module, "drv_init");
    if (NULL == drv_init)
    {
        return 0;
    }

    if (0 > drv_init())
    {
        andrea_free(module);
        return 0;
    }

    unlink(path);
    return module;
}

void
dos_unload_driver(uint16_t driver)
{
    pf_drvdeinit deinit =
        (pf_drvdeinit)andrea_get_procedure(driver, "drv_deinit");
    if (NULL != deinit)
    {
        deinit();
    }

    andrea_free(driver);
}
