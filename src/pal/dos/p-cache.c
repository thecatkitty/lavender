#include <dos.h>
#include <unistd.h>

#include <pal.h>

hcache
pal_cache(int fd, off_t at, size_t size)
{
    unsigned segment;
    if (0 != _dos_allocmem((size + 15) >> 4, &segment))
    {
        return -ENOMEM;
    }

    char   buff[512];
    size_t pos = 0;
    lseek(fd, at, SEEK_SET);

    size_t head = sizeof(buff) - (at % sizeof(buff));
    if (head != read(fd, buff, head))
    {
        return -errno;
    }
    _fmemcpy(MK_FP(segment, pos), buff, head);
    pos += head;

    while ((pos + sizeof(buff)) < size)
    {
        if (sizeof(buff) != read(fd, buff, sizeof(buff)))
        {
            return -errno;
        }
        _fmemcpy(MK_FP(segment, pos), buff, sizeof(buff));
        pos += sizeof(buff);
    }

    if (pos < size)
    {
        if (size - pos != read(fd, buff, size - pos))
        {
            return -errno;
        }
        _fmemcpy(MK_FP(segment, pos), buff, size - pos);
    }

    return segment;
}

void
pal_discard(hcache handle)
{
    _dos_freemem(handle);
}

void
pal_read(hcache handle, char *buff, off_t at, size_t size)
{
    _fmemcpy(buff, MK_FP(handle, at), size);
}
