#include <assert.h>
#include <sys/mman.h>
#include <unistd.h>

#include <pal.h>

#define MAX_CACHE 8

typedef struct
{
    int    fd;
    off_t  base;
    size_t size;
    void  *addr;
} cache_item;

static cache_item cache_[MAX_CACHE];

hcache
pal_cache(int fd, off_t at, size_t size)
{
    LOG("entry, fd: %d, %zu@%#lx", fd, size, (unsigned long)at);

    long i = 0;
    while ((MAX_CACHE > i) && (0 != cache_[i].fd))
    {
        i++;
    }

    if (MAX_CACHE == i)
    {
        LOG("exit, no slots");
        return -ENOMEM;
    }

    long  page = sysconf(_SC_PAGE_SIZE);
    off_t aligned_at = at / page * page;
    off_t base = at % page;
    off_t aligned_size = base + size;
    LOG("aligned %zu@%#lx", (size_t)aligned_size, (unsigned long)aligned_at);

    cache_[i].addr =
        mmap(NULL, aligned_size, PROT_READ, MAP_PRIVATE, fd, aligned_at);
    if (MAP_FAILED == cache_[i].addr)
    {
        LOG("exit, mmap failed with status %d", errno);
        return -errno;
    }

    cache_[i].fd = fd;
    cache_[i].base = base;
    cache_[i].size = aligned_size;

    i++;
    LOG("exit, handle: %ld", i);
    return i;
}

void
pal_discard(hcache handle)
{
    LOG("handle: %ld", handle);

    cache_item *cache = cache_ + handle - 1;
    assert(0 == munmap(cache->addr, cache->size));
}

void
pal_read(hcache handle, char *buff, off_t at, size_t size)
{
    LOG("handle: %ld, %zu@%#lx", handle, size, at);

    assert((0 < handle) && (MAX_CACHE >= handle));
    cache_item *cache = cache_ + handle - 1;
    memcpy(buff, (char *)cache->addr + cache->base + at, size);
}
