#pragma once

#include "memory.h"

#include "core/math/real.hpp"
#include "core/stream/ostream.hpp"


namespace ymd{
template<arithmetic T, size_t R, size_t C>
class Matrix{

public:
    static constexpr Matrix from_uninitialized(){
        return Matrix();
    }

    __fast_inline constexpr static Matrix from_zero(){
        Matrix ret = Matrix::from_uninitialized();
        ret.fill(0);
        return ret;
    }

    // 单位矩阵生成函数
    __fast_inline constexpr static Matrix<T, R, C> from_identity(){
        static_assert(R == C, "Identity matrix must be square.");

        Matrix<T, R, C> ret;

        for (size_t i = 0; i < R; ++i) {
            for (size_t j = 0; j < C; ++j) {
                ret.at(i, j) = (i == j) ? T(1) : T(0);
            }
        }

        return ret;
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

    [[nodiscard]] __fast_inline constexpr size_t rows() const { return R;}
    [[nodiscard]] __fast_inline constexpr size_t cols() const { return C;}
    [[nodiscard]] __fast_inline constexpr std::span<T, C> operator[](const size_t row) 
        {return std::span<T, C>(&storage_[row * C], C);}

    [[nodiscard]] __fast_inline constexpr std::span<const T, C> operator[](const size_t row) const 
        {return std::span<const T, C>(&storage_[row * C], C);}

    [[nodiscard]] __fast_inline constexpr T & at(const size_t row, const size_t col) 
        { return storage_[row * C + col];}
    [[nodiscard]] __fast_inline constexpr const T & at(const size_t row, const size_t col) const 
        { return storage_[row * C + col];}

    template<size_t I, size_t J>
    [[nodiscard]] __fast_inline constexpr T & at() 
        { return storage_[I * C + J];}

    template<size_t I, size_t J>
    [[nodiscard]] __fast_inline constexpr const T & at() const 
        { return storage_[I * C + J];}

    [[nodiscard]] __fast_inline constexpr T & operator()(const size_t row, const size_t col) 
        { return storage_[row * C + col];}
    [[nodiscard]] __fast_inline constexpr const T & operator()(const size_t row, const size_t col) const 
        { return storage_[row * C + col];}
    [[nodiscard]] __fast_inline constexpr size_t size() const { return R*C;}
    [[nodiscard]] __fast_inline constexpr T * data() { return storage_.data();}
    [[nodiscard]] __fast_inline constexpr const T * data() const { return storage_.data(); }

    template<size_t R2, size_t C2>
    [[nodiscard]] __fast_inline constexpr Matrix<T, R2, C2> block(
        const size_t row_start, const size_t col_start) const{   
        static_assert(R2 <= R and C2 <= C);

        if(std::is_constant_evaluated()){
            // size_t row_end = row_start + R2;
            // size_t col_end = col_start + C2;

            // static_assert(row_start <= row_end && row_end <= R);
            // static_assert(col_start <= col_end && col_end <= C);
        }

        Matrix<T, R2, C2> ret;

        for (size_t i = 0; i < R2; ++i) {
            for (size_t j = 0; j < C2; ++j) {
                ret.at(i, j) = this->at(row_start + i, col_start + j);
            }
        }

        return ret;
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
    [[nodiscard]] __fast_inline constexpr Matrix operator + (const Matrix<U, R, C> & other) const {
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
    [[nodiscard]] __fast_inline constexpr Matrix operator - (const Matrix<U, R, C> & other) const {
        Matrix ret = Matrix::from_uninitialized();
        auto ptr = data();
        auto ret_ptr = ret.data();
        auto other_ptr = other.data();
        for(size_t i = 0; i < size(); i++){
            ret_ptr[i] = ptr[i] - static_cast<T>(other_ptr[i]);
        }

        return ret;
    }

    [[nodiscard]] __fast_inline constexpr Matrix operator - () const {
        Matrix ret = Matrix::from_uninitialized();
        auto ptr = data();
        auto ret_ptr = ret.data();
        for(size_t i = 0; i < size(); i++){
            ret_ptr[i] = -ptr[i];
        }

        return ret;
    }


    [[nodiscard]] __fast_inline constexpr Matrix operator * (const arithmetic auto & scalar) const{
        Matrix ret = Matrix::from_uninitialized();
        auto ptr = data();
        auto ret_ptr = ret.data();
        for(size_t i = 0; i < size(); i++){
            ret_ptr[i] = ptr[i] * static_cast<T>(scalar);
        }
        return ret;
    }


    [[nodiscard]] __fast_inline constexpr Matrix operator / (const arithmetic auto & scalar) const{
        Matrix ret = Matrix::from_uninitialized();
        auto ptr = data();
        auto ret_ptr = ret.data();
        for(size_t i = 0; i < size(); i++){
            ret_ptr[i] = ptr[i] / static_cast<T>(scalar);
        }
        return ret;
    }

    
    template<typename Fn>
    [[nodiscard]] __fast_inline constexpr Matrix<T, R, C> map(Fn fn) const{ 
        Matrix<T, R, C> ret = Matrix<T, R, C>::from_uninitialized();
        auto ptr = data();
        auto ret_ptr = ret.data();
        for(size_t i = 0; i < size(); i++){
            ret_ptr[i] = std::forward<Fn>(fn)(ptr[i]);
        }
        return ret;
    }

    template<size_t C2>
    [[nodiscard]] __fast_inline constexpr Matrix<T, R, C2> operator * (const Matrix<T, C, C2> & other) const{
        auto ret = Matrix<T, R, C2>::from_uninitialized();
        for (size_t i = 0; i < R; i++) {
            for (size_t j = 0; j < C2; j++) {
                T sum = 0;
                for (size_t k = 0; k < C; k++) {
                    sum += this->at(i, k) * other.at(k, j);
                }
                ret.at(i, j) = sum;
            }
        }
        return ret;
    }

    [[nodiscard]] __fast_inline constexpr T trace() const {
        static_assert(R == C, "Matrix must be square");
        T ret = 0;
        for (size_t i = 0; i < C; i++) {
            ret += this->at(i, i);
        }
        return ret;
    }
    [[nodiscard]] __fast_inline constexpr Matrix<T, C, R> transpose() const{
        Matrix<T, C, R> ret = Matrix<T, C, R>::from_uninitialized();
        for (size_t i = 0; i < R; i++) {
            for (size_t j = 0; j < C; j++) {
                ret.at(j, i) = this->at(i, j);
            }
        }
        return ret;
    }

    [[nodiscard]] __fast_inline constexpr Matrix<T, R, R> lu_inverse() const{
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
    [[nodiscard]] __fast_inline constexpr Matrix<T, R, R> inverse() const{
        static_assert(R == C);

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
        Matrix<T, R, R> ret;
        for (size_t i = 0; i < R; i++) {
            for (size_t j = 0; j < R; j++) {
                ret.at(i, j) = augmented.at(i, j + R);
            }
        }

        return ret;
    }


    [[nodiscard]] __fast_inline constexpr Matrix<T, R-1, C-1> minor(size_t i, size_t j) const {
        static_assert(R > 1, "minor: matrix rows must be greater than 1");
        static_assert(C > 1, "minor: matrix columns must be greater than 1");
        Matrix<T, R-1, C-1> ret = Matrix<T, R-1, C-1>::from_uninitialized();
        size_t dst_row = 0;
        for (size_t src_row = 0; src_row < R; ++src_row) {
            if (src_row == i) continue;
            size_t dst_col = 0;
            for (size_t src_col = 0; src_col < C; ++src_col) {
                if (src_col == j) continue;
                ret.at(dst_row, dst_col) = this->at(src_row, src_col);
                ++dst_col;
            }
            ++dst_row;
        }
        return ret;
    }

    [[nodiscard]] __fast_inline constexpr T minor_determinant(size_t i, size_t j) const {
        static_assert(R > 1, "minor_determinant: matrix rows must be greater than 1");
        static_assert(C > 1, "minor_determinant: matrix columns must be greater than 1");
        static_assert(R == C, "minor_determinant: matrix must be square for determinant calculation");
        
        if constexpr (R == 2) {
            // 2x2矩阵：直接返回对角元素
            return this->at(1 - i, 1 - j);
        } else if constexpr (R == 3) {
            // 3x3矩阵：硬编码2x2行列式计算
            T elements[4];
            size_t idx = 0;
            for (size_t r = 0; r < 3; ++r) {
                if (r == i) continue;
                for (size_t c = 0; c < 3; ++c) {
                    if (c == j) continue;
                    elements[idx++] = this->at(r, c);
                }
            }
            return elements[0] * elements[3] - elements[1] * elements[2];
        } else if constexpr (R == 4) {
            // 也可以为4x4矩阵添加特化
            return compute_4x4_minor_determinant(i, j);
        } else {
            // 大矩阵使用通用算法
            return compute_determinant_without_row_col(i, j);
        }
    }



    [[nodiscard]] __fast_inline constexpr
    T determinant() const{
        static_assert(R == C, "Determinant can only be calculated for square matrices.");

        if constexpr (R == 1) {
            // return this->at(0, 0);
            return storage_[0];
        } else if constexpr (R == 2) {
            // return this->at(0, 0) * this->at(1, 1) - this->at(0, 1) * this->at(1, 0);
            return storage_[0] * storage_[3] - storage_[1] * storage_[2];
        } else {
            T det = 0;
            for (size_t j = 0; j < C; ++j) {
                det += ((j % 2 == 0 ? 1 : -1) * this->at(0, j)) * this->minor_determinant(0,j);
            }
            return det;
        }
    }

    [[nodiscard]] __fast_inline constexpr 
    T abs() const {
        return determinant();
    }

    [[nodiscard]] __fast_inline constexpr 
    const std::array<T, C * R> & to_flatten_array() const { 
        return storage_;
    }
private:
    std::array<T, C * R> storage_;

    __fast_inline constexpr Matrix(){}

    // 辅助函数：计算跳过指定行和列的行列式
    [[nodiscard]] __fast_inline constexpr T compute_determinant_without_row_col(size_t skip_row, size_t skip_col) const {
        constexpr size_t N = R - 1;
        
        if constexpr (N == 1) {
            // 找到唯一剩余的元素
            const size_t row = (skip_row == 0) ? 1 : 0;
            const size_t col = (skip_col == 0) ? 1 : 0;
            return this->at(row, col);
        } else if constexpr (N == 2) {
            // 2x2 子矩阵的行列式
            // 找到剩余的行和列索引
            size_t rows[2], cols[2];
            size_t r_idx = 0;
            for (size_t i = 0; i < R; ++i) {
                if (i != skip_row) rows[r_idx++] = i;
            }
            size_t c_idx = 0;
            for (size_t j = 0; j < C; ++j) {
                if (j != skip_col) cols[c_idx++] = j;
            }
            
            return this->at(rows[0], cols[0]) * this->at(rows[1], cols[1]) 
                - this->at(rows[0], cols[1]) * this->at(rows[1], cols[0]);
        } else {
            // 对于更大的矩阵，使用拉普拉斯展开
            T det = T(0);
            int sign = 1;
            
            // 选择第一行进行展开（跳过skip_row和skip_col后剩余的第一行）
            size_t expand_row = 0;
            while (expand_row == skip_row && expand_row < R) {
                expand_row++;
            }
            
            size_t col_idx = 0;
            for (size_t j = 0; j < C; ++j) {
                if (j == skip_col) continue;
                
                T cofactor = sign * this->at(expand_row, j) 
                        * compute_determinant_without_row_col_col(expand_row, j, skip_row, skip_col);
                det += cofactor;
                sign = -sign;
                col_idx++;
            }
            
            return det;
        }
    }

    // 辅助函数：计算跳过两行两列的行列式
    [[nodiscard]] __fast_inline constexpr T compute_determinant_without_row_col_col(
        size_t skip_row1, size_t skip_col1, size_t skip_row2, size_t skip_col2) const {
        
        constexpr size_t N = R - 2;
        
        if constexpr (N == 1) {
            // 找到唯一剩余的元素
            for (size_t i = 0; i < R; ++i) {
                if (i != skip_row1 && i != skip_row2) {
                    for (size_t j = 0; j < C; ++j) {
                        if (j != skip_col1 && j != skip_col2) {
                            return this->at(i, j);
                        }
                    }
                }
            }
            return T(0); // 不应该到达这里
        } else {
            // 递归计算更小的子矩阵
            Matrix<T, N, N> submatrix;
            size_t dst_row = 0;
            
            for (size_t src_row = 0; src_row < R; ++src_row) {
                if (src_row == skip_row1 || src_row == skip_row2) continue;
                
                size_t dst_col = 0;
                for (size_t src_col = 0; src_col < C; ++src_col) {
                    if (src_col == skip_col1 || src_col == skip_col2) continue;
                    
                    submatrix.at(dst_row, dst_col) = this->at(src_row, src_col);
                    dst_col++;
                }
                dst_row++;
            }
            
            return submatrix.determinant();
        }
    }


    [[nodiscard]] __fast_inline constexpr T compute_4x4_minor_determinant(size_t i, size_t j) const {
        // 手动展开4x4矩阵的3x3子矩阵行列式计算
        // 选择展开行（跳过第i行）
        const size_t rows[3];
        size_t r_idx = 0;
        for (size_t r = 0; r < 4; ++r) {
            if (r != i) rows[r_idx++] = r;
        }
        
        // 拉普拉斯展开计算3x3行列式
        return this->at(rows[0], (j+1)%4) * (this->at(rows[1], (j+2)%4) * this->at(rows[2], (j+3)%4) - 
                                            this->at(rows[1], (j+3)%4) * this->at(rows[2], (j+2)%4))
            - this->at(rows[0], (j+2)%4) * (this->at(rows[1], (j+1)%4) * this->at(rows[2], (j+3)%4) - 
                                            this->at(rows[1], (j+3)%4) * this->at(rows[2], (j+1)%4))
            + this->at(rows[0], (j+3)%4) * (this->at(rows[1], (j+1)%4) * this->at(rows[2], (j+2)%4) - 
                                            this->at(rows[1], (j+2)%4) * this->at(rows[2], (j+1)%4));
    }
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
struct ToMatrixDispatcher{
    //static constexpr auto cast();
};


template<typename T>
auto to_matrix(T && obj){
    return ToMatrixDispatcher<std::decay_t<T>>::cast(obj);
}


template<typename T>
using Matrix2x2 = Matrix<T,2,2>;

template<typename T>
using Matrix3x3 = Matrix<T,3,3>;

template<typename T>
using Matrix4x4 = Matrix<T,3,3>;

template<typename T, size_t N>
using RowVector = Matrix<T,N,1>;

template<typename T, size_t N>
using ColVector = Matrix<T,1,N>;

}