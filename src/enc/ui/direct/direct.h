#ifndef _UI_DIRECT_H_
#define _UI_DIRECT_H_

#include <gfx.h>

#include "../encui.h"

#define TEXT_WIDTH (GFX_COLUMNS - 2)

int
encui_direct_print(int top, char *text);

#endif
