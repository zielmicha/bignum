// awesome template bignum library by Michal Zielinski
#include <vector>
#include <iostream>
#include <cstdint>
#include <iomanip>
#include <string>
#include <algorithm>

template <int MOD, typename T>
struct _SimpleDivMod {
    const static int mod = MOD;

    void operator()(T a, T& div, T& mod) const {
        div = a / MOD;
        mod = a % MOD;
    }
};

template <typename DIVMOD, typename T>
struct _OverflowDigit {
    const static DIVMOD divmod;
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

    Self sub(Self other) {
        val -= other.val;
        T carry;
        divmod(val, carry, val);
        if(val < 0) {
            val += DIVMOD::mod;
            carry -= 1;
        }
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

    bool operator!=(Self other) const {
        return !(*this == other);
    }

    bool operator<(Self other) const {
        return val < other.val;
    }

    bool operator>(Self other) const {
        return val > other.val;
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

    bool is_zero() const {
        for(D digit: digits)
            if(digit != D()) return false;
        return true;
    }

    int compare(const _BasicBignum<D>& other) const {
        if(other.is_zero() && is_zero())
            return 0;
        if(other.negative && !negative)
            return 1;
        if(!other.negative && negative)
            return -1;
        int negativitymod = negative ? -1 : 1;
        unsigned msize = std::max(other.digits.size(), digits.size());
        for(unsigned i=0; i < msize; i++) {
            D sdigit = i < digits.size() ? digits[i] : D();
            D odigit = i < other.digits.size() ? other.digits[i] : D();
            if(sdigit == odigit) continue;
            if(sdigit < odigit)
                return -1 * negativitymod;
            if(sdigit > odigit)
                return 1 * negativitymod;
        }
        return 0;
    }

    void base_add(const _BasicBignum<D>& other) {
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

    void base_sub(_BasicBignum<D> other) {
        bool new_negative = negative;
        this->negative = false;
        other.negative = false;
        if(*this < other) {
            new_negative = !new_negative;
            std::swap(*this, other);
        }
        negative = new_negative;

        // *this >= other
        // then: digits.size() >= other.digits.size()
        other.digits.resize(digits.size());

        D carry;
        for(unsigned i=0; i < digits.size(); i ++) {
            D carry1 = digits[i].add(carry);
            std::cerr << "digit " << digits[i].val
                      << " other " << other.digits[i].val
                      << std::endl;
            if(i < other.digits.size()) {
                D carry2 = digits[i].sub(other.digits[i]);
                carry1.add(carry2); // guaranteed to not carry over
            }
            carry = carry1;
        }
    }

    _BasicBignum<D>& operator+=(const _BasicBignum<D>& other) {
        if(other.negative == negative) {
            base_add(other);
        } else {
            base_sub(other);
        }
        return *this;
    }

    _BasicBignum<D>& operator-=(const _BasicBignum<D>& other) {
        if(other.negative != negative) {
            base_add(other);
        } else {
            base_sub(other);
        }
        return *this;
    }
};

template <typename T>
T operator+(T self, T other) {
    static_assert(
        std::is_base_of<_BasicBignum<typename T::DigitType>, T>::value,
        "this operator only supports bignums");
    T copy = self;
    copy += other;
    return copy;
}

template <typename T>
T operator-(T self, T other) {
    static_assert(
        std::is_base_of<_BasicBignum<typename T::DigitType>, T>::value,
        "this operator only supports bignums");
    T copy = self;
    copy -= other;
    return copy;
}

template <typename T>
bool operator==(const T& self, const T& other) {
    static_assert(
        std::is_base_of<_BasicBignum<typename T::DigitType>, T>::value,
        "this operator only supports bignums");
    return self.compare(other) == 0;
}

template <typename T>
bool operator<(const T& self, const T& other) {
    static_assert(
        std::is_base_of<_BasicBignum<typename T::DigitType>, T>::value,
        "this operator only supports bignums");
    return self.compare(other) < 0;
}

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
        if(init[0] == '-') {
            this->negative = true;
            init = init.substr(1, init.size() - 1);
        }
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

        Self zero;
        if(self == zero) { // prevent negative zero
            os << "0";
            return os;
        }

        if(self.negative)
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

template <class T>
struct _EvilNumberWrapper: T {
    // Seriously broken!
    int operator<(const _EvilNumberWrapper<T>& other) {
        return compare(other);
    }

    using T::T;
};

typedef _EvilNumberWrapper<_number> number;

int main() {
    number n("123456789");
    std::cout << n << std::endl;
    auto b = number{"123"} + number{"90"};
    std::cout << b << std::endl;
    std::cout << number{"90"} + number{"90"} << std::endl;
    for(int i=99; i < 102; i++) {
        number n {std::to_string(i)};
        std::cout << i << " ~ " << n << " -> " << number{"-90"}
        + n << std::endl;
    }
}
