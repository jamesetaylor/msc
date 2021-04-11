#ifndef MSC_RC4_HPP
#define MSC_RC4_HPP

#include <array>
#include <cstdint>
#include <algorithm>

class Rc4 {
    std::array<std::uint8_t, 256> state;
    std::uint8_t i, j;

public:
    static constexpr int m = 16;
    typedef std::array<std::uint8_t, m> Key;

    Rc4(Key const &key) : state(), i(), j() {
        {
            int i = 0;
            std::generate(state.begin(), state.end(), [&]() { return i++; });
        }

        for (int i = 0, j = 0; i < 256; ++i) {
            j = (j + state[i] + key[i % m]) % 256;
            std::swap(state[i], state[j]);
        }
    }

    std::uint8_t next() {
        ++i;
        j += state[i];
        std::swap(state[i], state[j]);
        return state[(state[i] + state[j]) % 256];
    }
};

#endif //MSC_RC4_HPP
