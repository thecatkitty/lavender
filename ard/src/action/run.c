#define WIN32_LEAN_AND_MEAN

#include <windows.h>

#include <ard/action.h>
#include <ard/config.h>

int
arda_run(_In_ const ardc_config *cfg)
{
    PostQuitMessage(arda_exec(cfg, cfg->run));
    return 0;
}
