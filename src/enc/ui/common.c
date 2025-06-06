#include "encui.h"

static encui_field *
_find_field_by_type(encui_page *page, int type)
{
    int i;
    for (i = 0; i < page->length; i++)
    {
        if (type == page->fields[i].type)
        {
            return page->fields + i;
        }
    }

    return NULL;
}

encui_field *
encui_find_checkbox(encui_page *page)
{
    return _find_field_by_type(page, ENCUIFT_CHECKBOX);
}

encui_textbox_data *
encui_find_textbox(encui_page *page)
{
    encui_field *field = _find_field_by_type(page, ENCUIFT_TEXTBOX);
    return field ? (encui_textbox_data *)field->data : NULL;
}
