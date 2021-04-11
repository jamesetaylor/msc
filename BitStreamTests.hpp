#ifndef MSC_BITSTREAMTESTS_HPP
#define MSC_BITSTREAMTESTS_HPP

#include <cmath>
#include <tuple>
#include <boost/math/distributions/chi_squared.hpp>
#include <functional>

template<int bitWidth>
class SequenceTest {
protected:
    static constexpr int len = 1 << bitWidth;

    std::function<bool()> stream;
    boost::math::chi_squared chiSquared;
    std::uint64_t hands[len];
    std::uint64_t total;

    friend std::ostream &operator<<(std::ostream &lhs, SequenceTest const &rhs) {
        int const rows = 1 << (bitWidth / 2);
        int const cols = len / rows;

        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                lhs << std::setw(8) << rhs.hands[i * rows + j] << ", ";
            }
            lhs << '\n';
        }
        return lhs;
    }

public:
    SequenceTest(std::function<bool()> const &stream) : stream(stream), chiSquared(len - 1), hands(), total() {
        assert (0 == hands[0]);
        assert (0 == total);
    }

    double chiSquaredPValue() const {
        double const e = (double) total / (double) len;

        double sum = 0.;

        for (int i = 0; i < len; ++i) {
            sum += std::pow(hands[i] - e, 2.) / e;
        }

        return 1. - boost::math::cdf(chiSquared, sum);
    }

    virtual void extractObservation() = 0;
};

template<int bitWidth>
class SerialTest
        : public SequenceTest<bitWidth> {
    std::uint64_t bits;

    void extractBit() {
        bits <<= 1;
        bits |= this->stream() ? 1 : 0;
        bits %= SerialTest::len;
    }

public:
    SerialTest(std::function<bool()> const &g) : SequenceTest<bitWidth>(g), bits() {
        assert(0 == bits);
        for (int i = 0; i < bitWidth - 1; ++i) {
            extractBit();
        }
    }

    void extractObservation() {
        extractBit();
        this->hands[bits] += 1;
        this->total += 1;
    }
};

template<int bitWidth>
class PokerTest : public SequenceTest<bitWidth> {
public:
    PokerTest(std::function<bool()> const &g) : SequenceTest<bitWidth>(g) {
    }

    void extractObservation() {
        std::uint64_t bits = 0;

        for (int i = 0; i < bitWidth; ++i) {
            bits <<= 1;
            bits |= this->stream() ? 1 : 0;
        }
        assert(bits < PokerTest::len);
        this->hands[bits] += 1;
        this->total += 1;
    }
};

#endif //MSC_BITSTREAMTESTS_HPP
