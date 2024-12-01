#include "encui.h"

encui_field *
encui_find_checkbox(encui_page *page)
{
    encui_field *checkbox = NULL;
    int          i;

    for (i = 0; i < page->cpx.length; i++)
    {
        encui_field *field = page->cpx.fields + i;
        if (ENCUIFT_CHECKBOX == field->type)
        {
            return field;
            break;
        }
    }

    return checkbox;
}

encui_prompt_page *
encui_find_prompt(encui_page *page)
{
    encui_prompt_page *prompt = &page->prompt;
    int                i;

    if (0 == page->message)
    {
        for (i = 0; i < page->cpx.length; i++)
        {
            encui_field *field = page->cpx.fields + i;
            if (ENCUIFT_TEXTBOX == field->type)
            {
                prompt = (encui_prompt_page *)field->data;
                break;
            }
        }
    }

    return prompt;
}
