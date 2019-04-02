#pragma once
#include <cstddef>
#include <iostream>

class BigInt {
    uint64_t *number;
    std::size_t length;
    bool positive;
    static const std::size_t mem_increment = 10; // на сколько uint64_t выделяем памяти за раз
    static const uint64_t max_size();
    const bool abs_bigger_than(const BigInt&) const;
    BigInt(uint64_t*, std::size_t, bool);
public:
    BigInt();
    BigInt(const int64_t);
    BigInt(const BigInt&);

    BigInt& operator=(const BigInt&);
    
    BigInt operator- () const;
    
    BigInt operator+ (const BigInt&) const;
    BigInt operator- (const BigInt&) const;

    BigInt operator+ (const int) const;
    BigInt operator- (const int) const;
    friend BigInt operator+ (const int, const BigInt&);
    friend BigInt operator- (const int, const BigInt&);

    bool operator==(const BigInt&) const;
    bool operator!=(const BigInt&) const;
    bool operator>(const BigInt&) const;
    bool operator<(const BigInt&) const;
    bool operator>=(const BigInt&) const;
    bool operator<=(const BigInt&) const;

    bool operator==(const int) const;
    bool operator!=(const int) const;
    bool operator>(const int) const;
    bool operator<(const int) const;
    bool operator>=(const int) const;
    bool operator<=(const int) const;
    friend bool operator==(const int, const BigInt&);
    friend bool operator!=(const int, const BigInt&);
    friend bool operator>(const int, const BigInt&);
    friend bool operator<(const int, const BigInt&);
    friend bool operator>=(const int, const BigInt&);
    friend bool operator<=(const int, const BigInt&);

	friend std::ostream& operator<<(std::ostream&, const BigInt&);

    ~BigInt();
};