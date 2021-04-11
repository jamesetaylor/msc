#include "lfsr.hpp"
#include <random>
#include <iostream>
#include <vector>
#include <tuple>
#include <algorithm>
#include <cmath>
#include <stdexcept>

template<int n, int p, int m>
class Geffe {
    Lfsr<n> lfsr1;
    Lfsr<p> lfsr2;
    Lfsr<m> lfsr3;

public:
    Geffe(std::uint64_t iv1, std::uint64_t iv2, std::uint64_t iv3) : lfsr1(iv1), lfsr2(iv2), lfsr3(iv3) {
    }

    bool next() {
        bool x1 = lfsr1.next();
        bool x2 = lfsr2.next();
        bool x3 = lfsr3.next();
        bool result = (x1 && !x2) != (x2 && x3);
        return result;
    }
};

template<int n>
std::uint64_t guessIv(std::vector<bool> const &targetStream) {
    std::vector<bool> guessStream;
    guessStream.reserve(targetStream.size());

    std::uint64_t bestIv = 0;
    int bestIvCount = 0;

    for (std::uint64_t i = 1; i < 1 << n; ++i) {
        guessStream.resize(0);
        Lfsr<n> lfsr(i);

        std::generate_n(std::back_inserter(guessStream), targetStream.size(), std::bind(&Lfsr<n>::next, &lfsr));

        int sum = 0;
        for (auto i = targetStream.cbegin(), j = guessStream.cbegin(); i != targetStream.end(); ++i, ++j) {
            if (*i == *j) {
                sum += 1;
            }
        }

        std::cout << "i = " << i << ", total = " << sum << std::endl;
        if (sum > bestIvCount) {
            bestIv = i;
            bestIvCount = sum;
        }
    }

    return bestIv;
}

std::tuple<std::uint64_t, std::uint64_t, std::uint64_t>
bruteForce(std::vector<bool> const &targetStream) {
    std::vector<bool> guessStream;
    guessStream.reserve(targetStream.size());

    for (int i = 0; i < 1 << 3; ++i) {
        for (int j = 0; j < 1 << 4; ++j) {
            for (int k = 0; k < 1 << 5; ++k) {
                guessStream.resize(0);
                Geffe<3, 4, 5> geffe(i, j, k);
                std::generate_n(
                        std::back_inserter(guessStream),
                        targetStream.size(),
                        std::bind(&Geffe<3, 4, 5>::next, &geffe)
                );

                if (guessStream == targetStream) {
                    return std::make_tuple(i, j, k);
                }
            }
        }
    }

    throw std::runtime_error("brute force attack failed");
}

int
main(void) {
    static constexpr int n = 3;
    static constexpr int p = 4;
    static constexpr int m = 5;


    std::mt19937 prng;
    prng.seed(std::random_device()());
    std::uniform_int_distribution<std::uint64_t> d1(1, (1 << n) - 1);
    std::uniform_int_distribution<std::uint64_t> d2(1, (1 << p) - 1);
    std::uniform_int_distribution<std::uint64_t> d3(1, (1 << m) - 1);

    std::vector<bool> interceptedKeystream{0, 0, 1, 0, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1};

    std::cout << "guessing iv1" << std::endl;
    int iv1 = guessIv<3>(interceptedKeystream);
    std::cout << "guessing iv3" << std::endl;
    int iv3 = guessIv<5>(interceptedKeystream);

    std::cout << "iv1 = " << iv1 << std::endl;
    std::cout << "iv3 = " << iv3 << std::endl;

    std::vector<bool> v;
    v.reserve(interceptedKeystream.size());

    for (int i = 1; i < 1 << 4; ++i) {
        v.resize(0);
        Geffe<n, p, m> geffe(1, i, 6);
        std::generate_n(
                std::back_inserter(v),
                interceptedKeystream.size(),
                std::bind(&Geffe<n, p, m>::next, &geffe)
        );

        if (v == interceptedKeystream) {
            std::cout << "iv2 = " << i << std::endl;
        }
    }

    auto bruteForceResult = bruteForce(interceptedKeystream);

    std::cout << "brute force result = (" << std::get<0>(bruteForceResult) << ", " << std::get<1>(bruteForceResult)
              << ", " << std::get<2>(bruteForceResult) << ')' << std::endl;

    return EXIT_SUCCESS;
}