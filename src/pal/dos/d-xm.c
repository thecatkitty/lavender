#include <errno.h>

#include <arch/dos/xms.h>

static far void *pf_xms = NULL;

#define XMM_retA(fn, ax) "lcall *%1" : "=a"(ax) : "m"(pf_xms), "a"((fn) << 8)
#define XMM_reteA(fn, ax, err)                                                 \
    "lcall *%2" : "=a"(ax), "=Rbl"(err) : "m"(pf_xms), "a"((fn) << 8), "Rbl"(0)

static error_t
to_errno(uint8_t xmse)
{
    switch (xmse)
    {
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
}

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

long
dosxm_get_largest_free(void)
{
    uint8_t  err;
    uint16_t ax;
    asm volatile(XMM_reteA(XMSF_QUERYFREE, ax, err) : "d");
    return err ? -to_errno(err) : ax;
}
