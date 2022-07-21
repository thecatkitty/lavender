#include <string.h>

#include <sld.h>

int
sld_find_label(const char  *start,
               const char  *end,
               const char  *label,
               const char **line)
{
    sld_entry entry;
    int       length;

    *line = start;
    while (*line < end)
    {
        length = sld_load_entry(*line, &entry);
        if (0 > length)
        {
            return length;
        }

        *line += length;

        if (SLD_TYPE_LABEL != entry.type)
            continue;

        if (0 == strcmp(label, entry.content))
        {
            return 0;
        }
    }

    ERR(SLD_LABEL_NOT_FOUND);
}
