#ifndef _ARD_ACTION_H_
#define _ARD_ACTION_H_

#include <sal.h>
#include <stdbool.h>

#include <ard/config.h>

extern int
arda_exec(_In_ const ardc_config *cfg, _In_z_ const char *path);

extern int
arda_run(_In_ const ardc_config *cfg);

#endif // _ARD_ACTION_H_
