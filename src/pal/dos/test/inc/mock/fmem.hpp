#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <list>
#include <vector>

namespace mock
{
struct fmem
{
    static fmem instance;

    unsigned segments_start = 0x1000;
    unsigned segments_end = UINT16_MAX + 1;

    unsigned
    alloc(unsigned size, unsigned *seg);

    unsigned
    free(unsigned seg);

    void *
    make_far_pointer(unsigned s, unsigned o);

  private:
    struct allocation
    {
        unsigned          segment;
        std::vector<char> block;
    };

    std::list<allocation> m_segments = {};

    auto
    find(unsigned segment)
    {
        return std::find_if(begin(m_segments), end(m_segments),
                            [=](const allocation &a) {
                                return segment == a.segment;
                            });
    }
};
} // namespace mock
