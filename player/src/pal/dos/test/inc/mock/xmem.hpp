#pragma once

#include <cstdint>
#include <list>
#include <vector>

namespace mock
{
struct xmem
{
    static xmem instance;

    unsigned max_handle = 32;
    unsigned called_alloc = 0;
    unsigned called_free = 0;
    unsigned called_load = 0;
    unsigned called_store = 0;

    unsigned
    alloc(uint16_t size_kb);

    bool
    free(unsigned block);

    bool
    load(void *dst, unsigned src, uint32_t offset, uint32_t length);

    bool
    store(unsigned dst, uint32_t offset, void *src, uint32_t length);

    auto
    get_loads(unsigned block) const
    {
        return m_blocks.at(block - 1).loads;
    }

    auto
    get_stores(unsigned block) const
    {
        return m_blocks.at(block - 1).stores;
    }

  private:
    struct allocation
    {
        std::vector<char> block = {};
        unsigned          loads = 0;
        unsigned          stores = 0;
    };

    std::vector<allocation> m_blocks = {};
};
} // namespace mock
