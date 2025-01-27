#include <assert.h>
#include <io.h>
#include <windows.h>

#include <pal.h>

#define MAX_CACHE_FILES 8
#define MAX_CACHE_ITEMS 8

typedef struct
{
    int    fd;
    HANDLE file;
    HANDLE mapping;
    int    count;
} cache_file;

typedef struct
{
    int    fd;
    off_t  base;
    size_t size;
    void  *addr;
} cache_item;

static cache_file files_[MAX_CACHE_FILES];
static cache_item cache_[MAX_CACHE_ITEMS];

static int
acquire(int fd)
{
    int i = 0;

    while ((MAX_CACHE_FILES > i) && (0 != files_[i].fd) && (fd != files_[i].fd))
    {
        i++;
    }

    if (MAX_CACHE_FILES == i)
    {
        return -ENOMEM;
    }

    if (files_[i].fd != fd)
    {
        HANDLE file = (HANDLE)_get_osfhandle(fd);
        HANDLE mapping =
            CreateFileMappingW(file, NULL, PAGE_READONLY, 0, 0, NULL);
        if (NULL == mapping)
        {
            return (hcache)-GetLastError();
        }

        files_[i].fd = fd;
        files_[i].file = file;
        files_[i].mapping = mapping;
        files_[i].count = 0;
    }

    files_[i].count++;
    return i;
}

static void
release(int fd)
{
    int i = 0;

    while ((MAX_CACHE_FILES > i) && (fd != files_[i].fd))
    {
        i++;
    }

    if (MAX_CACHE_FILES == i)
    {
        return;
    }

    files_[i].count--;
    if (0 == files_[i].count)
    {
        CloseHandle(files_[i].mapping);
        memset(&files_[i], 0, sizeof(files_[i]));
    }
}

hcache
pal_cache(int fd, off_t at, size_t size)
{
    int            file, i = 0;
    SYSTEM_INFO    info;
    off_t          aligned_at, aligned_size, base;
    ULARGE_INTEGER offset = {0};

    while ((MAX_CACHE_ITEMS > i) && (0 != cache_[i].fd))
    {
        i++;
    }

    if (MAX_CACHE_ITEMS == i)
    {
        return -ENOMEM;
    }

    file = acquire(fd);
    if (0 > file)
    {
        return file;
    }

    GetSystemInfo(&info);
    aligned_at =
        at / info.dwAllocationGranularity * info.dwAllocationGranularity;
    base = at % info.dwAllocationGranularity;
    aligned_size = base + size;

    offset.QuadPart = aligned_at;
    cache_[i].addr =
        MapViewOfFile(files_[file].mapping, FILE_MAP_READ, offset.HighPart,
                      offset.LowPart, aligned_size);
    if (NULL == cache_[i].addr)
    {
        release(fd);
        return (hcache)-GetLastError();
    }

    cache_[i].fd = fd;
    cache_[i].base = base;
    cache_[i].size = aligned_size;

    i++;
    return i;
}

void
pal_discard(hcache handle)
{
    cache_item *cache = cache_ + handle - 1;
    assert(UnmapViewOfFile(cache->addr));
    release(cache->fd);
    memset(cache, 0, sizeof(*cache));
}

void
pal_read(hcache handle, char *buff, off_t at, size_t size)
{
    cache_item *cache = cache_ + handle - 1;
    assert((0 < handle) && (MAX_CACHE_ITEMS >= handle));
    memcpy(buff, (char *)cache->addr + cache->base + at, size);
}
