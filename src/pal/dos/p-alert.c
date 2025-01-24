#include <stdio.h>
#include <stdlib.h>

#include <gfx.h>
#include <pal.h>

void
pal_alert(const char *text, int error)
{
    char *texta = (char *)alloca(strlen(text + 1));
    utf8_encode(text, texta, pal_wctoa);

    puts("\n=====");
    puts(texta);
    if (0 != error)
    {
        char msg[GFX_COLUMNS] = "errno ";
        itoa(error, msg + strlen(msg), 10);
        puts(msg);
    }

    while (0 == pal_get_keystroke())
        ;
}
