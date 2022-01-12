#include <api/bios.h>
#include <vid.h>

#define VID_PAR(dx, dy, sx, sy)                                                \
    (uint8_t)(64U * (unsigned)dy * (unsigned)sx / (unsigned)dx / (unsigned)sy)

static int
VesaReadEdid(EDID *edid);

uint16_t
VidGetPixelAspectRatio(void)
{
    const uint8_t ratios[4] = {
        VID_PAR(16, 10, VID_CGA_HIMONO_WIDTH, VID_CGA_HIMONO_HEIGHT),
        VID_PAR(4, 3, VID_CGA_HIMONO_WIDTH, VID_CGA_HIMONO_HEIGHT),
        VID_PAR(5, 4, VID_CGA_HIMONO_WIDTH, VID_CGA_HIMONO_HEIGHT),
        VID_PAR(16, 9, VID_CGA_HIMONO_WIDTH, VID_CGA_HIMONO_HEIGHT)};

    EDID edid;
    if (0 > VesaReadEdid(&edid))
    {
        return ratios[EDID_TIMING_ASPECT_4_3];
    }

    return ratios[edid.StandardTiming[0] >> EDID_TIMING_ASPECT];
}

int
VesaReadEdid(EDID *edid)
{
    short ax;

    ax = BiosVideoVbeDcCapabilities();
    if (0x4F != (ax & 0xFF))
    {
        ERR(VID_UNSUPPORTED);
    }

    if (0 != (ax & 0xFF00))
    {
        ERR(VID_FAILED);
    }

    ax = BiosVideoVbeDcReadEdid(edid);
    if (0x4F != (ax & 0xFF))
    {
        ERR(VID_UNSUPPORTED);
    }

    if (0 != (ax & 0xFF00))
    {
        ERR(VID_UNSUPPORTED);
    }

    return 0;
}
