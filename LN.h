#include <cstdint>
#include <iostream>


class LN {

public:
    LN(long long = 0);

    LN(const LN &);

    LN(LN &&) noexcept;

    explicit LN(std::string_view);

    explicit LN(const char *);

    LN(int i);

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

    LN &operator/=(const LN &);

    LN &operator%=(const LN &);

    LN operator<(const LN &) const;

    LN operator<=(const LN &) const;

    LN operator>(const LN &) const;

    LN operator>=(const LN &) const;

    LN operator==(const LN &) const;

    LN operator!=(const LN &) const;

    explicit operator long long() const;

    explicit operator bool() const;

    explicit operator std::string() const;

private:
    uint32_t *digits_ = nullptr;
    bool isNaN_ = false;
    int sign_ = 1;
    size_t len_ = 0;

    void shift_for_one_sign(uint32_t x);

    void delete_zero();

    [[nodiscard]] int compareDigits(const LN &x) const;

    void add(const LN &);
    void sub(const LN &);
};



