#include <cassert>
#include <string>

#include <fcntl.h>
#include <unistd.h>

extern "C"
{
#include "../../impl.h"
#include <arch/dos.h>
#include <pal.h>
}

#include <mock/fmem.hpp>
#include <mock/xmem.hpp>

using namespace std::literals;

const auto URANDOM = "/dev/urandom";

template <typename Func>
void
test_case(Func func)
{
#ifdef CONFIG_IA16X
    mock::xmem::instance = mock::xmem{};

    dos_initialize_cache();
#else
    mock::fmem::instance = mock::fmem{};
#endif

    int fd = open(URANDOM, O_RDONLY);
    assert(0 <= fd);

    func(fd);

    close(fd);

#ifdef CONFIG_IA16X
    dos_cleanup_cache();
#endif
}

int
main(int argc, char *argv[])
{
    test_case([](int fd) {
        // can add and remove cache item

        auto cache1 = pal_cache(fd, 0, 256);
        assert(cache1);
        pal_discard(cache1);
    });

#ifdef CONFIG_IA16X
    test_case([](int fd) {
        // fails in case of handle exhaustion

        auto &m = mock::xmem::instance;
        m.max_handle = 2;

        auto cache1 = pal_cache(fd, 0, 256);
        assert(cache1);

        auto cache2 = pal_cache(fd, 1, 256);
        assert(cache2);

        auto cache3 = pal_cache(fd, 2, 256);
        assert(!cache3);

        pal_discard(cache1);
        cache3 = pal_cache(fd, 2, 256);
        assert(cache3);
    });

    test_case([](int fd) {
        // evicts when needed

        auto &m = mock::xmem::instance;
        m.max_handle = 2;

        auto cache1 = pal_cache(fd, 0, 256);
        assert(cache1);
        pal_discard(cache1);
        assert(1 == m.called_alloc);
        assert(0 == m.called_free);

        auto cache2 = pal_cache(fd, 1, 256);
        assert(cache2);
        pal_discard(cache1);
        assert(2 == m.called_alloc);
        assert(0 == m.called_free);

        auto cache3 = pal_cache(fd, 2, 256);
        assert(cache3);
        pal_discard(cache3);
        assert(4 == m.called_alloc);
        assert(1 == m.called_free);
    });
#endif

    return 0;
}
