#ifndef LAB4_LN_HPP
#define LAB4_LN_HPP

class LN {
public:
    explicit LN(long long = 0);

    LN(const LN &);

    LN(LN &&) noexcept;

    explicit LN(std::string_view);

    explicit LN(const char *);

    ~LN();

    LN &operator=(const LN &);

    LN &operator=(LN &&) noexcept;

    LN operator+(const LN &) const;

    LN operator-(const LN &) const;

    LN operator*(const LN &) const;

    LN operator/(const LN &) const;

    LN operator%(const LN &) const;

    LN operator~() const;

    LN operator-() const;

    LN &operator+=(const LN &);

    LN &operator-=(const LN &);

    LN &operator*=(const LN &);

    LN &operator/=(LN);

    LN &operator%=(LN);

    LN operator<(const LN &) const;

    LN operator<=(const LN &) const;

    LN operator>(const LN &) const;

    LN operator>=(const LN &) const;

    LN operator==(const LN &) const;

    LN operator!=(const LN &) const;

    explicit operator long long() const;

    explicit operator bool() const;

    explicit operator std::string() const;

    LN &changeSign();

private:
    bool NaN_;
    int sign_;
    size_t size_;
    size_t capacity_;
    int *digits_;
    static const long long delta_ = 20;
    constexpr static int llMax_[5] = {5807, 5477, 368, 3372, 922};
    constexpr static int llMin_[5] = {5808, 5477, 368, 3372, 922};

    explicit LN(bool isNaN);

    LN operator<<(long long) const;

    LN operator>>(long long) const;

    void setCapacity(size_t);

    void pushBack(int);

    int cmp(const LN &) const;

    int absCmp(const LN &) const;

    bool isZero() const;

    bool isOne() const;

    bool isAbsOne() const;

    LN &absAdd(const LN &);

    LN &absSub(const LN &);

    int &at(size_t);

    void removeLeadingZeros();

    void error(const std::string &) const;
};

LN operator "" _ln(const char *str);

#endif //LAB4_LN_HPP
