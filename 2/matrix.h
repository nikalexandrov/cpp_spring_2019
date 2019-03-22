class Matrix {
    int rows, columns;
    int **data;
public:
    Matrix(const int rows, const int columns);
    ~Matrix();
    class Array {
        int length;
        int *data;
    public:
        Array(int*, int);
        ~Array();
        int& operator[] (const int) const;
    };
    int getRows();
    int getColumns();
    Array& operator[] (const int) const;
    const Matrix& operator*=(const int);
    bool operator==(const Matrix&) const;
    bool operator!=(const Matrix&) const;
};