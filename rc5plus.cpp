#include <tuple>
#include <iostream>
#include <cstdint>
#include <cassert>
#include <random>
#include <vector>
#include <algorithm>
#include <functional>

namespace {
    using namespace std;

    typedef uint16_t HalfBlock;
    typedef HalfBlock Key;
    typedef vector<Key> Keys;
    typedef tuple<HalfBlock, HalfBlock> Block;
    typedef vector<Block> Blocks;

    inline Block operator^(Block const &lhs, Block const &rhs) {
        return make_tuple(
                get<0>(lhs) ^ get<0>(rhs),
                get<1>(lhs) ^ get<1>(rhs)
        );
    }

    inline constexpr HalfBlock leftRotate(HalfBlock t, int shift) {
        assert(i < 16);
        return (t << shift) | (t >> (16 - shift));
    }

    inline constexpr HalfBlock e(int i) {
        assert(i < 16);
        return uint16_t(1) << i;
    }

    inline Block roundFunction(Key k, Block xy) {
        auto &x = get<0>(xy);
        auto &y = get<1>(xy);
        y ^= x;
        x = leftRotate(x, y & 15);
        x ^= k;
        return make_tuple(y, x);
    }

    inline void encrypt(Keys const &keys, Block plaintext, Blocks &output) {
        output.resize(0);
        output.push_back(plaintext);
        for (auto k : keys) {
            output.push_back(roundFunction(k, output.back()));
        }
    }

    struct Tester {
        random_device rd;
        mt19937 prng;
        uniform_int_distribution<HalfBlock> dist;
        Blocks const characteristic;

        HalfBlock randomHalfBlock() {
            return dist(prng);
        }

    public:
        Tester()
                : rd(),
                  prng(rd()),
                  dist(),
                  characteristic{
                          Block(e(15), e(15)),
                          Block(0, e(15)),
                          Block(e(15), 0),
                          Block(e(15), e(15)),
                          Block(0, e(15)),
                          Block(e(15), 0),
                          Block(e(15), e(15))
                  } {
        }

        void runTest(vector<uint32_t> &results) {
            results.resize(characteristic.size());

            vector<uint16_t> keys;
            generate_n(
                    back_inserter(keys),
                    characteristic.size() - 1,
                    bind(&Tester::randomHalfBlock, this)
            );

            Blocks result1, result2;

            for (int i = 0; i < 1 << 20; ++i) {
                result1.resize(0);
                result2.resize(0);

                Block a1(randomHalfBlock(), randomHalfBlock());
                Block a2 = a1 ^characteristic.front();

                encrypt(keys, a1, result1);
                encrypt(keys, a2, result2);

                for (int i = 0, e = characteristic.size(); i < e; ++i) {
                    if ((result1[i] ^ result2[i]) == characteristic[i]) {
                        ++results[i];
                    } else {
                        break;
                    }
                }
            }
        }
    };
}

int main(void) {
    Tester tester;

    vector<uint32_t> results;

    for (int i = 0; i < 100; ++i) {
        tester.runTest(results);
    }

    for (auto i : results) {
        cout << i << endl;
    }
}