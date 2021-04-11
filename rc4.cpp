#include "rc4.hpp"
#include <random>
#include <iomanip>
#include <iostream>

int main(void) {

    std::mt19937 prng;
    prng.seed(std::random_device()());
    std::uniform_int_distribution<std::uint8_t> dist;

    Rc4::Key k{
            0x01, 0x02, 0x03, 0x04,
            0x05, 0x06, 0x07, 0x08,
            0x09, 0x0a, 0x0b, 0x0c,
            0x0d, 0x0e, 0x0f, 0x10
    };

    Rc4 rc4(k);

    for (int i = 0; i < 20; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (unsigned) rc4.next();
    }

    std::cout << std::endl;

    return EXIT_SUCCESS;
}