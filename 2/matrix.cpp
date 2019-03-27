#include "matrix.h"
#include <stdexcept>

Matrix::Matrix(const size_t rows_number, const size_t columns_number) : rows(rows_number), columns(columns_number), data(new int*[rows_number]()) {
    for(int i = 0; i < rows_number; i++)
        *(data + i) = new int[columns_number]();
}

Matrix::~Matrix() {
    for(int i = 0; i < rows; i++)
        delete[] (*(data+i));
    delete[] data;
}

Matrix::Array::Array(int* new_data, size_t new_length) : data(new_data), length(new_length) {}

Matrix::Array::~Array() {}

Matrix& Matrix::operator*= (const int number) {
    for(size_t i = 0; i < rows; i++)
        for(size_t j = 0; j < columns; j++)
            data[i][j] *= number;
    return *this;
}

bool Matrix::operator== (const Matrix &other) const {
    if (this == &other)
        return true;
    if(columns != other.columns || rows != other.rows)
        return false;
    for(int i = 0; i < rows; i++)
        for(int j = 0; j < columns; j++)
            if(data[i][j] != other.data[i][j])
                return false;
    return true;
}

bool Matrix::operator!= (const Matrix &other) const {
    return !(*this == other);
}

int Matrix::Array::operator[] (const int index) const {
    if(index >= 0 && index < length) {
        int elem =  *(data + index);
        delete this;
        return elem;
    }
    delete this;
    throw std::out_of_range("Index is out of range!");
}

const Matrix::Array& Matrix::operator[] (const int index) const {
    if(index >= 0 && index < rows)
        return *(new Array(*(data + index), columns));
    throw std::out_of_range("Index is out of range!");
}

int& Matrix::Array::operator[] (const int index) {
    if(index >= 0 && index < length) {
        int& elem =  *(data + index);
        delete this;
        return elem;
    }
    delete this;
    throw std::out_of_range("Index is out of range!");
}

Matrix::Array& Matrix::operator[] (const int index) {
    if(index >= 0 && index < rows)
        return *(new Array(*(data + index), columns));
    throw std::out_of_range("Index is out of range!");
}

size_t Matrix::getColumns() {
    return columns;
}
size_t Matrix::getRows() {
    return rows;
}