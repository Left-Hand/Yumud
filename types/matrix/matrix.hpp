#ifndef __MATRIX_HPP__

#define __MATRIX_HPP__

#include <vector>
#include <initializer_list>
#include <algorithm>
#include <cstdint>

#include "../real.hpp"

typedef real_t mat_real_t;

class Matrix {
private:
    std::vector<std::vector<mat_real_t>> data;
    size_t rows, cols;

	void Copy(mat_real_t* A, int n, int m, mat_real_t* B);
	void Multiply(mat_real_t* A, mat_real_t* B, int m, int p, int n, mat_real_t* C);
	void Add(mat_real_t* A, mat_real_t* B, int m, int n, mat_real_t* C);
	void Subtract(mat_real_t* A, mat_real_t* B, int m, int n, mat_real_t* C);
	void Transpose(mat_real_t* A, int m, int n, mat_real_t* C);
	void Scale(mat_real_t* A, int m, int n, mat_real_t k, mat_real_t* C);
	int Invert(mat_real_t* A, int n);

public:
    template <typename real>
    Matrix(std::initializer_list<real> row);

    template <typename real>
    Matrix(std::initializer_list<std::initializer_list<real>> matrix);

};

#include "matrix.tpp"

#endif