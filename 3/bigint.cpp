#include "bigint.h"

// вспомогательные функции: 
// вычисляем основание системы счисления
const uint64_t BigInt::max_size() {
    uint64_t res = 1;
    while(res * 10 > res)
        res *= 10;
    return res;
}
// сравниваем по абсолютному значению (this > another -> true, else -> false)
const bool BigInt::abs_bigger_than(const BigInt& another) const {
    if(length == another.length) {
        for(int i = 0; i < length; i++)
            if(number[i] != another.number[i])
                return (number[i] > another.number[i]);
        return false;
    } else return (length > another.length);
}
// конструкторы
BigInt::BigInt() : number(new uint64_t[mem_increment]), length(1), positive(true) {
    for(int i = 0; i < mem_increment; i++)
        number[i] = 0;
}
BigInt::BigInt(const int64_t num) : number(new uint64_t[(mem_increment > 1 ? mem_increment : 2)]), length(1), positive(num >= 0 ? true : false) {
    number[0] = (positive ? num : -num) % max_size();
    number[1] = (positive ? num : (-num)) / max_size();
    for(int i = 2; i < mem_increment; i++)
        number[i] = 0;
}

BigInt::BigInt(std::size_t length_, bool pos) : number(new uint64_t[length_]), length(length_), positive(pos) {
    for(int i = 0; i < length; i++)
        number[i] = 0;
}
// присваивание
BigInt& BigInt::operator= (const BigInt& another) {
    if(number)
        delete[] number;
    length = another.length;
    const std::size_t arr_size = sizeof(another.number) / sizeof(uint64_t);
    number = new uint64_t[arr_size];
    for(int i = 0; i < arr_size; i++)
        number[i] = another.number[i];
    return *this;
}
BigInt::BigInt(const BigInt& num) : number(nullptr), length(sizeof(num.number) / sizeof(uint64_t)), positive(num.positive) {
    number = new uint64_t[length];
    for(int i = 0; i < length; i++)
        number[i] = num.number[i];
    length = num.length;
}
// унарный минус
BigInt BigInt::operator- () const {
    BigInt res = *this;
    if(res != 0)
        res.positive = !positive;
    return res;
}
// арифметические операторы между BigInt
BigInt BigInt::operator+ (const BigInt& num) const {
    const std::size_t len = length > num.length ?
                            sizeof(number) / sizeof(uint64_t) + (sizeof(number) / sizeof(uint64_t) == length) * mem_increment :
                            sizeof(num.number) / sizeof(uint64_t) + (sizeof(num.number) / sizeof(uint64_t) == num.length) * mem_increment;
    BigInt res(len, true);
    int flag = false;
    if(positive == num.positive) {
        res.positive = positive;
        for(int i = 0; i < num.length; i++) {
            res.number[i] = number[i] + num.number[i] + flag;
            flag = false;
            if(res.number[i] >= max_size()) {
                flag = true;
                res.number[i] -= max_size();
            }
        }
    } else {
        res.positive = abs_bigger_than(num) ? positive : num.positive;
        if(res.positive == positive) {
            for(int i = 0; i < length; i++) {
                if(number[i] >= (num.number[i] + flag)) {
                    res.number[i] = number[i] - num.number[i] - flag;
                    flag = false;

                } else {
                    res.number[i] = max_size() + number[i] - num.number[i] - flag;
                    flag = true;
                }
            }
        } else {
            for(int i = 0; i < num.length; i++) {
                if(num.number[i] >= (number[i] + flag)) {
                    res.number[i] = num.number[i] - number[i] - flag;
                    flag = false;
                } else {
                    res.number[i] = max_size() + num.number[i] - number[i] - flag;
                    flag = true;
                }
            }
        }
    }
    res.number[length] = flag;
    int decr_len = 1;
    while(decr_len < len && res.number[len - decr_len] == 0)
        decr_len++;
        res.length -= (decr_len - 1);
    if(res == 0)
        res.positive = true;
    return res;
}
BigInt BigInt::operator- (const BigInt& num) const {
    return operator+ (-num);
}
// арифметические операторы между BigInt и int
BigInt BigInt::operator+ (const int num) const {
    return (*this + BigInt(num));
}
BigInt BigInt::operator- (const int num) const {
    return (*this - BigInt(num));
}
BigInt operator+ (const int first, const BigInt& second) {
    return BigInt(first) + second;
}
BigInt operator- (const int first, const BigInt& second) {
    return BigInt(first) - second;
}
// операторы сравнения между BigInt
bool BigInt::operator== (const BigInt& num) const {
    if(positive == num.positive && length == num.length) {
        for(int i = 0; i < length; i++)
            if(number[i] != num.number[i])
                return false;
        return true;
    } else return false;
}
bool BigInt::operator!=(const BigInt& num) const {
    return !(*this == num);
}
bool BigInt::operator>(const BigInt& num) const {
    if(positive == num.positive) {
        if(length != num.length)
            return positive ? (length > num.length) : (length < num.length);
        else {
            for(int i = length - 1; i >= 0; i--)
                if(number[i] != num.number[i])
                    return positive ? (number[i] > num.number[i]) : (number[i] < num.number[i]);
            return false;
        }
    } else return positive;
}
bool BigInt::operator<(const BigInt& num) const {
    if(positive == num.positive) {
        if(length != num.length)
            return positive ? (length < num.length) : (length > num.length);
        else {
            for(int i = length - 1; i >= 0; i--)
                if(number[i] != num.number[i])
                    return positive ? (number[i] < num.number[i]) : (number[i] > num.number[i]);
            return false;
        }
    } else return !positive;
}
bool BigInt::operator>=(const BigInt& num) const {
    return !(*this < num);
}
bool BigInt::operator<=(const BigInt& num) const {
    return !(*this > num);
}

// операторы сравнения между int и BigInt
bool BigInt::operator==(const int num) const {
    if(length > 1 || number[0] != num)
        return false;
    return true;
}
bool BigInt::operator!=(const int num) const {
    return !(*this == num);
}
bool BigInt::operator>(const int num) const {
    if(length > 1 || number[0] > num)
        return true;
    return false;
    
}
bool BigInt::operator<(const int num) const {
    if(length > 1 || number[0] >= num)
        return false;
    return true;
}
bool BigInt::operator>=(const int num) const {
    return !(*this < num);
}
bool BigInt::operator<=(const int num) const {
    return !(*this > num);
}

bool operator==(const int first, const BigInt& second) {
    return (second == first);
}
bool operator!=(const int first, const BigInt& second) {
    return (second != first);
}
bool operator>(const int first, const BigInt& second) {
    return (second < first);
}
bool operator<(const int first, const BigInt& second) {
    return (second > first);
}
bool operator>=(const int first, const BigInt& second) {
    return (second <= first);
}
bool operator<=(const int first, const BigInt& second) {
    return (second >= first);
}

// оператор вывода в поток
std::ostream& operator<<(std::ostream& s, const BigInt& num) {
    if(num == 0)
        s << "0";
    else{
        s << (num.positive ? "": "-");
        for(int i = num.length - 1; i >= 0; i--)
            if(num.number[i] != 0)
                s << num.number[i];
            else
                for(uint64_t i = 1; i < BigInt::max_size(); i *= 10)
                    s << "0";
    }
    return s;
}

// деструктор
BigInt::~BigInt(){
    delete[] number;
}