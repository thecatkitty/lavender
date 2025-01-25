#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include <pal.h>

static char state_dir_[PATH_MAX] = "";

static void
mkdir_recursive(const char *path)
{
    LOG("entry, '%s'", path);

    char   temp_path[PATH_MAX] = "";
    size_t len = strlen(path);

    strncpy(temp_path, path, PATH_MAX);
    temp_path[len] = '\0';

    for (char *part = temp_path + 1; *part; part++)
    {
        if (*part == '/')
        {
            *part = '\0'; // Terminate the string
            if ((0 != mkdir(temp_path, 0700)) && (EEXIST != errno))
            {
                LOG("exit, cannot create '%s'", temp_path);
                return;
            }
            *part = '/'; // Restore the slash
        }
    }

    if ((0 != mkdir(temp_path, 0700)) && (EEXIST != errno))
    {
        LOG("exit, cannot create '%s'", temp_path);
        return;
    }

    LOG("exit, ok");
    return;
}

static bool
find_state_dir(void)
{
    LOG("entry");

    if (0 != state_dir_[0])
    {
        LOG("exit, already known: '%s'", state_dir_);
        return true;
    }

    const char *xdg_state_home = getenv("XDG_STATE_HOME");
    if (NULL != xdg_state_home)
    {
        strncpy(state_dir_, xdg_state_home, PATH_MAX);
    }
    else
    {
        const char *home = getenv("HOME");
        if (home == NULL)
        {
            LOG("exit, unknown $HOME");
            return false;
        }

        snprintf(state_dir_, PATH_MAX, "%s/.local/state", home);
    }

    strncat(state_dir_, "/celones/lavender", PATH_MAX - strlen(state_dir_));
    LOG("exit, '%s'", state_dir_);
    return true;
}

static bool
concat_path(char *path, const char *parent, const char *child)
{
    size_t parent_len = strlen(parent);
    size_t child_len = strlen(child);

    if (PATH_MAX <= (parent_len + child_len + 2))
    {
        return false;
    }

    strncpy(path, parent, PATH_MAX - 1);
    path[PATH_MAX - 1] = '\0';

    if (parent[parent_len - 1] != '/')
    {
        strncat(path, "/", PATH_MAX - strlen(path) - 1);
    }

    strncat(path, child, PATH_MAX - strlen(path) - 1);
    return true;
}

size_t
pal_load_state(const char *name, uint8_t *buffer, size_t size)
{
    LOG("entry, '%s'", name);

    if (!find_state_dir())
    {
        LOG("exit, unknown state directory");
        return 0;
    }

    char path[PATH_MAX];
    if (!concat_path(path, state_dir_, name))
    {
        LOG("exit, path too long");
        return 0;
    }

    int fd;
    if (0 > (fd = open(path, O_RDONLY)))
    {
        LOG("exit, cannot open '%s'", path);
        return 0;
    }

    int byte_count;
    if (0 >= (byte_count = read(fd, buffer, size)))
    {
        LOG("exit, cannot read '%s'", path);
        close(fd);
        return 0;
    }

    close(fd);
    LOG("exit, %d bytes read", byte_count);
    return (size_t)byte_count;
}

bool
pal_save_state(const char *name, const uint8_t *buffer, size_t size)
{
    LOG("entry, '%s'", name);

    if (!find_state_dir())
    {
        LOG("exit, unknown state directory");
        return false;
    }
    mkdir_recursive(state_dir_);

    char path[PATH_MAX];
    if (!concat_path(path, state_dir_, name))
    {
        LOG("exit, path too long");
        return 0;
    }

    int fd;
    if (0 > (fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0600)))
    {
        LOG("exit, cannot open '%s'", path);
        return false;
    }

    if (size != write(fd, buffer, size))
    {
        LOG("exit, cannot write '%s'", path);
        close(fd);
        return false;
    }

    close(fd);
    LOG("exit, %d bytes written", size);
    return true;
}
