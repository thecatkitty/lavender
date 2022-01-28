#include <api/bios.h>
#include <api/dos.h>
#include <ker.h>
#include <sld.h>
#include <vid.h>

static bool
IsEnvironmentCompatible(void);

int
Main(ZIP_CDIR_END_HEADER *zip)
{
    ZIP_LOCAL_FILE_HEADER *lfh;
    void *                 data;

    // Check compatibility and display a message on unsupported systems
    if (!IsEnvironmentCompatible())
    {
        const char support[] = "support.txt";
        if (0 > KerSearchArchive(zip, support, sizeof(support) - 1, &lfh))
        {
            DosPutS("Lavender cannot run in your environment.$");
            return 1;
        }

        if (0 > KerGetArchiveData(lfh, &data))
        {
            KerTerminate();
        }

        DosPutS((const char *)data);
        BiosKeyboardGetKeystroke();
        return 1;
    }

    // Locate slideshow description
    const char slides[] = "slides.txt";
    if (0 > KerSearchArchive(zip, slides, sizeof(slides) - 1, &lfh))
    {
        KerTerminate();
    }

    if (0 > KerGetArchiveData(lfh, &data))
    {
        KerTerminate();
    }

    // Set video mode and start the slideshow
    uint16_t oldMode = VidSetMode(VID_MODE_CGA_HIMONO);
    VidLoadFont();

    const char *line = (const char *)data;
    int         length;
    SLD_ENTRY   entry;
    while (line < (const char *)data + lfh->UncompressedSize)
    {
        if (0 > (length = SldLoadEntry(line, &entry)))
        {
            KerTerminate();
        }

        int status;
        if (0 > (status = SldExecuteEntry(&entry, zip)))
        {
            KerTerminate();
        }

        if (INT_MAX == status)
        {
            if (0 > (length = SldFindLabel((const char *)data,
                                           data + lfh->UncompressedSize,
                                           entry.Content, &line)))
            {
                KerTerminate();
            }
            continue;
        }

        line += length;
    }

    // Clean up
    VidUnloadFont();
    VidSetMode(oldMode);
    return 0;
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
