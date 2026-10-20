#ifndef _PAL_EVTMOUSE_H_
#define _PAL_EVTMOUSE_H_

#include <base.h>

extern void
evtmouse_press(uint16_t buttons);

extern void
evtmouse_release(uint16_t buttons);

extern void
evtmouse_set_position(uint16_t x, uint16_t y);

#endif // _PAL_EVTMOUSE_H_
