#include <sld.h>

int
SldRunScript(void *script, int size, ZIP_CDIR_END_HEADER *zip)
{
    const char *line = (const char *)script;
    int         length;
    SLD_ENTRY   entry;
    while (line < (const char *)script + size)
    {
        if (0 > (length = SldLoadEntry(line, &entry)))
        {
            return length;
        }

        int status;
        if (0 > (status = SldExecuteEntry(&entry, zip)))
        {
            return status;
        }

        if (INT_MAX == status)
        {
            if (0 > (length = SldFindLabel((const char *)script, script + size,
                                           entry.Content, &line)))
            {
                return length;
            }
            continue;
        }

        line += length;
    }

    return 0;
}
