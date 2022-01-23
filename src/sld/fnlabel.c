#include <api/dos.h>
#include <string.h>

#include <sld.h>

int
SldFindLabel(const char *start, const char *label, const char **line)
{
    SLD_ENTRY entry;
    int       length;

    *line = start;
    while (0 < (length = SldLoadEntry(*line, &entry)))
    {
        *line += length;

        if (SLD_TYPE_LABEL != entry.Type)
            continue;

        if (0 == strcmp(label, entry.Content))
        {
            return 0;
        }
    }

    if (0 > length)
    {
        return length;
    }

    ERR(SLD_LABEL_NOT_FOUND);
}
