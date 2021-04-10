#include <cstdint>
#include <bit>
#include <iostream>

template<int len> struct PrimitivePolynomial;
template<> struct PrimitivePolynomial<1> { static constexpr std::uint64_t value = 0b1ull; };
template<> struct PrimitivePolynomial<2> { static constexpr std::uint64_t value = 0b11ull; };
template<> struct PrimitivePolynomial<3> { static constexpr std::uint64_t value = 0b110ull; };
template<> struct PrimitivePolynomial<4> { static constexpr std::uint64_t value = 0b1100ull; };
template<> struct PrimitivePolynomial<5> { static constexpr std::uint64_t value = 0b10100ull; };
template<> struct PrimitivePolynomial<6> { static constexpr std::uint64_t value = 0b110000ull; };
template<> struct PrimitivePolynomial<7> { static constexpr std::uint64_t value = 0b1100000ull; };
template<> struct PrimitivePolynomial<8> { static constexpr std::uint64_t value = 0b10111000ull; };
template<> struct PrimitivePolynomial<9> { static constexpr std::uint64_t value = 0b100010000ull; };
template<> struct PrimitivePolynomial<10> { static constexpr std::uint64_t value = 0b1001000000ull; };
template<> struct PrimitivePolynomial<11> { static constexpr std::uint64_t value = 0b10100000000ull; };
template<> struct PrimitivePolynomial<12> { static constexpr std::uint64_t value = 0b111000001000ull; };
template<> struct PrimitivePolynomial<13> { static constexpr std::uint64_t value = 0b1110010000000ull; };
template<> struct PrimitivePolynomial<14> { static constexpr std::uint64_t value = 0b11100000000010ull; };
template<> struct PrimitivePolynomial<15> { static constexpr std::uint64_t value = 0b110000000000000ull; };

template<int len_, std::uint64_t taps_ = PrimitivePolynomial<len_>::value>
class Lfsr {
    static constexpr std::uint64_t outputBit = std::uint64_t(1) << len_;

    std::uint64_t state_;

    friend std::ostream &operator<<(std::ostream &lhs, Lfsr const &rhs) {
        std::uint64_t bits = rhs.state_;
        bits <<= 1;
        for (int i = 0; i < len_; ++i) {
            lhs << ((bits & outputBit) ? '1' : '0');
            bits <<= 1;
        }
        return lhs;
    }

public:
    Lfsr(std::uint64_t iv) : state_(iv) {
    }

    bool next() {
        std::uint64_t nextBit = std::popcount(taps_ & state_) & 1;
        state_ <<= 1;
        state_ |= nextBit;
        return state_ & outputBit;
    }

    std::uint64_t state() const {
        return state_;
    }

    static constexpr std::uint64_t taps() {
        return taps_;
    }

    static constexpr std::uint64_t len() {
        return len_;
    }
};
