#include <api/bios.h>
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
        if (SLD_TYPE_JUMP == entry.Type)
        {
            if (0 > (length = SldFindLabel((const char *)data, entry.Content, &line)))
            {
                KerTerminate();
            }
            continue;
        }

        if (0 > SldExecuteEntry(&entry, cdir))
        {
            KerTerminate();
        }

        line += length;
    }

    if (0 > length)
    {
        KerTerminate();
    }

    BiosKeyboardGetKeystroke();
    VidUnloadFont();
    VidSetMode(oldMode);
    return 0;
}
