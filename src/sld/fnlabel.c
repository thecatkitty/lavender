#include <string.h>

#include <sld.h>

int
SldFindLabel(const char * start,
             const char * end,
             const char * label,
             const char **line)
{
    SLD_ENTRY entry;
    int       length;

    *line = start;
    while (*line < end)
    {
        length = SldLoadEntry(*line, &entry);
        if (0 > length)
        {
            return length;
        }

        *line += length;

        if (SLD_TYPE_LABEL != entry.Type)
            continue;

        if (0 == strcmp(label, entry.Content))
        {
            return 0;
        }
    }

    ERR(SLD_LABEL_NOT_FOUND);
}
