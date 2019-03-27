#pragma once
#include <cstddef>
using std::size_t;

class Matrix {
    int **data;
    size_t rows, columns;
public:
    Matrix(const std::size_t rows, const std::size_t columns);
    ~Matrix();
    class Array {
        int *data;
        size_t length;
    public:
        Array(int*, size_t);
        ~Array();
        int operator[] (const int) const;
        int& operator[] (const int);
    };
    size_t getRows();
    size_t getColumns();
    const Array& operator[] (const int) const;
    Array& operator[] (const int);
    Matrix& operator*= (const int);
    bool operator== (const Matrix&) const;
    bool operator!= (const Matrix&) const;
};