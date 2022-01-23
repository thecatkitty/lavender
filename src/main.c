#include <limits.h>

#include <ker.h>
#include <sld.h>
#include <vid.h>

extern char __edata[], __sbss[];

const char SLIDES_TXT[] = "slides.txt";

int
Main(void)
{
    uint16_t oldMode = VidSetMode(VID_MODE_CGA_HIMONO);
    VidLoadFont();

    ZIP_CDIR_END_HEADER *cdir;
    if (0 > KerLocateArchive(__edata, __sbss, &cdir))
    {
        KerTerminate();
    }

    ZIP_LOCAL_FILE_HEADER *lfh;
    if (0 > KerSearchArchive(cdir, SLIDES_TXT, sizeof(SLIDES_TXT) - 1, &lfh))
    {
        KerTerminate();
    }

    void *data;
    if (0 > KerGetArchiveData(lfh, &data))
    {
        KerTerminate();
    }

    const char *line = (const char *)data;
    int         length;
    SLD_ENTRY   entry;
    while (0 < (length = SldLoadEntry(line, &entry)))
    {
        int status;
        if (0 > (status = SldExecuteEntry(&entry, cdir)))
        {
            KerTerminate();
        }

        if (INT_MAX == status)
        {
            if (0 > (length = SldFindLabel((const char *)data, entry.Content, &line)))
            {
                KerTerminate();
            }
            continue;
        }

        line += length;
    }

    if (0 > length)
    {
        KerTerminate();
    }

    VidUnloadFont();
    VidSetMode(oldMode);
    return 0;
}
