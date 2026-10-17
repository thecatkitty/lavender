#include <stdio.h>
#include <string.h>

#include <pal.h>

void
pal_alert(const char *text, int error)
{
    puts("\n=====");
    puts(text);
    if (0 != error)
    {
        printf("errno %d, %s\n", error, strerror(error));
    }
}
