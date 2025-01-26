#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include <fmt/zip.h>

#include <assets.h>

#ifdef __ia16__
// newlib-ia16 seems to have some misbehaving tmpnam implementation
static int tmpnam_num_ = 0;

static char *
my_tmpnam(char *path)
{
    *path = 0;

    const char *tmp_dir = getenv("TMP");
    tmp_dir = tmp_dir ? tmp_dir : getenv("TEMP");
    if (tmp_dir)
    {
        strcpy(path, tmp_dir);
        strcat(path, "\\");
    }

    char name[13];
    sprintf(name, "~laven%02x.tmp", tmpnam_num_++);
    strcat(path, name);

    return path;
}
#elif defined(_WIN32)
#include <windows.h>

static char *
my_tmpnam(char *path)
{
    if (0 == GetTempPathA(PATH_MAX, path))
    {
        return NULL;
    }

    if (0 == GetTempFileNameA(path, "lav", 0, path))
    {
        return NULL;
    }

    return path;
}
#else
#define my_tmpnam tmpnam
#endif

int
pal_extract_asset(const char *name, char *path)
{
    FILE *out = NULL;
    off_t lfh;

    LOG("entry, name: '%s'", name);

    lfh = zip_search(name, strlen(name));
    if (0 > lfh)
    {
        LOG("exit, cannot find asset. %s", strerror(errno));
        goto exit;
    }

    if (NULL == my_tmpnam(path))
    {
        LOG("cannot create the temporary file name!");
        errno = ENOMEM;
        goto exit;
    }

    if (NULL == (out = fopen(path, "wb")))
    {
        LOG("cannot open the file!");
        goto exit;
    }

    if (!zip_extract_data(lfh, out))
    {
        LOG("cannot write the file!");
        goto exit;
    }

    errno = 0;

exit:
    if (NULL != out)
    {
        fclose(out);
    }

    LOG("exit, %d", -errno);
    return -errno;
}
