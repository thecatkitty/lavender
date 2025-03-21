#include <dos.h>
#include <unistd.h>

#include <arch/dos.h>
#include <pal.h>

#include "impl.h"

typedef unsigned cacheblk;

#ifdef CONFIG_IA16X
typedef uint64_t cacheptr;

static inline cacheptr
MK_XP(hdosxm hnd, uint32_t off)
{
    uint32_t xp[2] = {off, (uintptr_t)hnd};
    return *(cacheptr *)xp;
}

#define MK_HDOSXM(hnd)        ((hdosxm)(uintptr_t)(hnd))
#define MK_CACHEBLK(hnd)      ((cacheptr)(uintptr_t)(hnd))
#define MK_CACHEPTR(hnd, off) MK_XP(MK_HDOSXM(hnd), (off))

#define XP_HND(xp) MK_HDOSXM(((const uint32_t *)(&(xp)))[1])
#define XP_OFF(xp) ((uint32_t)(xp))

#define MAX_CACHE_ITEMS 16

typedef struct
{
    int      fd;
    off_t    base;
    size_t   size;
    cacheblk handle;
} cache_item;

static cache_item cache_[MAX_CACHE_ITEMS];
#else
typedef far void *cacheptr;

#define MK_CACHEPTR(hnd, off) MK_FP((cacheblk)(hnd), (off))
#endif

static cacheblk
retrieve_cache(int fd, off_t at, size_t size)
{
#ifdef CONFIG_IA16X
    for (int i = 0; i < MAX_CACHE_ITEMS; i++)
    {
        if ((fd == cache_[i].fd) && (at == cache_[i].base) &&
            (size == cache_[i].size))
        {
            return cache_[i].handle;
        }
    }
#endif

    return 0;
}

#ifdef CONFIG_IA16X
static cache_item *
find_cache(cacheblk handle)
{
    for (int i = 0; i < MAX_CACHE_ITEMS; i++)
    {
        if (handle == cache_[i].handle)
        {
            return cache_ + i;
        }
    }

    return NULL;
}

static bool
remember_cache(int fd, off_t at, size_t size, cacheblk handle)
{
    cache_item *slot = find_cache(0);
    if (NULL == slot)
    {
        return false;
    }

    slot->fd = fd;
    slot->base = at;
    slot->size = size;
    slot->handle = handle;
    return true;
}
#endif

static cacheblk
allocate_cache(size_t length)
{
#ifdef CONFIG_IA16X
    return MK_CACHEBLK(dosxm_alloc((length + 1023) / 1024));
#else
    unsigned segment;
    if (0 != _dos_allocmem((length + 15) / 16, &segment))
    {
        return 0;
    }
    return segment;
#endif
}

static bool
free_cache(cacheblk handle)
{
#ifdef CONFIG_IA16X
    return dosxm_free(MK_HDOSXM(handle));
#else
    _dos_freemem(handle);
    return true;
#endif
}

static bool
load_cache(void *dst, cacheptr src, size_t length)
{
#ifdef CONFIG_IA16X
    return dosxm_load(dst, XP_HND(src), XP_OFF(src), length);
#else
    _fmemcpy(dst, src, length);
    return true;
#endif
}

static bool
store_cache(cacheptr dst, void *src, size_t length)
{
#ifdef CONFIG_IA16X
    return dosxm_store(XP_HND(dst), XP_OFF(dst), src, length);
#else
    _fmemcpy(dst, src, length);
    return true;
#endif
}

static int
load_data(int fd, off_t at, size_t size, cacheptr output)
{
    char   buff[512];
    size_t pos = 0;
    lseek(fd, at, SEEK_SET);

    size_t head = sizeof(buff) - (at % sizeof(buff));
    if (head != read(fd, buff, head))
    {
        return -errno;
    }
    store_cache(output + pos, buff, head);
    pos += head;

    while ((pos + sizeof(buff)) < size)
    {
        if (sizeof(buff) != read(fd, buff, sizeof(buff)))
        {
            return -errno;
        }
        store_cache(output + pos, buff, sizeof(buff));
        pos += sizeof(buff);
    }

    if (pos < size)
    {
        if (size - pos != read(fd, buff, size - pos))
        {
            return -errno;
        }
        store_cache(output + pos, buff, size - pos);
    }

    return 0;
}

hcache
pal_cache(int fd, off_t at, size_t size)
{
    cacheblk block = retrieve_cache(fd, at, size);
    if (0 == block)
    {
        block = allocate_cache(size);
        if (0 == block)
        {
            return 0;
        }

#ifdef CONFIG_IA16X
        if (!remember_cache(fd, at, size, block))
        {
            free_cache(block);
            return 0;
        }
#endif
    }

    if (0 != load_data(fd, at, size, MK_CACHEPTR(block, 0)))
    {
        free_cache(block);
        return 0;
    }

    return block;
}

void
pal_discard(hcache handle)
{
#ifndef CONFIG_IA16X
    free_cache((cacheblk)handle);
#endif
}

void
pal_read(hcache handle, char *buff, off_t at, size_t size)
{
    load_cache(buff, MK_CACHEPTR(handle, at), size);
}

#ifdef CONFIG_IA16X
void
dos_initialize_cache(void)
{
    memset(cache_, 0, sizeof(cache_));
}

void
dos_cleanup_cache(void)
{
    for (int i = 0; i < MAX_CACHE_ITEMS; i++)
    {
        if (cache_[i].handle)
        {
            free_cache(cache_[i].handle);
        }
    }
}
#endif
