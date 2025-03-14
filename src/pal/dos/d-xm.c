#include <errno.h>

#include <arch/dos.h>
#include <arch/dos/xms.h>

static far void *pf_xms = NULL;

#define XMM_retA(fn, ax) "lcall *%1" : "=a"(ax) : "m"(pf_xms), "a"((fn) << 8)
#define XMM_retABD(fn, ax, bx, dx)                                             \
    "lcall *%3" : "=a"(ax),                                                    \
                  "=b"(bx),                                                    \
                  "=d"(dx)                                                     \
        : "m"(pf_xms), "a"((fn) << 8), "b"(0), "d"(0)
#define XMM_argD_retAB(fn, ax, bx, dx)                                         \
    "lcall *%2" : "=a"(ax),                                                    \
                  "=b"(bx)                                                     \
        : "m"(pf_xms), "a"((fn) << 8), "b"(0), "d"(dx)
#define XMM_argS_retAB(fn, ax, bx, si)                                         \
    "lcall *%2" : "=a"(ax),                                                    \
                  "=b"(bx)                                                     \
        : "m"(pf_xms), "a"((fn) << 8), "b"(0), "S"(si)
#define XMM_argD_retABD(fn, ax, bx, dx)                                        \
    "lcall *%3" : "=a"(ax),                                                    \
                  "=b"(bx),                                                    \
                  "=d"(dx)                                                     \
        : "m"(pf_xms), "a"((fn) << 8), "b"(0), "d"(dx)

static error_t
to_errno(uint8_t xmse)
{
    switch (xmse)
    {
    case 0:
        return 0;
    case XMSE_NOTIMPLEMENTED:
        return ENOSYS;
    case XMSE_VDISKPRESENT:
    case XMSE_BADA20:
    case XMSE_PARITY:
    case XMSE_BADLOCK:
        return EIO;
    case XMSE_OUTOFMEMORY:
    case XMSE_OUTOFHANDLES:
        return ENOMEM;
    case XMSE_BADHANDLE:
    case XMSE_BADSRC:
    case XMSE_BADDST:
        return EBADF;
    case XMSE_BADSRCOFF:
    case XMSE_BADDSTOFF:
    case XMSE_BADLENGTH:
    case XMSE_BADOVERLAP:
        return EFAULT;
    case XMSE_NOTLOCKED:
    case XMSE_LOCKED:
        return EINVAL;
    case XMSE_TOOMANYLOCKS:
        return EOVERFLOW;
    }

    return ENOENT;
}

#define FALSE_IF_ERROR(err)   (errno = to_errno((err)), !(err))
#define ZERO_IF_ERROR(x, err) (errno = to_errno((err)), (err) ? 0 : (x))

uint16_t
dosxm_init(void)
{
    if (XMS_PRESENT != xms_detect())
    {
        return 0;
    }

    pf_xms = xms_get_entry_point();

    uint16_t ax;
    asm volatile(XMM_retA(XMSF_VERSION, ax) : "b", "d");
    return ax;
}

uint16_t
dosxm_getmaxfree(void)
{
    uint16_t ax, bx, __unused dx = 0;
    asm volatile(XMM_retABD(XMSF_QUERYFREE, ax, bx, dx));
    return ZERO_IF_ERROR(ax, bx & 0xFF);
}

hdosxm
dosxm_alloc(uint16_t size_kb)
{
    uint16_t ax, bx, dx = size_kb;
    asm volatile(XMM_argD_retABD(XMSF_ALLOC, ax, bx, dx));
    bx = ax ? 0 : bx;
    return (hdosxm)(ZERO_IF_ERROR(dx, bx & 0xFF));
}

bool
dosxm_free(hdosxm block)
{
    uint16_t ax, bx, dx = (uint16_t)block;
    asm volatile(XMM_argD_retAB(XMSF_FREE, ax, bx, dx));
    bx = ax ? 0 : bx;
    return FALSE_IF_ERROR(bx & 0xFF);
}

static bool
xmemmove(const xms_move_args *args)
{
    // SS = DS
    uint16_t ax, bx, si = (uint16_t)args;
    asm volatile(XMM_argS_retAB(XMSF_MOVE, ax, bx, si));
    bx = ax ? 0 : bx;
    return FALSE_IF_ERROR(bx & 0xFF);
}

bool
dosxm_load(far void *dst, hdosxm src, uint32_t offset, uint32_t length)
{
    xms_move_args args = {length, (uint16_t)src, offset, 0, (uint32_t)dst};
    return xmemmove(&args);
}

bool
dosxm_store(hdosxm dst, uint32_t offset, far void *src, uint32_t length)
{
    xms_move_args args = {length, 0, (uint32_t)src, (uint16_t)dst, offset};
    return xmemmove(&args);
}
