#pragma once

#include "memory.h"

#include "core/math/real.hpp"
#include "core/stream/ostream.hpp"


namespace ymd{
template<arithmetic T, size_t R, size_t C>
class Matrix{
private:
    __fast_inline constexpr Matrix(){}
public:
    static constexpr Matrix from_uninitialized(){
        return Matrix();
    }

    __fast_inline constexpr static Matrix from_zero(){
        Matrix result = Matrix::from_uninitialized();
        result.fill(0);
        return result;
    }

    // 单位矩阵生成函数
    __fast_inline constexpr static Matrix<T, R, C> from_identity(){
        static_assert(R == C, "Identity matrix must be square.");

        Matrix<T, R, C> result;

        for (size_t i = 0; i < R; ++i) {
            for (size_t j = 0; j < C; ++j) {
                result.at(i, j) = (i == j) ? T(1) : T(0);
            }
        }

        return result;
    }



    template<typename ... Args>
    requires (sizeof...(Args) == R * C) 
    __fast_inline constexpr explicit 
    Matrix(Args... args) {
        static_assert(sizeof...(args) == R * C);
        auto values = std::array<T, R * C>{{static_cast<T>(args)...}};
        std::copy(values.data(), values.end(), data());
    }


    __fast_inline constexpr Matrix& operator = (const Matrix & other){
        for(size_t i = 0; i < size(); i++){
            data()[i] = static_cast<T>(other.storage_[i]);
        }
        return *this;
    }


    __fast_inline constexpr void fill(const T val){
        auto ptr = data();
        for(size_t i = 0; i < size(); i++){
            ptr[i] = static_cast<T>(val);
        }
    }

    __fast_inline constexpr size_t rows() const { return R;}
    __fast_inline constexpr size_t cols() const { return C;}
    __fast_inline constexpr std::span<T, C> operator[](const size_t row) 
        {return std::span<T, C>(&storage_[row * C], C);}

    __fast_inline constexpr std::span<const T, C> operator[](const size_t row) const 
        {return std::span<const T, C>(&storage_[row * C], C);}

    __fast_inline constexpr T & at(const size_t row, const size_t col) 
        { return storage_[row * C + col];}
    __fast_inline constexpr const T & at(const size_t row, const size_t col) const 
        { return storage_[row * C + col];}
    __fast_inline constexpr size_t size() const { return R*C;}

    // __fast_inline constexpr T * data() { return static_cast<T *>(&data_[0]);}
    // __fast_inline constexpr const T * data() const 
    //     { return static_cast<const T *>(&data_[0]);}
    // __fast_inline constexpr T * end() { return data() + size();}
    // __fast_inline constexpr const T * end() const { return data() + size();}

    __fast_inline constexpr T * data() { return storage_.data();}
    __fast_inline constexpr const T * data() const { return storage_.data(); }

    // __fast_inline constexpr void setZero() {
    //     auto ptr = data();
    //     for(size_t i = 0; i < size(); i++){
    //         ptr[i] = static_cast<T>(0);
    //     }
    // }

    template<size_t R2, size_t C2>
    __fast_inline constexpr Matrix<T, R2, C2> block(
        const size_t row_start, const size_t col_start) const{   
        static_assert(R2 <= R and C2 <= C);

        if(std::is_constant_evaluated()){
            // size_t row_end = row_start + R2;
            // size_t col_end = col_start + C2;

            // static_assert(row_start <= row_end && row_end <= R);
            // static_assert(col_start <= col_end && col_end <= C);
        }

        Matrix<T, R2, C2> result;

        for (size_t i = 0; i < R2; ++i) {
            for (size_t j = 0; j < C2; ++j) {
                result.at(i, j) = this->at(row_start + i, col_start + j);
            }
        }

        return result;
    }

    template<arithmetic U>
    __fast_inline constexpr Matrix & operator += (const Matrix<U, R, C> & other){
        auto ptr = data();
        auto other_ptr = other.data();
        for(size_t i = 0; i < size(); i++){
            ptr[i] += static_cast<T>(other_ptr[i]);
        }

        return *this;
    }

    template<arithmetic U>
    __fast_inline constexpr Matrix operator + (const Matrix<U, R, C> & other) const {
        Matrix ret = Matrix::from_uninitialized();
        auto ptr = data();
        auto ret_ptr = ret.data();
        auto other_ptr = other.data();
        for(size_t i = 0; i < size(); i++){
            ret_ptr[i] = ptr[i] + static_cast<T>(other_ptr[i]);
        }

        return ret;
    }

    template<arithmetic U>
    __fast_inline constexpr Matrix & operator -= (const Matrix<U, R, C> & other){
        auto ptr = data();
        auto other_ptr = other.data();
        for(size_t i = 0; i < size(); i++){
            ptr[i] -= static_cast<T>(other_ptr[i]);
        }

        return *this;
    }

    template<arithmetic U>
    __fast_inline constexpr Matrix operator - (const Matrix<U, R, C> & other) const {
        Matrix ret = Matrix::from_uninitialized();
        auto ptr = data();
        auto ret_ptr = ret.data();
        auto other_ptr = other.data();
        for(size_t i = 0; i < size(); i++){
            ret_ptr[i] = ptr[i] - static_cast<T>(other_ptr[i]);
        }

        return ret;
    }

    __fast_inline constexpr Matrix operator - () const {
        Matrix ret = Matrix::from_uninitialized();
        auto ptr = data();
        auto ret_ptr = ret.data();
        for(size_t i = 0; i < size(); i++){
            ret_ptr[i] = -ptr[i];
        }

        return ret;
    }


    __fast_inline constexpr Matrix operator * (const arithmetic auto & scalar) const{
        Matrix ret = Matrix::from_uninitialized();
        auto ptr = data();
        auto ret_ptr = ret.data();
        for(size_t i = 0; i < size(); i++){
            ret_ptr[i] = ptr[i] * static_cast<T>(scalar);
        }
        return ret;
    }


    __fast_inline constexpr Matrix operator / (const arithmetic auto & scalar) const{
        Matrix ret = Matrix::from_uninitialized();
        auto ptr = data();
        auto ret_ptr = ret.data();
        for(size_t i = 0; i < size(); i++){
            ret_ptr[i] = ptr[i] / static_cast<T>(scalar);
        }
        return ret;
    }



    template<size_t C2>
    __fast_inline constexpr Matrix<T, R, C2> operator * (const Matrix<T, C, C2> & other) const{
        auto result = Matrix<T, R, C2>::from_uninitialized();
        for (size_t i = 0; i < R; i++) {
            for (size_t j = 0; j < C2; j++) {
                T sum = 0;
                for (size_t k = 0; k < C; k++) {
                    sum += this->at(i, k) * other.at(k, j);
                }
                result.at(i, j) = sum;
            }
        }
        return result;
    }


    __fast_inline constexpr Matrix<T, C, R> transpose() const{
        Matrix<T, C, R> result = Matrix<T, C, R>::from_uninitialized();
        for (size_t i = 0; i < R; i++) {
            for (size_t j = 0; j < C; j++) {
                result.at(j, i) = this->at(i, j);
            }
        }
        return result;
    }

    // __fast_inline constexpr Matrix<T, R, R> guassian_inverse() const{

    //     Matrix<T, R, 2 * R> W;
    //     Matrix<T, R, R> result;
    //     T tem_1, tem_2, tem_3;
    
    //     // 对矩阵右半部分进行扩增
    //     for(size_t i = 0;i < R; i++){
    //         for(size_t j = 0;j < 2 * R; j++){
    //             if(j<R){
    //                 W[i][j] = (T) (*this)[i][j];
    //             }
    //             else{
    //                 W[i][j] = (T) (j-R == i ? 1:0);
    //             }
    //         }
    //     }
    
    //     for(size_t i=0;i<R;i++)
    //     {
    //         // 判断矩阵第一行第一列的元素是否为0，若为0，继续判断第二行第一列元素，直到不为0，将其加到第一行
    //         if( ((int) W[i][i]) == 0)
    //         { 
    //             size_t j=i+1;
    //             for(;j<R;j++)
    //             {
    //                 if( ((int) W[j][i]) != 0 ) break;
    //             }
    //             if(j == R){
    //                 break;
    //             }
    //             //将前面为0的行加上后面某一行
    //             for(size_t k=0;k<2*R;k++)
    //             {
    //                 W[i][k] += W[j][k];
    //             }
    //         }
    
    //         //将前面行首位元素置1
    //         tem_1 = W[i][i];
    //         for(size_t j=0;j<2*R;j++)
    //         {
    //             W[i][j] = W[i][j] / tem_1;
    //         }
    
    //         //将后面所有行首位元素置为0
    //         for(size_t j=i+1;j<R;j++)
    //         {
    //             tem_2 = W[j][i];
    //             for(size_t k=i;k<2*R;k++)
    //             {
    //                 W[j][k] = W[j][k] - tem_2 * W[i][k];
    //             }
    //         }
    //     }
    
    //     // 将矩阵前半部分标准化
    //     for(size_t i=R-1;i>=0;i--)
    //     {
    //         for(size_t j=i-1;j>=0;j--)
    //         {
    //             tem_3 = W[j][i];
    //             for(size_t k=i;k<2*R;k++)
    //             {
    //                 W[j][k] = W[j][k] - tem_3*W[i][k];
    //             }
    //         }
    //     }
    
    //     //得出逆矩阵
    //     for(size_t i=0;i<R;i++)
    //     {
    //         for(size_t j=R;j<2*R;j++)
    //         {
    //             result[i][j-R] = W[i][j];
    //         }
    //     }
    //     return result;
    // }
    __fast_inline constexpr Matrix<T, R, R> lu_inverse() const{
        // https://blog.csdn.net/weixin_46207279/article/details/120374064

        T L[R][R], U[R][R], L_n[R][R], U_n[R][R];
        const auto & W = *this;
        Matrix<T, R, R> W_n;
        // int k, d;
        T s;
    
        // 赋初值
        for(size_t i=0;i<R;i++){
            for(size_t j=0;j<R;j++){
                L[i][j] = 0;
                U[i][j] = 0;
                L_n[i][j] = 0;
                U_n[i][j] = 0;
                W_n[i][j] = 0;
            }
        }
    
        for(size_t i=0;i<R;i++)  // L对角置1
        {
            L[i][i] = 1.0;
        }
    
        for(size_t j=0;j<R;j++)  
        {
            U[0][j] = W[0][j];
        }
    
        for(size_t i=1;i<R;i++)
        {
            L[i][0] = W[i][0] / U[0][0];
        }
    
        for(size_t i=1;i<R;i++)
        {
            for(size_t j=i;j<R;j++) // 求U
            {
                s = 0;
                for(size_t k=0;k<i;k++)
                {
                    s += L[i][k] * U[k][j];
                }
                U[i][j] = W[i][j] - s;
            }
    
            for(size_t d=i;d<R;d++) // 求L
            {
                s = 0;
                for(size_t k=0;k<i;k++)
                {
                    s += L[d][k] * U[k][i];
                }
                L[d][i] = (W[d][i] - s) / U[i][i];
            }
        }
    
        for(size_t j=0;j<R;j++)  //求L的逆
        {
            for(size_t i=j;i<R;i++)
            {
                if(i==j) 
                    L_n[i][j] = 1 / L[i][j];
                else if(i<j) 
                    L_n[i][j] = 0;
                else
                {
                    s = 0.;
                    for(size_t k=j;k<i;k++)
                    {
                        s += L[i][k] * L_n[k][j];
                    }
                    L_n[i][j] = -L_n[j][j] * s;
                }
            }
        }
    
        for(size_t i=0;i<R;i++)  //求U的逆
        {
            for(size_t j=i;j--;)
            {
                if(i==j)
                    U_n[j][i] = 1 / U[j][i];
                else if(j>i) 
                    U_n[j][i] = 0;
                else
                {
                    s = 0.;
                    for(size_t k=j+1;k<=i;k++)
                    {
                        s += U[j][k] * U_n[k][i];
                    }
                    U_n[j][i] = -1 / U[j][j] * s;
                }
            }
        }
    
    
        for(size_t i=0;i<R;i++)
        {
            for(size_t j=0;j<R;j++)
            {
                for(size_t k=0;k<R;k++)
                {
                    W_n[i][j] += U_n[i][k] * L_n[k][j];
                }
            }
        }

        return W_n;
    }

    template<typename U = T, typename std::enable_if_t<std::is_arithmetic_v<U>, int> = 0>
    __fast_inline constexpr Matrix<T, R, R> inverse() const{
        static_assert(R == C);
        // return this->transpose() / this->determinant();
        // return lu_inverse();
        Matrix<T, R, 2 * R> augmented = Matrix<T, R, 2 * R>::from_uninitialized();
        for (size_t i = 0; i < R; i++) {
            for (size_t j = 0; j < R; j++) {
                augmented.at(i, j) = this->at(i, j);
                augmented.at(i, j + R) = (i == j) ? 1 : 0;
            }
        }

        // Gauss-Jordan 消元
        for (size_t i = 0; i < R; i++) {
            // 寻找当前列的最大元素
            size_t max_row = i;
            for (size_t j = i + 1; j < R; j++) {
                if (std::abs(augmented.at(j, i)) > std::abs(augmented.at(max_row, i))) {
                    max_row = j;
                }
            }

            // 交换最大元素所在的行与当前行
            if (max_row != i) {
                for (size_t j = 0; j < 2 * R; j++) {
                    std::swap(augmented.at(i, j), augmented.at(max_row, j));
                }
            }

            // 将主元素设置为 1
            T pivot = augmented.at(i, i);
            for (size_t j = 0; j < 2 * R; j++) {
                augmented.at(i, j) /= pivot;
            }

            // 消除其他行的当前列元素
            for (size_t j = 0; j < R; j++) {
                if (j != i) {
                    T factor = augmented.at(j, i);
                    for (size_t k = 0; k < 2 * R; k++) {
                        augmented.at(j, k) -= factor * augmented.at(i, k);
                    }
                }
            }
        }

        // 提取逆矩阵部分
        Matrix<T, R, R> result;
        for (size_t i = 0; i < R; i++) {
            for (size_t j = 0; j < R; j++) {
                result.at(i, j) = augmented.at(i, j + R);
            }
        }

        return result;
    }

    __fast_inline constexpr
    const T & operator()(const size_t i, const size_t j) const {
        return this->at(i,j);
    }

    __fast_inline constexpr
    T determinant() const{
        static_assert(R == C, "Determinant can only be calculated for square matrices.");

        if constexpr (R == 1) {
            return this->at(0, 0);
        } else if constexpr (R == 2) {
            return this->at(0, 0) * this->at(1, 1) - this->at(0, 1) * this->at(1, 0);
        } else {
            T det = 0;
            for (size_t j = 0; j < C; ++j) {
                det += ((j % 2 == 0 ? 1 : -1) * this->at(0, j)) * this->minor(0, j).determinant();
            }
            return det;
        }
    }
private:
    // std::array<std::array<T, C>, R> storage_;
    std::array<T, C * R> storage_;
};


template<arithmetic T, size_t R, size_t C>
__inline OutputStream & operator<<(OutputStream & os, const Matrix<T, R, C> & mat){
    const auto splt = os.splitter();
    os << os.brackets<'['>();
	for (size_t _i = 0; _i < mat.rows(); _i++) {
		os << os.brackets<'['>();
		for (size_t _j = 0; _j < mat.cols(); _j++) {
			os << mat.at(_i,_j);
            if(_j == mat.cols() - 1) break;
            os << splt;
		}
		os << os.brackets<']'>();
        if(_i == mat.rows() - 1) break;
        os << splt;
	}
    os << os.brackets<']'>();
    return os;
}


template<typename T>
using Matrix2x2 = Matrix<T,2,2>;

template<typename T>
using Matrix3x3 = Matrix<T,3,3>;

template<typename T>
using Matrix4x4 = Matrix<T,3,3>;
}