#include <sld.h>

int
sld_run_script(void *script, int size)
{
    const char *line = (const char *)script;
    int         length;
    sld_entry   entry;
    while (line < (const char *)script + size)
    {
        if (0 > (length = sld_load_entry(line, &entry)))
        {
            return length;
        }

        int status;
        if (0 > (status = sld_execute_entry(&entry)))
        {
            return status;
        }

        if (INT_MAX == status)
        {
            if (0 >
                (length = sld_find_label((const char *)script, script + size,
                                         entry.content, &line)))
            {
                return length;
            }
            continue;
        }

        line += length;
    }

    return 0;
}
