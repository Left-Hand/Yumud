#pragma once

namespace ymd::dsp{

template<typename T, size_t N>
class StateVector{
public:
    constexpr StateVector(){
        reset();
    }

    constexpr void reset(){
        for(size_t i=0;i<N;i++) data_[i] = 0;
    }

    constexpr StateVector(const std::array<T, N> list){
        std::copy(list.begin(), list.end(), data_.begin());
    }

    constexpr StateVector(const std::initializer_list<T> list){
        std::copy(list.begin(), list.end(), data_.begin());
    }

    constexpr const T & operator [](const size_t idx) const {
        return data_[idx];
    }
    
    constexpr T & operator [](const size_t idx){
        return data_[idx];
    }

    constexpr StateVector & operator += (const StateVector & other){
        for(size_t i=0;i<N;i++)
            data_[i] += other.data_[i];
        return *this;
    }

    template<size_t I>
    requires (I < N)
    constexpr const T & get() const{return data_[I];}

    constexpr T * begin() {return data_.begin();}
    constexpr const T * begin() const {return data_.begin();}

    constexpr T * end() {return data_.end();}
    constexpr const T * end() const {return data_.end();}

    constexpr const T & back() const{return data_.back();}
private:    
    std::array<T, N> data_;
};
}