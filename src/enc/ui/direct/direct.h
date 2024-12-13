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

void
encui_direct_create_textbox(encui_prompt_page *prompt,
                            encui_page        *page,
                            int               *cy);

void
encui_direct_draw_textbox(encui_prompt_page *prompt, encui_page *page);

bool
encui_direct_animate_textbox(encui_prompt_page *prompt,
                             encui_page        *page,
                             bool               valid);

void
encui_direct_set_textbox_error(char *message);

const gfx_rect *
encui_direct_get_textbox_area(void);

void
encui_direct_click_textbox(encui_prompt_page *prompt,
                           encui_page        *page,
                           uint16_t           x,
                           uint16_t           y);

void
encui_direct_key_textbox(encui_prompt_page *prompt,
                         encui_page        *page,
                         uint16_t           scancode);

#endif
