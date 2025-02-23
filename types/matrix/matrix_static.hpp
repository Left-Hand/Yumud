#pragma once

#include "memory.h"

#include "sys/math/real.hpp"
#include "sys/stream/ostream.hpp"


namespace ymd{
template<arithmetic T, size_t R, size_t C>
class Matrix_t
{
public:
    __fast_inline constexpr Matrix_t(){}

    __fast_inline constexpr Matrix_t(const arithmetic auto * _data){
        for(size_t i = 0; i < size(); i++){
            begin()[i] = static_cast<T>(_data[i]);
        }
    }
    
    __fast_inline constexpr Matrix_t& operator = (const Matrix_t & other){
        for(size_t i = 0; i < size(); i++){
            begin()[i] = static_cast<T>(other.data_[i]);
        }
    }

    __fast_inline constexpr static Matrix_t Zero(){
        Matrix_t result;
        result.fill(0);
        return result;
    }

    __fast_inline constexpr void fill(const T val){
        auto ptr = begin();
        for(size_t i = 0; i < size(); i++){
            ptr[i] = static_cast<T>(0);
        }
    }


    __fast_inline constexpr size_t rows(void) const { return R;}
    __fast_inline constexpr size_t cols(void) const { return C;}


    __fast_inline constexpr auto & row(const size_t row_){return data_[row_];}  
    __fast_inline constexpr const auto & row(const size_t row_) const {return data_[row_];}

    __fast_inline constexpr auto & operator[](const size_t row_){return data_[row_];}  
    __fast_inline constexpr const auto & operator[](const size_t row_) const {return data_[row_];}

    __fast_inline constexpr T & at(const size_t _row, const size_t _col) { return data_[_row][_col];}
    __fast_inline constexpr const T & at(const size_t _row, const size_t _col) const { return data_[_row][_col];}
    __fast_inline constexpr size_t size() const { return R*C;}

    __fast_inline constexpr T * begin() { return reinterpret_cast<T *>(&data_[0][0]);}
    __fast_inline constexpr const T * begin() const { return reinterpret_cast<const T *>(&data_[0][0]);}
    __fast_inline constexpr T * end() { return begin() + size();}
    __fast_inline constexpr const T * end() const { return begin() + size();}

    __fast_inline constexpr auto & data() { return data_;}
    __fast_inline constexpr const auto & data() const { return data_; }

    __fast_inline constexpr void setZero() {
        auto ptr = begin();
        for(size_t i = 0; i < size(); i++){
            ptr[i] = static_cast<T>(0);
        }
    }

    template<size_t R2, size_t C2>
    __fast_inline constexpr Matrix_t<T, R2, C2> block(const size_t row_start, const size_t col_start) const{   
        static_assert(R2 <= R and C2 <= C);

        if(std::is_constant_evaluated()){
            // size_t row_end = row_start + R2;
            // size_t col_end = col_start + C2;

            // static_assert(row_start <= row_end && row_end <= R);
            // static_assert(col_start <= col_end && col_end <= C);
        }

        Matrix_t<T, R2, C2> result;

        for (size_t i = 0; i < R2; ++i) {
            for (size_t j = 0; j < C2; ++j) {
                result.at(i, j) = this->at(row_start + i, col_start + j);
            }
        }

        return result;
    }

    template<arithmetic U>
    __fast_inline constexpr Matrix_t & operator += (const Matrix_t<U, R, C> & other){
        auto ptr = begin();
        auto other_ptr = other.begin();
        for(size_t i = 0; i < size(); i++){
            ptr[i] += static_cast<T>(other_ptr[i]);
        }

        return *this;
    }

    template<arithmetic U>
    __fast_inline constexpr Matrix_t operator + (const Matrix_t<U, R, C> & other){
        Matrix_t ret;
        auto ptr = begin();
        auto ret_ptr = ret.begin();
        auto other_ptr = other.begin();
        for(size_t i = 0; i < size(); i++){
            ret_ptr[i] = ptr[i] + static_cast<T>(other_ptr[i]);
        }

        return ret;
    }

    template<arithmetic U>
    __fast_inline constexpr Matrix_t & operator -= (const Matrix_t<U, R, C> & other){
        auto ptr = begin();
        auto other_ptr = other.begin();
        for(size_t i = 0; i < size(); i++){
            ptr[i] -= static_cast<T>(other_ptr[i]);
        }

        return *this;
    }

    template<arithmetic U>
    __fast_inline constexpr Matrix_t operator - (const Matrix_t<U, R, C> & other){
        Matrix_t ret;
        auto ptr = begin();
        auto ret_ptr = ret.begin();
        auto other_ptr = other.begin();
        for(size_t i = 0; i < size(); i++){
            ret_ptr[i] = ptr[i] - static_cast<T>(other_ptr[i]);
        }

        return ret;
    }

    __fast_inline constexpr Matrix_t & operator *= (const arithmetic auto & scalar){
        auto ptr = begin();
        for(size_t i = 0; i < size(); i++){
            ptr[i] *= static_cast<T>(scalar);
        }

        return *this;
    }

    __fast_inline constexpr Matrix_t & operator /= (const arithmetic auto & scalar){
        auto ptr = begin();
        for(size_t i = 0; i < size(); i++){
            ptr[i] /= static_cast<T>(scalar);
        }

        return *this;
    }



    template<size_t C2>
    __fast_inline constexpr Matrix_t<T, R, C2> operator * (const Matrix_t<T, C, C2> & other) const
    {
        static_assert(this->cols() == other.rows());

        Matrix_t<T, R, C2> result;
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


    __fast_inline constexpr Matrix_t<T, C, R> transpose() const
    {
        Matrix_t<T, C, R> result;
        for (size_t i = 0; i < R; i++) {
            for (size_t j = 0; j < C; j++) {
                result.at(j, i) = this->at(i, j);
            }
        }
        return result;
    }


    template<typename U = T, typename std::enable_if_t<std::is_arithmetic_v<U>, int> = 0>
    __fast_inline constexpr Matrix_t<T, R, R> inverse() const{
        static_assert(R == C);

        Matrix_t<T, R, 2 * R> augmented;
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
        Matrix_t<T, R, R> result;
        for (size_t i = 0; i < R; i++) {
            for (size_t j = 0; j < R; j++) {
                result.at(i, j) = augmented.at(i, j + R);
            }
        }

        return result;
    }

    // 单位矩阵生成函数
    __fast_inline constexpr static Matrix_t<T, R, C> Identity(){
        static_assert(R == C, "Identity matrix must be square.");

        Matrix_t<T, R, C> result;

        for (size_t i = 0; i < R; ++i) {
            for (size_t j = 0; j < C; ++j) {
                result.at(i, j) = (i == j) ? T(1) : T(0);
            }
        }

        return result;
    }

private:
    std::array<std::array<T, C>, R> data_;
};


template<arithmetic T, size_t R, size_t C>
__inline OutputStream & operator<<(OutputStream & os, const Matrix_t<T, R, C> & mat){
    const auto splt = os.splitter();
    os << "[";
	for (size_t _i = 0; _i < mat.rows(); _i++) {
		os << "[";
		for (size_t _j = 0; _j < mat.cols(); _j++) {
			os << mat.at(_i,_j);
            if(_j == mat.cols() - 1) break;
            os << splt;
		}
		os << "]";
        if(_i == mat.rows() - 1) break;
        os << splt;
	}
    os << "]";
    return os;
}

}