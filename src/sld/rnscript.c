#include <sld.h>

int
SldRunScript(ZIP_LOCAL_FILE_HEADER *file, ZIP_CDIR_END_HEADER *zip)
{
    int   status;
    void *data;
    if (0 > (status = KerGetArchiveData(file, &data)))
    {
        return status;
    }

    const char *line = (const char *)data;
    int         length;
    SLD_ENTRY   entry;
    while (line < (const char *)data + file->UncompressedSize)
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
            if (0 > (length = SldFindLabel((const char *)data,
                                           data + file->UncompressedSize,
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
