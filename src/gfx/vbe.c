#include <api/bios.h>
#include <gfx.h>

#define _par(dx, dy, sx, sy)                                                   \
    (uint8_t)(64U * (unsigned)dy * (unsigned)sx / (unsigned)dx / (unsigned)sy)

static bool
_read_edid(edid_block *edid)
{
    short ax;

    ax = bios_get_vbedc_capabilities();
    if (0x4F != (ax & 0xFF))
    {
        return false;
    }

    if (0 != (ax & 0xFF00))
    {
        return false;
    }

    ax = bios_read_edid(edid);
    if (0x4F != (ax & 0xFF))
    {
        return false;
    }

    if (0 != (ax & 0xFF00))
    {
        return false;
    }

    return true;
}

uint16_t
gfx_get_pixel_aspect(void)
{
    gfx_dimensions screen;
    gfx_get_screen_dimensions(&screen);

    uint8_t ratios[4] = {_par(16, 10, screen.width, screen.height),
                         _par(4, 3, screen.width, screen.height),
                         _par(5, 4, screen.width, screen.height),
                         _par(16, 9, screen.width, screen.height)};

    edid_block edid;
    if (!_read_edid(&edid))
    {
        return ratios[EDID_TIMING_ASPECT_4_3];
    }

    return ratios[edid.standard_timing[0] >> EDID_TIMING_ASPECT];
}
