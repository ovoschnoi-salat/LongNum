#include <sstream>
#include <charconv>
#include "LN.hpp"

#define BASE 10000

using namespace std;

LN::LN(long long a) : LN(false) {
    setCapacity(10);
    if (a == 0) pushBack(0);
    else {
        if (a < 0) {
            sign_ = -1;
            while (a != 0) {
                pushBack(-(a % BASE));
                a /= BASE;
            }
        } else {
            while (a != 0) {
                pushBack(a % BASE);
                a /= BASE;
            }
        }
    }
    removeLeadingZeros();
}

LN::LN(const LN &src) : LN(false) {
    *this = src;
}

LN::LN(LN &&src) noexcept {
    NaN_ = src.NaN_;
    size_ = src.size_;
    sign_ = src.sign_;
    digits_ = src.digits_;
    capacity_ = src.capacity_;
    src.digits_ = nullptr;
}

LN::LN(string_view str) : LN(false) {
    if (str == "NaN" || str.empty()) NaN_ = true;
    else {
        if (str[0] != '-' && str[0] != '+' && (str[0] < '0' || str[0] > '9'))
            error("Invalid string passed to constructor: \"" + string(str) + "\".");
        for (size_t i = 1; i < str.size(); ++i) {
            if (str[i] < '0' || str[i] > '9') error("Invalid string passed to constructor: \"" + string(str) + "\".");
        }
        long long i = str.length();
        setCapacity(i / 4 + 1);
        int a;
        for (i -= 4; i > 0; i -= 4) {
            from_chars(str.data() + i, str.data() + i + 4, a);
            pushBack(a);
        }
        if (str[0] == '-' || str[0] == '+') {
            if (str[0] == '-') {
                sign_ = -1;
            }
            if (i + 4 > 1) {
                from_chars(str.data() + 1, str.data() + i + 4, a);
                pushBack(a);
            }
        } else {
            from_chars(str.data(), str.data() + i + 4, a);
            pushBack(a);
        }
    }
    removeLeadingZeros();
}

LN::LN(const char *str) : LN(string_view(str)) {}

LN::~LN() {
    free(digits_);
}

LN &LN::operator=(const LN &src) {
    if (&src == this) return *this;
    NaN_ = src.NaN_;
    size_ = src.size_;
    sign_ = src.sign_;
    if (capacity_ != src.capacity_) setCapacity(src.capacity_);
    memcpy(digits_, src.digits_, size_ * sizeof(int));
    return *this;
}

LN &LN::operator=(LN &&src) noexcept {
    NaN_ = src.NaN_;
    size_ = src.size_;
    sign_ = src.sign_;
    free(digits_);
    digits_ = src.digits_;
    capacity_ = src.capacity_;
    src.digits_ = nullptr;
    return *this;
}

LN LN::operator+(const LN &addendum) const {
    if (NaN_ || addendum.NaN_) return LN(true);
    if (isZero()) return addendum;
    if (addendum.isZero()) return *this;
    if (sign_ == addendum.sign_) return LN(*this).absAdd(addendum);
    else return LN(*this).absSub(addendum);
}

LN LN::operator-(const LN &subtrahend) const {
    if (NaN_ || subtrahend.NaN_) return LN(true);
    if (isZero()) return -subtrahend;
    if (subtrahend.isZero()) return *this;
    if (sign_ == subtrahend.sign_) return LN(*this).absSub(subtrahend);
    else return LN(*this).absAdd(subtrahend);
}

LN LN::operator*(const LN &multiplier) const {
    if (NaN_ || multiplier.NaN_) return LN(true);
    if (multiplier.isZero() || isZero()) return LN();
    if (multiplier.isAbsOne()) {
        if (multiplier.sign_ == -1) return -*this;
        return *this;
    }
    if (isAbsOne()) {
        if (sign_ == -1) return -multiplier;
        return multiplier;
    }
    LN res;
    if (multiplier.size_ == 1) {
        res.setCapacity(size_ + multiplier.size_);
        int carry = 0;
        for (size_t i = 0; i < size_; ++i) {
            int cur = res.at(i) + digits_[i] * multiplier.digits_[0] + carry;
            res.at(i) = cur % BASE;
            carry = cur / BASE;
        }
        if (carry) res.at(size_) = carry;
    } else {
        res.setCapacity(size_ + multiplier.size_);
        for (size_t i = 0; i < size_; ++i) {
            int carry = 0;
            size_t j = 0;
            for (; j < multiplier.size_; ++j) {
                int cur = res.at(i + j) + digits_[i] * multiplier.digits_[j] + carry;
                res.at(i + j) = cur % BASE;
                carry = cur / BASE;
            }
            while (carry) {
                int cur = res.at(i + j) + carry;
                res.at(i + j) = cur % BASE;
                carry = cur / BASE;
                j++;
            }
        }
    }
    res.sign_ = sign_ * multiplier.sign_;
    res.removeLeadingZeros();
    return res;
}

LN LN::operator/(const LN &divisor) const {
    if (NaN_ || divisor.NaN_ || divisor.isZero()) return LN(true);
    if (isZero() || absCmp(divisor) == -1) return LN();
    if (absCmp(divisor) == 0) {
        if (sign_ == divisor.sign_) return LN(1ll);
        return LN(-1ll);
    }
    return LN(*this) /= divisor;
}

LN LN::operator%(const LN &divisor) const {
    if (NaN_ || divisor.NaN_ || divisor.isZero()) return LN(true);
    if (isZero() || absCmp(divisor) == 0) return LN();
    if (absCmp(divisor) < 0) return *this;
    return LN(*this) %= divisor;
}

LN LN::operator~() const {
    if (NaN_ || sign_ == -1) return LN(true);
    if (isZero()) return *this;
    size_t m = (size_ + 1) / 2;
    LN max("1" + string(m * 4, '0'));
    LN min("1" + string((m - 1) * 4, '0'));
    LN temp;
    LN TWO(2ll);
    do {
        temp = (max + min) / TWO;
        if (temp * temp <= *this) {
            min = temp;
        } else {
            max = temp;
        }
    } while (!(max - min).isOne());
    LN res;
    res = min;
    return res;
}

LN LN::operator-() const {
    if (isZero()) return *this;
    LN res(*this);
    res.sign_ *= -1;
    return res;
}

LN &LN::operator+=(const LN &addendum) {
    if (NaN_ || addendum.NaN_) return *this = LN(true);
    if (isZero()) return *this = addendum;
    if (addendum.isZero()) return *this;
    if (sign_ == addendum.sign_) return absAdd(addendum);
    else return absSub(addendum);
}

LN &LN::operator-=(const LN &subtrahend) {
    if (NaN_ || subtrahend.NaN_) return *this = LN(true);
    if (isZero()) return *this = -subtrahend;
    if (subtrahend.isZero()) return *this;
    if (sign_ == subtrahend.sign_) return absSub(subtrahend);
    else return absAdd(subtrahend);
}

LN &LN::operator*=(const LN &multiplier) {
    return *this = *this * multiplier;
}

LN &LN::operator/=(LN divisor) {
    if (NaN_ || divisor.NaN_ || divisor.isZero()) return *this = LN(true);
    if (isZero() || absCmp(divisor) == -1) return *this = LN();
    if (absCmp(divisor) == 0) {
        if (sign_ == divisor.sign_) return *this = LN(1ll);
        return *this = LN(-1ll);
    }
    sign_ *= divisor.sign_;
    if (divisor.size_ == 1) {
        int b = divisor.digits_[0];
        int carry = 0;
        for (size_t i = size_ - 1; i > 0; --i) {
            int cur = digits_[i] + carry * BASE;
            digits_[i] = cur / b;
            carry = cur % b;
        }
        digits_[0] = (digits_[0] + carry * BASE) / b;
    } else {
        size_t p = divisor.size_ - 1;
        LN divisible(*this);
        divisible.sign_ = 1;
        divisor.sign_ = 1;
        divisor = divisor << (size_ - divisor.size_);
        for (size_t i = size_ - 1; i >= p; --i) {
            int temp;
            int max = 10000;
            int min = 0;
            do {
                temp = (min + max) / 2;
                if (divisible.absCmp(divisor * LN((long long) temp)) >= 0) {
                    min = temp;
                } else max = temp;
            } while (max - min != 1);
            divisible -= divisor * LN((long long) min);
            digits_[i - p] = min;
            divisor = divisor >> 1;
        }
        size_ -= p;
    }
    removeLeadingZeros();
    return *this;
}

LN &LN::operator%=(LN divisor) {
    if (NaN_ || divisor.NaN_ || divisor.isZero()) return *this = LN(true);
    if (isZero() || absCmp(divisor) == 0) return *this = LN();
    if (absCmp(divisor) < 0) return *this;
    if (divisor.size_ == 1) {
        int b = divisor.digits_[0];
        int carry = 0;
        for (size_t i = size_ - 1; i > 0; --i) {
            carry = (digits_[i] + carry * BASE) % b;
        }
        carry = (digits_[0] + carry * BASE) % b;
        digits_[0] = carry;
        size_ = 1;
        removeLeadingZeros();
    } else {
        size_t p = divisor.size_ - 1;
        LN divisible(*this);
        divisible.sign_ = 1;
        divisor.sign_ = 1;
        divisor = divisor << (size_ - divisor.size_);
        for (size_t i = size_ - 1; i >= p; --i) {
            int temp;
            int max = 10000;
            int min = 0;
            do {
                temp = (min + max) / 2;
                if (divisible.absCmp(divisor * LN((long long) temp)) >= 0) {
                    min = temp;
                } else max = temp;
            } while (max - min != 1);
            divisible -= divisor * LN((long long) min);
            divisor = divisor >> 1;
        }
        divisible.sign_ = sign_;
        *this = divisible;
    }
    return *this;
}

LN LN::operator<(const LN &to) const {
    if (NaN_ || to.NaN_) return LN();
    if (cmp(to) < 0) return LN(1ll);
    return LN();
}

LN LN::operator<=(const LN &to) const {
    if (NaN_ || to.NaN_) return LN();
    if (cmp(to) <= 0) return LN(1ll);
    return LN();
}

LN LN::operator>(const LN &to) const {
    if (NaN_ || to.NaN_) return LN();
    if (cmp(to) > 0) return LN(1ll);
    return LN();
}

LN LN::operator>=(const LN &to) const {
    if (NaN_ || to.NaN_) return LN();
    if (cmp(to) >= 0) return LN(1ll);
    return LN();
}

LN LN::operator==(const LN &to) const {
    if (NaN_ || to.NaN_) return LN();
    if (cmp(to) == 0) return LN(1ll);
    return LN();
}

LN LN::operator!=(const LN &to) const {
    if (NaN_ || to.NaN_) return LN(1ll);
    if (cmp(to) != 0) return LN(1ll);
    return LN();
}

LN::operator long long() const {
    if (NaN_) error("NaN to long long int.");
    if (size_ > 5) error("Long number bigger than long long int.");
    if (size_ == 5) {
        if (sign_ == -1) {
            for (int i = 4; i >= 0; --i) {
                if (digits_[i] > llMin_[i]) error("Long number bigger than long long int.");
                if (digits_[i] < llMin_[i]) break;
            }
        } else {
            for (int i = 4; i >= 0; --i) {
                if (digits_[i] > llMax_[i]) error("Long number bigger than long long int.");
                if (digits_[i] < llMax_[i]) break;
            }
        }
    }
    long long res = 0;
    for (size_t i = size_ - 1; i > 0; --i) {
        res = res * BASE + digits_[i];
    }
    res = res * BASE + digits_[0];
    return res * sign_;
}

LN::operator bool() const {
    if (isZero()) return false;
    return true;
}

LN::operator std::string() const {
    stringstream str;
    if (NaN_) return "NaN";
    if (sign_ == -1) str << '-';
    if (size_ > 0) {
        str << to_string(digits_[size_ - 1]);
    }
    if (size_ > 1) {
        for (size_t i = size_ - 2; i > 0; --i) {
            string t = to_string(digits_[i]);
            str << string(4 - t.length(), '0') << t;
        }
        string t = to_string(digits_[0]);
        str << string(4 - t.length(), '0') << t;
    }
    return str.str();
}

LN &LN::changeSign() {
    if (isZero()) return *this;
    sign_ *= -1;
    return *this;
}

LN::LN(bool isNaN) {
    NaN_ = isNaN;
    sign_ = 1;
    size_ = 0;
    capacity_ = 0;
    digits_ = nullptr;
}

LN LN::operator<<(long long a) const {
    if (NaN_ || a == 0 || isZero()) return *this;
    if (a < 0) return *this >> -a;
    LN res(false);
    res.sign_ = sign_;
    res.size_ = size_ + a;
    res.setCapacity(size_ + a);
    memcpy(res.digits_ + a, digits_, size_ * sizeof(int));
    memset(res.digits_, 0, a * sizeof(int));
    return res;
}

LN LN::operator>>(long long a) const {
    if (NaN_ || a == 0 || isZero()) return *this;
    if (a < 0) return *this << -a;
    if (a >= size_) return LN();
    LN res(false);
    res.sign_ = sign_;
    res.size_ = size_ - a;
    res.setCapacity(size_ - a);
    memcpy(res.digits_, digits_ + a, (size_ - a) * sizeof(int));
    return res;
}

void LN::setCapacity(size_t n) {
    if (n > ULONG_MAX) error("Too many digits.");
    auto *newDigits = (int *) (realloc(digits_, n * sizeof(int)));
    if (newDigits == nullptr) error("Error occurred while allocating memory for digits.");
    capacity_ = n;
    digits_ = newDigits;
}

void LN::pushBack(int d) {
    if (size_ == capacity_)
        setCapacity(capacity_ + delta_ > capacity_ * 11 / 10 ? capacity_ + delta_ : capacity_ * 11 / 10);
    digits_[size_++] = d;
}

int LN::cmp(const LN &to) const {
    if (sign_ != to.sign_) return sign_;
    return absCmp(to) * sign_;
}

int LN::absCmp(const LN &to) const {
    if (size_ > to.size_) return 1;
    else if (size_ < to.size_) return -1;
    for (size_t i = size_ - 1; i > 0; --i) {
        if (digits_[i] > to.digits_[i]) return 1;
        if (digits_[i] < to.digits_[i]) return -1;
    }
    if (digits_[0] > to.digits_[0]) return 1;
    if (digits_[0] < to.digits_[0]) return -1;
    return 0;
}

bool LN::isZero() const {
    if (size_ == 1 && digits_[0] == 0) return true;
    return false;
}

bool LN::isOne() const {
    if (isAbsOne() && sign_ == 1) return true;
    return false;
}

bool LN::isAbsOne() const {
    if (size_ == 1 && digits_[0] == 1) return true;
    return false;
}

LN &LN::absAdd(const LN &addendum) {
    int p = 0;
    size_t s = min(size_, addendum.size_);
    for (size_t i = 0; i < s; ++i) {
        p += digits_[i] + addendum.digits_[i];
        digits_[i] = p % BASE;
        p /= BASE;
    }
    for (size_t i = addendum.size_; i < size_; ++i) {
        p += digits_[i];
        digits_[i] = p % BASE;
        p /= BASE;
    }
    for (size_t i = size_; i < addendum.size_; ++i) {
        p += addendum.digits_[i];
        pushBack(p % BASE);
        p /= BASE;
    }
    pushBack(p);
    removeLeadingZeros();
    return *this;
}

LN &LN::absSub(const LN &subtrahend) {
    if (absCmp(subtrahend) >= 0) {
        int p = 0;
        for (size_t i = 0; i < subtrahend.size_; i++) {
            p = digits_[i] - subtrahend.digits_[i] - p;
            digits_[i] = (p + BASE) % BASE;
            if (p < 0) p = 1;
            else p = 0;
        }
        for (size_t i = subtrahend.size_; i < size_; ++i) {
            p = digits_[i] - p;
            digits_[i] = (p + BASE) % BASE;
            if (p < 0) p = 1;
            else p = 0;
        }
    } else {
        int p = 0;
        for (size_t i = 0; i < size_; i++) {
            p = subtrahend.digits_[i] - digits_[i] - p;
            digits_[i] = ((p + BASE) % BASE);
            if (p < 0) p = 1;
            else p = 0;
        }
        for (size_t i = size_; i < subtrahend.size_; ++i) {
            p = subtrahend.digits_[i] - p;
            pushBack((p + BASE) % BASE);
            if (p < 0) p = 1;
            else p = 0;
        }
        sign_ *= -1;
    }
    removeLeadingZeros();
    return *this;
}

int &LN::at(size_t p) {
    if (p >= size_) {
        for (size_t i = size_; i <= p; ++i) {
            pushBack(0);
        }
    }
    return digits_[p];
}

void LN::removeLeadingZeros() {
    for (size_t i = size_ - 1; i > 0; --i) {
        if (digits_[i] == 0) size_--;
        else break;
    }
    if (size_ == 1 && digits_[0] == 0) sign_ = 1;
    setCapacity(size_);
}

void LN::error(const string &str) const {
    free(digits_);
    throw runtime_error(str);
}

LN operator "" _ln(const char *str) {
    return LN(str);
}