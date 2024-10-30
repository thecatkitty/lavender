#include <string.h>

#include <fmt/exe.h>
#include <fmt/utf8.h>

// FIXME: W/A for https://sourceware.org/bugzilla/show_bug.cgi?id=30719
#if defined(__x86_64__) && !defined(WINVER)
extern char __executable_start[];
#endif

const char *
exe_pe_get_resource(void *rsrc, WORD type, WORD id)
{
    exe_pe_resource_directory       *dir;
    exe_pe_resource_directory_entry *ent;

    // Type
    dir = (exe_pe_resource_directory *)rsrc;
    ent = (exe_pe_resource_directory_entry *)(dir + 1);
    ent += dir->NumberOfNamedEntries;
    for (int i = 0; (dir->NumberOfIdEntries > i) && (type != ent->Id);
         ++i, ++ent)
        ;

    if (type != ent->Id)
    {
        return NULL;
    }

    // Item
    dir = (exe_pe_resource_directory *)((char *)rsrc +
                                        ent->dir.OffsetToDirectory);
    ent = (exe_pe_resource_directory_entry *)(dir + 1);
    ent += dir->NumberOfNamedEntries;
    for (int i = 0; (dir->NumberOfIdEntries > i) && (id != ent->Id); ++i, ++ent)
        ;

    if (id != ent->Id)
    {
        return NULL;
    }

    // Language: first available
    dir = (exe_pe_resource_directory *)((char *)rsrc +
                                        ent->dir.OffsetToDirectory);
    ent = (exe_pe_resource_directory_entry *)(dir + 1);

    exe_pe_resource_data_entry *data_ent =
        (exe_pe_resource_data_entry *)((char *)rsrc + ent->OffsetToData);
    char *data = (char *)(intptr_t)data_ent->OffsetToData;

// FIXME: W/A for https://sourceware.org/bugzilla/show_bug.cgi?id=30719
#if defined(__x86_64__) && !defined(WINVER)
    if (data < (char *)rsrc)
    {
        data += (intptr_t)__executable_start;
    }
#endif

    return data;
}

int
exe_pe_load_string(void *rsrc, unsigned id, char *buffer, int max_length)
{
    const char *resource =
        exe_pe_get_resource(rsrc, EXE_PE_RT_STRING, (id >> 4) + 1);
    if (NULL == resource)
    {
        return -1;
    }

    // One table = 16 strings
    const uint16_t *wstr = (const uint16_t *)resource;
    for (int i = 0; (id & 0xF) > i; ++i)
    {
        wstr += *wstr + 1;
    }

    // First WORD is string length
    const uint16_t *end = wstr + *wstr + 1;
    wstr++;

    // Convert WSTR to UTF-8
    char *buffptr = buffer;
    while ((wstr < end) && (buffer + max_length - 1 > buffptr))
    {
        char mb[3];

        int seqlen = utf8_get_sequence(*wstr, mb);
        if (buffer + max_length - 1 < buffptr + seqlen)
        {
            break;
        }

        memcpy(buffptr, mb, seqlen);
        wstr++;
        buffptr += seqlen;
    }

    *buffptr = 0;
    return buffptr - buffer;
}
