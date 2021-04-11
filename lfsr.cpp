#include "lfsr.hpp"
#include <random>
#include <vector>
#include "BitStreamTests.hpp"

template<int len, int taps>
void f(std::uint64_t iv, int count, std::ostream &o, std::vector<char> &v) {
    Lfsr<len, taps> lfsr(iv);

    o << " |" << lfsr << std::endl;

    for (int i = 0; i < count; ++i) {
        v.push_back(lfsr.next() ? '1' : '0');
        o << v.back() << '|' << lfsr << std::endl;
    }
}

void unit6_question() {
    Lfsr<5, 0b11000> lfsr1(0b11101);
    std::vector<char> result2, result3, result4a, result4b;

    std::cout << "result2" << std::endl;
    f<5, 0b11000>(0b11101, 10, std::cout, result2);
    std::cout << "result3" << std::endl;
    f<5, 0b11000>(0b00110, 10, std::cout, result3);
    std::cout << "result4a" << std::endl;
    for (auto i = result2.begin(), j = result3.begin(); i != result2.end(); ++i, ++j) {
        result4a.push_back(*i == *j ? '0' : '1');
    }
    std::copy(result4a.begin(), result4a.end(), std::ostream_iterator<char>(std::cout));
    std::cout << std::endl;
    std::cout << "result4b" << std::endl;
    f<5, 0b11000>(0b11011, 10, std::cout, result4b);
}

int main() {
    std::mt19937 prng;
    prng.seed(std::random_device()());
    std::uniform_int_distribution<std::uint64_t> d(1, (1 << 21) - 1);

    unit6_question();

    typedef Lfsr<24> LfsrType;
    LfsrType lfsr(d(prng));

    {
        PokerTest<5> pokerTest(std::bind(&LfsrType::next, &lfsr));
        for (int i = 0; i < 1 << 20; ++i) {
            pokerTest.extractObservation();
        }
        std::cout << "poker test: " << std::endl;
        std::cout << pokerTest << std::endl;
        std::cout << "chi-squared confidence-value = " << pokerTest.chiSquaredPValue() << std::endl;
    }

    {
        SerialTest<5> serialTest(std::bind(&LfsrType::next, &lfsr));
        for (int i = 0; i < 1 << 20; ++i) {
            serialTest.extractObservation();
        }
        std::cout << "serial test: " << std::endl;
        std::cout << serialTest << std::endl;
        std::cout << "chi-squared confidence-value = " << serialTest.chiSquaredPValue() << std::endl;
    }

    return EXIT_SUCCESS;
}