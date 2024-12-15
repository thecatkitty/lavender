#ifndef _UI_DIRECT_H_
#define _UI_DIRECT_H_

#include <gfx.h>

#include "../encui.h"

#define TEXT_WIDTH (GFX_COLUMNS - 2)

int
encui_direct_print(int top, char *text);

void
encui_direct_init_frame(void);

void
encui_direct_enter_page(encui_page *pages, int id);

int
encui_direct_click(uint16_t x, uint16_t y);

int
encui_direct_key(uint16_t scancode);

bool
encui_direct_animate(bool valid);

void
encui_direct_set_error(char *message);

#endif
