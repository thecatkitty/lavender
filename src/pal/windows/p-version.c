#include <stdio.h>

#include <arch/windows.h>
#include <pal.h>

#include "impl.h"

static LPVOID info_ = NULL;
static WORD  *translation_ = NULL;
static char   version_[MAX_PATH] = {0};

static bool
load_version_info(void)
{
    WCHAR self[126]; // Length limited by 9x
    DWORD size = 0;
    WORD *translation = NULL;
    UINT  translation_len = 0;

    if (!GetModuleFileNameW(NULL, self, lengthof(self)))
    {
        LOG("cannot get the module name of self");
        return false;
    }

    if (0 == (size = GetFileVersionInfoSizeW(self, NULL)))
    {
        LOG("cannot get the version info size");
        return false;
    }

    info_ = LocalAlloc(LMEM_FIXED, size);
    if (NULL == info_)
    {
        LOG("cannot allocate memory for version info");
        return false;
    }

    if (!GetFileVersionInfoW(self, 0, size, info_))
    {
        LOG("cannot access version info");
        LocalFree(info_);
        info_ = NULL;
        return false;
    }

    if (!VerQueryValueW(info_, L"\\VarFileInfo\\Translation",
                        (LPVOID *)&translation, &translation_len))
    {
        LOG("cannot query Translation");
        LocalFree(info_);
        info_ = NULL;
        return false;
    }

    translation_ = translation;
    return true;
}

static LPCWSTR
load_string_file_info(const char *name)
{
    WCHAR  path[MAX_PATH];
    WCHAR *string = NULL;
    UINT   string_len = 0;

    swprintf(path, MAX_PATH, L"\\StringFileInfo\\%04X%04X\\" FMT_AS,
             translation_[0], translation_[1], name);

    if (!VerQueryValueW(info_, path, (LPVOID *)&string, &string_len))
    {
        LOG("cannot query %s", name);
        return NULL;
    }

    return string;
}

const char *
pal_get_version_string(void)
{
    LPCWSTR part;
    LOG("entry");

    if (version_[0])
    {
        LOG("exit, cached");
        return version_;
    }

    if (!load_version_info())
    {
        LOG("cannot load version info");
        return "Lavender";
    }

    part = load_string_file_info("ProductName");
    if (NULL == part)
    {
        LOG("cannot load product name string");
        strcpy(version_, "Lavender");
        goto end;
    }
    WideCharToMultiByte(CP_UTF8, 0, part, -1, version_, lengthof(version_),
                        NULL, NULL);

    part = load_string_file_info("ProductVersion");
    if (NULL == part)
    {
        LOG("cannot load product version string");
        goto end;
    }

    strncat(version_, " ", lengthof(version_) - strlen(version_));
    WideCharToMultiByte(CP_UTF8, 0, part, -1, version_ + strlen(version_),
                        lengthof(version_) - strlen(version_), NULL, NULL);

end:
    LocalFree(info_);
    LOG("exit, '%s'", version_);
    return version_;
}
