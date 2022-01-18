#include <format>
#include <iostream>

#include "key.hpp"

std::pair<uint32_t, uint32_t>
encode_key(uint64_t key, unsigned i)
{
    uint32_t low_part = (key >> KEY_LOW_PART) & KEY_LOW_PART_MASK;
    uint32_t high_part = (key >> KEY_HIGH_PART) & KEY_HIGH_PART_MASK;

    unsigned low_oi = i % KEY_ROTATION_OFFSETS_COUNT;
    unsigned high_oi = i / KEY_ROTATION_OFFSETS_COUNT;

    low_part = rotl(low_part, KEY_ROTATION_OFFSETS[low_oi], 24);
    high_part = rotl(high_part, KEY_ROTATION_OFFSETS[high_oi], 24);

    return {((high_oi & DISKID_HIGH_ROTATION_MASK) << DISKID_HIGH_ROTATION) |
                ((high_part & DISKID_HIGH_PART_MASK) << DISKID_HIGH_PART) |
                ((low_part & DISKID_LOW_PART_MASK) << DISKID_LOW_PART),
            ((low_oi & SECRET_LOW_ROTATION_MASK) << SECRET_LOW_ROTATION) |
                (((high_part >> SECRET_HIGH_PART_SHIFT) & SECRET_HIGH_PART_MASK)
                 << SECRET_HIGH_PART)};
}

int
main(int argc, char *argv[])
{
    while (true)
    {
        uint64_t key;
        std::cin >> std::hex >> key;

        if (!std::cin)
        {
            break;
        }

        if (key > umaxval(48))
        {
            std::cerr << "ERROR: Key cannot be over 48-bit long!" << std::endl;
            continue;
        }

        for (unsigned i = 0; i < 16; i++)
        {
            auto [disk_id, secret] = encode_key(key, i);

            if (secret >= 1000000)
            {
                continue;
            }

            std::cout << std::format("{:04X}-{:04X}\t{:06}", disk_id >> 16,
                                     disk_id & ((1 << 16) - 1), secret)
                      << std::endl;
        }
    }

    return 0;
}
