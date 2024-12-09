#include <algorithm>

extern "C"
{
#include "direct.h"
}

encui_field *
find_field(encui_page *page, int type)
{
    auto field = std::find_if(page->fields, page->fields + page->length,
                              [=](encui_field &field) {
                                  return type == field.type;
                              });
    return field == page->fields + page->length ? NULL : field;
}
