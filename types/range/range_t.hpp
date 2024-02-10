#ifndef __RANGE_HPP__

#define __RANGE_HPP__

#include "../../src/defines/comm_inc.h"

template<typename T>
struct Range_t{
public:
    T start = T(0);
    T end = T(0);

    Range_t() = default;

    template<typename U>
    Range_t(const U & _start, const U & _end): start(_start), end(_end) {;}

    template<typename U>
    Range_t<T> & operator=(const Range_t<U> & other) {
        this->start = other.start;
        this->end = other.end;
        return *this;
    }

    template<typename U>
    static Range_t<T> from_center(const U & center, const U & length){
        return Range_t<T>(center - length / T(2), center + length / T(2));
    }
    bool is_regular() const {
        return start <= end;
    }

    T get_length() const{
        return ABS(end - start);
    }

    Range_t<T> abs() const{
        if(start > end) return Range_t<T>(end, start);
        else return *this;
    }

    template<typename U>
    bool has_value(const U & value){
        Range_t<T> regular = this -> abs();
        return (regular.start <= value && value < regular.end);
    }

    template<typename U>
    bool intersects(const Range_t<U> & _other) const {
        Range_t<T> regular = this -> abs();
        Range_t<T> other_regular = _other.abs();
        return(other_regular.end >= regular.start || regular.end >= other_regular.start);
    }

    template<typename U>
    bool contains(const Range_t<U> & _other) const {
        Range_t<T> regular = this -> abs();
        Range_t<T> other_regular = _other.abs();
        return (regular.start <= other_regular.start && regular.end >= other_regular.end);
    }

    template<typename U>
    Range_t<T> intersection(const Range_t<U> & _other) const {
        Range_t<T> regular = this -> abs();
        Range_t<T> other_regular = _other.abs();
        if(!regular.intersects(other_regular)) return Range_t<T>();
        return Range_t<T>(MAX(regular.start, other_regular.start), MIN(regular.end, other_regular.end));
    }

    T get_center()const{
        return (start + end) / 2;
    }

    template<typename U>
    Range_t<T> scale(const U & amount){
        Range_t<T> regular = *this.abs();
        T len = regular.get_length();
        T center = regular.get_center();
        Range_t<T> ret = Range_t<T>(center - len * amount / 2, center + len * amount / 2);
        if (ret.is_regular()) return ret;
        else return Range_t<T>();
    }

    template<typename U>
    Range_t<T> grow(const U & amount){
        Range_t<T> regular = this -> abs();
        Range_t<T> ret = Range_t<T>(regular.start - amount, regular.end + amount);
        if (ret.is_regular()) return ret;
        else return Range_t<T>();
    }

    template<typename U>
    Range_t<T> merge(const Range_t<U> & other){
        Range_t<T> regular = this -> abs();
        Range_t<T> other_regular = other -> abs();
        return Range_t<T>(MIN(regular.start, other_regular.start), MAX(regular.end, other_regular.end));
    }

    template<typename U>
    Range_t<T> shift(const U & amount){
        Range_t<T> regular = this -> abs();
        Range_t<T> ret = Range_t<T>(regular.start + amount, regular.end + amount);
        return ret;
    }

    template<typename U>
    Range_t merge(const U & value){
        Range_t<T> regular = this -> abs();
        return Range_t<T>(MIN(regular.start, value), MAX(regular.end, value));
    }

    explicit operator bool() const{
        return start!= end;
    }
};

typedef Range_t<int> Rangei;
#endif
