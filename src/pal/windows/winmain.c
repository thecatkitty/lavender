#include "impl.h"

extern int
main(int argc, char *argv[]);

int       windows_cmd_show;
HINSTANCE windows_instance = NULL;

int WINAPI
wWinMain(HINSTANCE instance,
         HINSTANCE prevwindows_instance,
         PWSTR     cmd_line,
         int       cmd_show)
{
    int    argc = __argc, i, status;
    char **argv = (char **)malloc(((size_t)argc + 2) * sizeof(char *));
    if (NULL == argv)
    {
        return errno;
    }

    windows_instance = instance;
    windows_cmd_show = cmd_show;

    for (i = 0; i < argc; i++)
    {
        size_t length = WideCharToMultiByte(CP_UTF8, 0, __wargv[i], -1, NULL, 0,
                                            NULL, NULL);
        argv[i] = (char *)malloc(length + 1);
        if (NULL == argv[i])
        {
            argc = i;
            break;
        }
        WideCharToMultiByte(CP_UTF8, 0, __wargv[i], -1, argv[i], length + 1,
                            NULL, NULL);
    }
    argv[argc] = NULL;

    status = main(argc, argv);

    for (i = 0; i < argc; i++)
    {
        free(argv[i]);
    }
    free(argv);
    return status;
}
