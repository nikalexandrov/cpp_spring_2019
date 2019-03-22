#include "matrix.h"
#include <stdexcept>

Matrix::Matrix(const int rows_number, const int columns_number) : rows(rows_number), columns(columns_number), data(new int*[rows_number]()) {
    for(int i = 0; i < rows_number; i++)
        *(data + i) = new int[columns_number]();
}
Matrix::~Matrix() {
    for(int i = 0; i < rows; i++)
        delete[] *(data+i);
    delete[] data;
}

Matrix::Array::Array(int* new_data, int new_length) : data(new_data), length(new_length) {}
Matrix::Array::~Array() {}

const Matrix& Matrix::operator*= (const int number) {
    for(int i = 0; i < rows; i++)
        for(int j = 0; j < columns; j++)
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

int& Matrix::Array::operator[] (const int index) const {
    if(index >= 0 && index < length)
        return *(data + index);
    throw std::out_of_range("Index is out of range!");
}

Matrix::Array& Matrix::operator[] (const int index) const {
    if(index >= 0 && index < rows)
        return *new Array(*(data + index), columns);
    throw std::out_of_range("Index is out of range!");
}

int Matrix::getColumns() {
    return columns;
}
int Matrix::getRows() {
    return rows;
}