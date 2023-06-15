#ifndef _DLG_H_
#define _DLG_H_

#include <base.h>

enum dlg_result
{
    DLG_OK = 1,
    DLG_CANCEL = 2,
    DLG_ABORT = 3,
    DLG_RETRY = 4,
    DLG_IGNORE = 5,
    DLG_YES = 6,
    DLG_NO = 7,
    DLG_TRY_AGAIN = 10,
    DLG_CONTINUE = 11,
    DLG_INCOMPLETE = -1
};

typedef bool (*dlg_validator)(const char *);

extern int
dlg_alert(const char *title, const char *message);

extern int
dlg_prompt(const char   *title,
           const char   *message,
           char         *buffer,
           int           size,
           dlg_validator validator);

#endif // _DLG_H_
