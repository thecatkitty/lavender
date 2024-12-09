#ifndef _UI_DIRECT_H_
#define _UI_DIRECT_H_

#include <gfx.h>

#include "../encui.h"

#define TEXT_WIDTH (GFX_COLUMNS - 2)

int
encui_direct_print(int top, char *text);

void
encui_direct_create_checkbox(encui_field *field, char *buffer, size_t size);

const gfx_rect *
encui_direct_get_checkbox_area(void);

void
encui_direct_click_checkbox(encui_field *field);

#endif
