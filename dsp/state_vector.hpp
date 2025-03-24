#pragma once

namespace ymd::dsp{

template<typename T, size_t N>
class StateVector{
public:
    StateVector(){
        reset();
    }

    void reset(){
        for(size_t i=0;i<N;i++) data_[i] = 0;
    }

    StateVector(const std::initializer_list<T> list){
        std::copy(list.begin(), list.end(), data_.begin());
    }

    const T & operator [](const size_t idx) const {
        return data_[idx];
    }
    
    T & operator [](const size_t idx){
        return data_[idx];
    }

    StateVector & operator += (const StateVector & other){
        for(size_t i=0;i<N;i++)
            data_[i] += other.data_[i];
        return *this;
    }

    const auto & get() const{return data_;}
    const T & back() const{return data_.back();}
private:    
    std::array<T, N> data_;
};
}