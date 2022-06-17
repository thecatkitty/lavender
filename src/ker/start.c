#include <pal.h>

extern int
Main();

int
main(int argc, char *argv[])
{
    pal_initialize();

    int status = Main();

    pal_cleanup(status);
    return status;
}
