#include <stdio.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <unistd.h>

#include <blkid/blkid.h>

#include <pal.h>

uint32_t
pal_get_medium_id(const char *tag)
{
    LOG("entry");
    unsigned volume_sn_high = 0, volume_sn_low = 0;

    char self[PATH_MAX];
    if (0 > readlink("/proc/self/exe", self, PATH_MAX))
    {
        LOG("exit, cannot retrieve executable path!");
        return 0;
    }

    struct stat info;
    if (0 > stat(self, &info))
    {
        LOG("exit, cannot get file info for '%s'!", self);
        return 0;
    }

    blkid_cache cache;
    if (0 > blkid_get_cache(&cache, NULL))
    {
        LOG("exit, cannot get blkid cache!");
        return 0;
    }

    char *devname = blkid_devno_to_devname(info.st_dev);
    if (NULL == devname)
    {
        LOG("exit, cannot get devname for %d:%d!", major(info.st_dev),
            minor(info.st_dev));
        return 0;
    }

    const char *label = blkid_get_tag_value(cache, "LABEL", devname);
    if (NULL == label)
    {
        LOG("cannot get label of '%s'!", devname);
        goto end;
    }

    if (0 != strcmp(tag, label))
    {
        LOG("label '%s' not matching '%s'", label, tag);
        goto end;
    }

    const char *uuid = blkid_get_tag_value(cache, "UUID", devname);
    if (NULL == uuid)
    {
        LOG("cannot get UUID of '%s'!", devname);
        goto end;
    }

    LOG("dev %d:%d [%s] '%s' - uuid: %s", major(info.st_dev),
        minor(info.st_dev), devname, label, uuid);

    if (2 != sscanf(uuid, "%04X-%04X", &volume_sn_high, &volume_sn_low))
    {
        LOG("unknown volume serial number format!");
        volume_sn_high = volume_sn_low = 0;
        goto end;
    }

end:
    LOG("exit, %04X-%04X", volume_sn_high, volume_sn_low);
    free(devname);
    return (volume_sn_high << 16) | volume_sn_low;
}
