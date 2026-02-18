#pragma once

#include "core/math/fixed/fixed.hpp"

namespace ymd::dsp{

    
//状态向量
template<typename T, size_t N>
class [[nodiscard]] StateVector{
public:
    //以零值构造
    constexpr explicit StateVector(){
        reset();
    }

    constexpr StateVector(const std::array<T, N> list){
        std::copy(list.begin(), list.end(), x_.begin());
    }

    constexpr StateVector(const std::initializer_list<T> list){
        std::copy(list.begin(), list.end(), x_.begin());
    }

    //全部设置为零
    constexpr void reset(){
        x_.fill(0);
    }

    constexpr const T & operator [](const size_t idx) const {
        return x_[idx];
    }
    
    constexpr T & operator [](const size_t idx){
        return x_[idx];
    }

    constexpr StateVector & operator += (const StateVector & other){
        for(size_t i=0;i<N;i++)
            x_[i] += other.x_[i];
        return *this;
    }

    template<size_t I>
    requires (I < N)
    constexpr const T & get() const{return x_[I];}

    constexpr T * begin() {return x_.begin();}
    constexpr const T * begin() const {return x_.begin();}

    constexpr T * end() {return x_.end();}
    constexpr const T * end() const {return x_.end();}

    constexpr const T & back() const{return x_.back();}
private:    
    std::array<T, N> x_;
};


//二阶状态向量

template<typename T>
struct SecondOrderState;

template<>
struct [[nodiscard]] SecondOrderState<iq16>{
    using Self = SecondOrderState;

    //原始信号
    math::fixed<32, int64_t> x1 = 0;

    //一阶导信号
    math::fixed<16, int32_t> x2 = 0;

    //从零构造
    static constexpr Self zero(){
        return Self{0, 0};
    }
};


}