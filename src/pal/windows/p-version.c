#include <stdio.h>

#include <arch/windows.h>
#include <pal.h>

#include "impl.h"

static LPVOID resource_ = NULL;
static WORD  *translation_ = NULL;
static char   version_[MAX_PATH] = {0};

static bool
load_version_info(void)
{
    DWORD   resource_size;
    HGLOBAL resource_data;
    LPVOID  resource;
    WORD   *translation = NULL;
    UINT    translation_len = 0;

    HRSRC resource_info = FindResourceW(NULL, MAKEINTRESOURCEW(1), RT_VERSION);
    if (NULL == resource_info)
    {
        LOG("cannot find version resource");
        return false;
    }

    resource_size = SizeofResource(NULL, resource_info);
    resource_data = LoadResource(NULL, resource_info);
    if (NULL == resource_data)
    {
        LOG("cannot load version resource");
        return false;
    }

    resource = LockResource(resource_data);
    if (NULL == resource)
    {
        LOG("cannot lock version resource");
        FreeResource(resource_data);
        return false;
    }

    resource_ = LocalAlloc(LMEM_FIXED, resource_size);
    if (NULL == resource_)
    {
        LOG("cannot allocate memory for version resource");
        FreeResource(resource_data);
        return false;
    }

    CopyMemory(resource_, resource, resource_size);
    FreeResource(resource_data);

    if (!VerQueryValueW(resource_, L"\\VarFileInfo\\Translation",
                        (LPVOID *)&translation, &translation_len))
    {
        LOG("cannot query Translation");
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

    if (!VerQueryValueW(resource_, path, (LPVOID *)&string, &string_len))
    {
        LOG("cannot query %s", name);
        return NULL;
    }

    return string;
}

const char *
pal_get_version_string(void)
{
    LPCWSTR name, version;
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

    name = load_string_file_info("ProductName");
    if (NULL == name)
    {
        LOG("cannot load product name string");
        return "Lavender";
    }

    version = load_string_file_info("ProductVersion");
    if (NULL == version)
    {
        LOG("cannot load product version string");
        WideCharToMultiByte(CP_UTF8, 0, name, -1, version_, MAX_PATH, NULL,
                            NULL);
    }
    else
    {
        snprintf(version_, MAX_PATH, "%ls %ls", name, version);
    }

    LocalFree(resource_);
    LOG("exit, '%s'", version_);
    return version_;
}
