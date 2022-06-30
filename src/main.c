#include <pal.h>
#include <sld.h>
#include <vid.h>

int
main(int argc, char *argv[])
{
    int status;

    pal_initialize();

    // Locate slideshow description
    hasset slides = pal_open_asset("slides.txt", O_RDONLY);
    if (NULL == slides)
    {
        status = EXIT_ERRNO;
        goto cleanup;
    }

    // Get script
    char *data = pal_get_asset_data(slides);
    if (NULL == data)
    {
        status = EXIT_ERRNO;
        goto cleanup;
    }

    // Initialize video
    VidInitialize();

    // Start the slideshow
    status = SldRunScript(data, pal_get_asset_size(slides));

    // Clean up
    VidCleanup();

cleanup:
    pal_cleanup(status);
    return status;
}
