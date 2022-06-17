#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <api/dos.h>
#include <ker.h>
#include <pal.h>

extern int
Main();

extern const char StrKerError[];

int
main(int argc, char *argv[])
{
    pal_initialize();

    int status = Main();

    pal_cleanup();

    if (0 > status)
    {
        KerTerminate(-status);
    }

    if (EXIT_ERRNO == status)
    {
        DosPutS(StrKerError);
        
        char code[10];
        itoa(errno, code, 10);
        code[strlen(code)] = '$';
        DosPutS(code);
    }

    return status;
}
