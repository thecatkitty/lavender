#include <api/bios.h>
#include <api/dos.h>
#include <ker.h>
#include <pal.h>
#include <sld.h>
#include <vid.h>

static bool
IsEnvironmentCompatible(void);

int
main(int argc, char *argv[])
{
    char *data;
    int   status;

    pal_initialize();

    // Check compatibility and display a message on unsupported systems
    if (!IsEnvironmentCompatible())
    {
        hasset support = pal_open_asset("support.txt", O_RDONLY);
        if (NULL == support)
        {
            DosPutS("Lavender cannot run in your environment.$");
            return 1;
        }

        data = pal_get_asset_data(support);
        if (NULL == data)
        {
            return 1;
        }

        DosPutS(data);
        BiosKeyboardGetKeystroke();
        return 1;
    }

    // Locate slideshow description
    hasset slides = pal_open_asset("slides.txt", O_RDONLY);
    if (NULL == slides)
    {
        status = EXIT_ERRNO;
        goto cleanup;
    }

    // Get script
    data = pal_get_asset_data(slides);
    if (NULL == data)
    {
        status = EXIT_ERRNO;
        goto cleanup;
    }

    // Set video mode
    uint16_t oldMode = VidSetMode(VID_MODE_CGA_HIMONO);
    VidLoadFont();

    // Start the slideshow
    status = SldRunScript(data, pal_get_asset_size(slides));

    // Clean up
    VidUnloadFont();
    VidSetMode(oldMode);

cleanup:
    pal_cleanup(status);
    return status;
}

bool
IsEnvironmentCompatible(void)
{
    if (KerIsDosMajor(1))
    {
        // We're using DOS 2.0+ API here, so that's unfortunate.
        return false;
    }

    if (!KerIsWindowsNt())
    {
        return true;
    }

    return 0x0600 > KerGetWindowsNtVersion();
}
