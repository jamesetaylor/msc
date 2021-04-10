#include <bit>
#include <map>
#include <array>
#include <tuple>
#include <random>
#include <vector>
#include <iomanip>
#include <cstdint>
#include <cassert>
#include <iostream>
#include <algorithm>
#include <functional>

namespace {
    using namespace std;
    using namespace std::placeholders;

    typedef uint16_t HalfBlock;
    typedef tuple<HalfBlock, HalfBlock> Block;
    typedef HalfBlock Key;
    typedef vector<Key> Keys;
    typedef vector<Block> Blocks;

    inline Keys randomKeySchedule(mt19937 &prng) {
        Keys result;
        uniform_int_distribution<HalfBlock> dist;
        generate_n(back_inserter(result), 10, [&]() { return dist(prng); });
        return result;
    }

    inline Block operator^(Block const &lhs, Block const &rhs) {
        return make_tuple(
                get<0>(lhs) ^ get<0>(rhs),
                get<1>(lhs) ^ get<1>(rhs)
        );
    }

    inline constexpr HalfBlock e(int i) {
        assert(i < 16);
        return HalfBlock(1) << i;
    }

    class Cipher {
        Keys const keys_;

        typedef HalfBlock (*KeyCombiner)(HalfBlock, HalfBlock);

        inline static HalfBlock xor_(HalfBlock x, HalfBlock k) {
            return x ^ k;
        }

        inline static HalfBlock add_(HalfBlock x, HalfBlock k) {
            return x + k;
        }

        inline Block roundFunction(KeyCombiner combineKey, Key k, Block xy) const {
            auto &x = get<0>(xy);
            auto &y = get<1>(xy);
            y ^= x;
            x = rotl(x, y & 15);
            x = combineKey(x, k);
            return make_tuple(y, x);
        }

    public:
        inline explicit Cipher(Keys const &keys)
                : keys_(keys) {
            assert(10 == keys_.size());
        }

        inline void encrypt(Block plaintext, Blocks &result) const {
            result.resize(0);
            result.push_back(plaintext);

            for (int i = 0; i < 9; ++i) {
                result.push_back(roundFunction(xor_, keys_[i], result.back()));
            }

            result.push_back(roundFunction(add_, keys_.back(), result.back()));
        }
    };

    struct Attack {
        mt19937 &prng;
        uniform_int_distribution<HalfBlock> dist;
        Cipher &cipher;

        inline Block nextRandomBlock() {
            return Block(dist(prng), dist(prng));
        }

        /**
         * Filter ciphertext pairs that have zero difference in the leftmost 16 state_.  Accesses only the ciphertext.
         */
        inline bool filter(tuple<Blocks, Blocks> const &encryptionResults) const {
            assert(11 == get<0>(encryptionResults).size());
            assert(11 == get<1>(encryptionResults).size());
            return get<0>(get<0>(encryptionResults).back()) == get<0>(get<1>(encryptionResults).back());
        }

        /**
         * Determine whether a pair of ciphertexts is a right pair. Accesses the result of round 9.
         */
        inline bool isRightPair(tuple<Blocks, Blocks> const &encryptionResults) const {
            auto &c0 = get<0>(encryptionResults);
            auto &c1 = get<1>(encryptionResults);
            auto d = (c0[9] ^ c1[9]);
            return get<0>(d) == get<1>(d) && 1 == popcount(get<0>(d));
        }

    public:
        inline Attack(mt19937 &prng, Cipher &cipher) :
                prng(prng),
                dist(),
                cipher(cipher) {
        }

        inline Key recoverKey10() {
            vector<tuple<Blocks, Blocks>> rightPairCandidates;
            tuple<Blocks, Blocks> encryptionResults;

            /**
             * Generate 2^20 plaintext pairs with xor difference (e15,e15) and filter using filter() above. These are
             * our right pair candidates.
             */
            for (int i = 0; i < 1 << 20; ++i) {
                Block plaintext = nextRandomBlock();
                cipher.encrypt(plaintext, get<0>(encryptionResults));
                cipher.encrypt(plaintext ^ Block(e(15), e(15)), get<1>(encryptionResults));
                if (filter(encryptionResults)) {
                    rightPairCandidates.push_back(encryptionResults);
                }
            }

            /**
             * Report how many pairs passed the filter and how many were actually right pairs.
             */
            auto filterCount = rightPairCandidates.size();
            auto rightPairCount = count_if(
                    rightPairCandidates.begin(),
                    rightPairCandidates.end(),
                    bind(&Attack::isRightPair, this, _1)
            );

            cout << dec;
            cout << filterCount << " pairs passed the filter of which " << rightPairCount << " are right pairs" << endl;

            /**
             * For each key for each right pair candidate, calculate a metric which will be 1 when k is the correct key
             * and [0,16) otherwise.  The sum of this metric is stored per key in weights.  Since E(hwt) > 1 in general
             * but E(hwt) = 1 for the correct key, we expect the key with the lowest weight to be the correct key.
             */
            array<unsigned long, 1 << 16> weights;
            fill(weights.begin(), weights.end(), 0);

            for (int k = 0; k < 1 << 16; ++k) {
                for (auto &rightPairCandidate : rightPairCandidates) {
                    HalfBlock const& N0 = get<1>(get<0>(rightPairCandidate).back());
                    HalfBlock const& N1 = get<1>(get<1>(rightPairCandidate).back());
                    weights[k] += popcount(HalfBlock(HalfBlock(N0 - k) ^ HalfBlock(N1 - k)));
                }
            }

            return distance(weights.begin(), min_element(weights.begin(), weights.end()));
        }
    };
}

int main(void) {
    random_device rd;
    mt19937 prng(rd());

    int successes = 0;

    for (int i = 0; i < 10; ++i) {
        Keys keySchedule = randomKeySchedule(prng);
        Cipher cipher(keySchedule);
        Attack attack(prng, cipher);

        auto actualKey = keySchedule.back();
        auto recoveredKey = attack.recoverKey10();
        bool success = !(0x7fff & (actualKey ^ recoveredKey));

        successes += success ? 1 : 0;

        cout << hex;
        cout << "test " << i << ": result = " << (success ? "SUCCESS" : "FAILURE") << endl;
        cout << "actual    key = [" << setw(4) << setfill('0') << actualKey << ']' << endl;
        cout << "recovered key  = [" << setw(4) << setfill('0') << recoveredKey << ']' << endl;
    }

    cout << dec << successes << "/10 successes" << endl;
}