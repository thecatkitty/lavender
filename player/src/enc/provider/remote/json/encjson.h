#ifndef _ENCJSON_H_
#define _ENCJSON_H_

#include <base.h>

extern const char *
encjson_get_child_string(const char *str,
                         size_t      str_len,
                         const char *key,
                         size_t     *child_len);

#endif // _ENCJSON_H_
