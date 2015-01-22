// awesome template bignum library by Michal Zielinski
#include <vector>
#include <iostream>
#include <cstdint>
#include <iomanip>
#include <string>
#include <algorithm>

template <int MOD, typename T>
struct _SimpleDivMod {
    void operator()(T a, T& div, T& mod) const {
        div = a / MOD;
        mod = a % MOD;
    }
};

template <typename DIVMOD, typename T>
struct _OverflowDigit {
    static DIVMOD divmod;
    T val;

    typedef _OverflowDigit<DIVMOD, T> Self;

    _OverflowDigit(T val): val(val) {}
    _OverflowDigit(): val() {}
    _OverflowDigit(const Self& o): val(o.val) {}

    Self add(Self other) {
        val += other.val;
        T carry;
        divmod(val, carry, val);
        return Self(carry);
    }

    Self mul(Self other) {
        val *= other.val;
        T carry;
        divmod(val, carry, val);
        return Self(carry);
    }

    bool operator==(Self other) const {
        return val == other.val;
    }
};

template <int MOD, typename T>
using _SimpleDivmodOverflowDigit =
    _OverflowDigit<_SimpleDivMod<MOD, T>, T>;

template <typename D>
struct _BasicBignum {
    typedef D DigitType;

    bool negative = false;
    std::vector<D> digits;

    void operator+=(const _BasicBignum<D>& other) {
        if(digits.size() < other.digits.size()) {
            digits.resize(other.digits.size());
        }
        D carry;
        for(unsigned i=0; i < digits.size(); i ++) {
            D carry1 = digits[i].add(carry);
            if(i < other.digits.size()) {
                D carry2 = digits[i].add(other.digits[i]);
                carry1.add(carry2); // guaranteed to not carry over
            }
            carry = carry1;
        }
        if(carry != D()) {
            digits.push_back(carry);
        }
    }
};

template <int A, int B>
struct _StaticPow {
    const static int value = A * _StaticPow<A, B - 1>::value;
};

template <int A>
struct _StaticPow<A, 0> {
    const static int value = 1;
};

template <int B>
struct _SmallestInt {
    typedef typename _SmallestInt<B + 1>::signed_type signed_type;
    typedef typename _SmallestInt<B + 1>::unsigned_type unsigned_type;
};

template <>
struct _SmallestInt<16> {
    typedef int16_t signed_type;
    typedef uint16_t unsigned_type;
};

template <>
struct _SmallestInt<32> {
    typedef int32_t signed_type;
    typedef uint32_t unsigned_type;
};

template <>
struct _SmallestInt<64> {
    typedef int64_t signed_type;
    typedef uint64_t unsigned_type;
};

template <int CNT>
using _DecimalBignumSuper = _BasicBignum<_SimpleDivmodOverflowDigit<_StaticPow<10, CNT>::value, typename _SmallestInt<CNT * 7 + 1>::signed_type > >;

template <typename T>
T str_to_int(const std::string& s) {
    return (T)std::stoll(s);
}

template <int CNT>
struct _DecimalBignum: public _DecimalBignumSuper<CNT> {
    typedef _DecimalBignum<CNT> Self;

    _DecimalBignum(std::string init) {
        int first_slice = init.size() % CNT;
        if(first_slice != 0)
            this->digits.push_back(
                str_to_int<typename Self::DigitType>(
                    init.substr(0, first_slice)));
        int start = first_slice;
        for(; start < init.size(); start += CNT) {
            this->digits.push_back(
                str_to_int<typename Self::DigitType>(
                    init.substr(start, CNT)));
        }
        std::reverse(this->digits.begin(), this->digits.end());
    }

    _DecimalBignum() {}

    friend std::ostream& operator<< (std::ostream& os,
                                     const _DecimalBignum<CNT>& self) {
        bool anything = false;
        bool first = true;

        if(self.negative) // negative zero?
            os << "-";

        auto& digits = self.digits;
        for(int i=digits.size() - 1; i >= 0; i --) {
            auto digit = digits[i];
            int pad;
            if(first) {
                if(digit.val == 0) continue;
                first = false;
                pad = 0;
            } else {
                pad = CNT;
            }
            os << std::setfill('0') << std::setw(pad) << digit.val;
            anything = true;
        }

        if(!anything) {
            os << "0";
        }
        return os;
    }

    friend std::istream& operator>>(std::istream& stream,
                                   _DecimalBignum<CNT>& n) {
        std::string data;
        stream >> data;
        n = _DecimalBignum<CNT>(data);
        return stream;
    }
};

typedef _DecimalBignum<2> _number;

template <typename T>
struct _SeriouslyBrokenBignum: T {
    int operator<(const T& other) {
        return compare(other);
    }
};

typedef _SeriouslyBrokenBignum<_number> number;

int main() {
    _number n("123456789");
    std::cout << n << std::endl;
}
