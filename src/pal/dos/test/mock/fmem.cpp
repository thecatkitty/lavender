#include <cassert>

extern "C"
{
#include <dos.h>
}

#include <mock/fmem.hpp>

mock::fmem mock::fmem::instance{};

unsigned
_dos_allocmem(unsigned __size, unsigned *__seg)
{
    return mock::fmem::instance.alloc(__size, __seg);
}

unsigned
_dos_freemem(unsigned __seg)
{
    return mock::fmem::instance.free(__seg);
}

void *
MK_FP(unsigned __s, unsigned __o)
{
    return mock::fmem::instance.make_far_pointer(__s, __o);
}

unsigned
mock::fmem::alloc(unsigned size, unsigned *seg)
{
    assert(UINT16_MAX >= size);

    unsigned segment = segments_start;

    auto it = m_segments.begin();
    while (it != m_segments.end())
    {
        if ((segment + size) <= it->segment)
        {
            break;
        }

        segment = it->segment + it->block.size() / 16;
        it++;
    }

    if (segments_end <= segment)
    {
        return 8;
    }

    auto item =
        m_segments.emplace(it, allocation{segment, std::vector<char>{}});
    item->block.resize(size * 16);
    *seg = item->segment;
    return 0;
}

unsigned
mock::fmem::free(unsigned seg)
{
    auto it = find(seg);
    if (m_segments.end() == it)
    {
        return 9;
    }

    m_segments.erase(it);
    return 0;
}

void *
mock::fmem::make_far_pointer(unsigned s, unsigned o)
{
    auto it = find(s);
    return m_segments.end() == it ? nullptr : it->block.data() + o;
}
