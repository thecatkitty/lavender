#include <fstream>
#include <iostream>
#include <sstream>

#include "bits.hpp"

int
main(int argc, char *argv[])
{
    if (argc < 3)
    {
        std::cerr << "ERROR: 3 arguments expected - input, output, key!"
                  << std::endl;
        return 1;
    }

    std::ifstream fin{argv[1], std::ios::binary};
    if (!fin)
    {
        std::cerr << "ERROR: Cannot open input!" << std::endl;
        return 1;
    }

    std::ofstream fout{argv[2], std::ios::binary};
    if (!fout)
    {
        std::cerr << "ERROR: Cannot open output!" << std::endl;
        return 1;
    }

    uint64_t           key;
    std::istringstream iss{argv[3]};
    iss >> std::hex >> key;
    if (!iss)
    {
        std::cerr << "ERROR: Invalid key format!" << std::endl;
        return 1;
    }

    if (key > umaxval(48))
    {
        std::cerr << "ERROR: Key cannot be over 48-bit long!" << std::endl;
        return 1;
    }

    char *key_bytes{reinterpret_cast<char *>(&key)};
    int   i{0};

    while (true)
    {
        char byte = fin.get();
        if (!fin)
        {
            break;
        }

        fout.put(byte ^ key_bytes[i]);
        i++;
        i %= 6;
    }

    return 0;
}
