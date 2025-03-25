#include <algorithm>

extern "C"
{
#include <arch/dos.h>
}

#include <mock/xmem.hpp>

mock::xmem mock::xmem::instance{};

hdosxm
dosxm_alloc(uint16_t size_kb)
{
    return reinterpret_cast<hdosxm>(mock::xmem::instance.alloc(size_kb));
}

bool
dosxm_free(hdosxm block)
{
    return mock::xmem::instance.free(
        static_cast<unsigned>(reinterpret_cast<uintptr_t>(block)));
}

bool
dosxm_load(far void *dst, hdosxm src, uint32_t offset, uint32_t length)
{
    return mock::xmem::instance.load(
        dst, static_cast<unsigned>(reinterpret_cast<uintptr_t>(src)), offset,
        length);
}

bool
dosxm_store(hdosxm dst, uint32_t offset, far void *src, uint32_t length)
{
    return mock::xmem::instance.store(
        static_cast<unsigned>(reinterpret_cast<uintptr_t>(dst)), offset, src,
        length);
}

unsigned
mock::xmem::alloc(uint16_t size_kb)
{
    called_alloc++;

    if (max_handle <=
        std::count_if(begin(m_blocks), end(m_blocks), [](const allocation &it) {
            return !it.block.empty();
        }))
    {
        return 0;
    }

    auto item =
        std::find_if(begin(m_blocks), end(m_blocks), [](const auto &it) {
            return it.block.empty();
        });
    if (m_blocks.end() != item)
    {
        item->block.resize(size_kb * 1024);
        return item - m_blocks.begin() + 1;
    }

    m_blocks.emplace_back();
    m_blocks.back().block.resize(size_kb * 1024);
    return m_blocks.size();
}

bool
mock::xmem::free(unsigned block)
{
    called_free++;

    auto &item = m_blocks.at(block - 1);
    item.loads = 0;
    item.stores = 0;

    auto size = item.block.size();
    item.block.clear();
    return 0 < size;
}

bool
mock::xmem::load(void *dst, unsigned src, uint32_t offset, uint32_t length)
{
    called_load++;

    auto &item = m_blocks.at(src - 1);
    std::copy_n(begin(item.block) + offset, length,
                reinterpret_cast<char *>(dst));
    item.loads++;
    return true;
}

bool
mock::xmem::store(unsigned dst, uint32_t offset, void *src, uint32_t length)
{
    called_store++;

    auto &item = m_blocks.at(dst - 1);
    std::copy_n(reinterpret_cast<char *>(src), length, begin(item.block));
    item.stores++;
    return true;
}
