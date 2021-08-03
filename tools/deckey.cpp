#include <format>
#include <iostream>

#include "key.hpp"


uint64_t decode_key(uint32_t disk_id, uint32_t secret)
{
    uint32_t low_part = (disk_id >> DISKID_LOW_PART) & DISKID_LOW_PART_MASK;
    uint32_t high_part = (((secret >> SECRET_HIGH_PART) & SECRET_HIGH_PART_MASK) << SECRET_HIGH_PART_SHIFT)
        | ((disk_id >> DISKID_HIGH_PART) & DISKID_HIGH_PART_MASK);

    unsigned low_oi = (secret >> SECRET_LOW_ROTATION) & SECRET_LOW_ROTATION_MASK;
    unsigned high_oi = (disk_id >> DISKID_HIGH_ROTATION) & DISKID_HIGH_ROTATION_MASK;

    low_part = rotr(low_part, KEY_ROTATION_OFFSETS[low_oi], 24);
    high_part = rotr(high_part, KEY_ROTATION_OFFSETS[high_oi], 24);

    return (((uint64_t)high_part & KEY_HIGH_PART_MASK) << KEY_HIGH_PART)
        | (((uint64_t)low_part & KEY_LOW_PART_MASK) << KEY_LOW_PART);
}

int main(int argc, char* argv[])
{
    while (true)
    {
        uint16_t disk_id_high, disk_id_low;
        uint32_t secret;

        std::cin >> std::hex >> disk_id_high;
        if (std::cin.get() != '-')
        {
            break;
        }
        std::cin >> std::hex >> disk_id_low;
        std::cin >> std::dec >> secret;

        if (!std::cin)
        {
            break;
        }

        uint32_t disk_id = ((uint32_t)disk_id_high << 16) | disk_id_low;

        uint64_t key = decode_key(disk_id, secret);
        std::cout << std::format("{:012X}", key) << std::endl;
    }

    return 0;
}
