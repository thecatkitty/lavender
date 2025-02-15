#include <string.h>
#include <windows.h>

#include <ard/config.h>
#include <ard/ui.h>

static char title_[ARDC_LENGTH_MID] = "";

int
ardui_msgbox(_In_z_ const char *str, _In_ unsigned style)
{
    return MessageBox(NULL, str, title_, style);
}

_Ret_z_ const char *
ardui_get_title(void)
{
    return title_;
}

void
ardui_set_title(_In_z_ const char *str)
{
    strcpy(title_, str);
}
