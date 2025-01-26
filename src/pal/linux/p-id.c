#include <unistd.h>

#include <pal.h>

bool
pal_get_machine_id(uint8_t *mid)
{
    LOG("entry");
    int fd = 0;

    if (0 > (fd = open("/var/lib/dbus/machine-id", O_RDONLY)))
    {
        LOG("exit, cannot open the file!");
        return false;
    }

    char buffer[PAL_MACHINE_ID_SIZE * 2];
    int  size = read(fd, buffer, sizeof(buffer));
    close(fd);

    if (sizeof(buffer) != size)
    {
        LOG("exit, cannot read the file!");
        return false;
    }

    if (NULL == mid)
    {
        LOG("exit, capable");
        return true;
    }

    for (int i = 0; i < PAL_MACHINE_ID_SIZE; i++)
    {
        mid[i] = xtob(buffer + (i * 2));
    }

    LOG("exit, "
        "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",
        mid[0] & 0xFF, mid[1] & 0xFF, mid[2] & 0xFF, mid[3] & 0xFF,
        mid[4] & 0xFF, mid[5] & 0xFF, mid[6] & 0xFF, mid[7] & 0xFF,
        mid[8] & 0xFF, mid[9] & 0xFF, mid[10] & 0xFF, mid[11] & 0xFF,
        mid[12] & 0xFF, mid[13] & 0xFF, mid[14] & 0xFF, mid[15] & 0xFF);
    return true;
}
