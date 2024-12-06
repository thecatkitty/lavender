#include "encui.h"

encui_field *
encui_find_checkbox(encui_page *page)
{
    encui_field *checkbox = NULL;
    int          i;

    for (i = 0; i < page->length; i++)
    {
        encui_field *field = page->fields + i;
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
    encui_prompt_page *prompt = NULL;
    int                i;

    for (i = 0; i < page->length; i++)
    {
        encui_field *field = page->fields + i;
        if (ENCUIFT_TEXTBOX == field->type)
        {
            prompt = (encui_prompt_page *)field->data;
            break;
        }
    }

    return prompt;
}
