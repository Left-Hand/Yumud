#ifndef __RANGE2_HPP__

#define __RANGE2_HPP__

#include "../../src/defines/comm_inc.h"

template<typename T>
struct Range2_t{
public:
    T start = T(0);
    T end = T(0);

    template<typename U>
    Range2_t(const U & _start, const U & _end): start(_start), end(_end) {;}

    template<typename U>
    Range2_t<T> & operator=(const Range2_t<U> & other) {
        this->start = other.start;
        this->end = other.end;
        return *this;
    }

    bool is_regular() const {
        return start <= end;
    }

    T get_length() const{
        return abs(end - start);
    }

    Range2_t<T> abs() const{
        if(start > end) return Range2_t<T>(end, start);
        else return *this;
    }

    template<typename U>
    bool has_value(const U & value){
        Range2_t<T> regular = *this.abs();
        return (start <= value && value <= end);
    }

    template<typename U>
    bool intersects(const Range2_t<U> & _other) const {
        Range2_t<T> regular = *this.abs();
        Range2_t<T> other_regular = _other.abs();
        if(regular.start > other_regular.end || regular.end <= other_regular.start) return true;
        else return false;
    }

    template<typename U>
    bool contains(const Range2_t<U> & _other) const {
        Range2_t<T> regular = *this.abs();
        Range2_t<T> other_regular = _other.abs();
        if(regular.start <= other_regular.start && regular.end >= other_regular.end) return true;
        else return false;
    }

    template<typename U>
    Range2_t<T> intersection(const Range2_t<U> & _other) const {
        Range2_t<T> regular = *this.abs();
        Range2_t<T> other_regular = _other.abs();
        if(!regular.intersects(other_regular)) return Range2_t<T>();
        return Range2_t<T>(max(regular.start, other_regular.start), min(regular.end, other_regular.end));
    }

    T get_center()const{
        return (start + end) / 2;
    }

    template<typename U>
    Range2_t<T> scale(const U & amount){
        Range2_t<T> regular = *this.abs();
        T len = regular.get_length();
        T center = regular.get_center();
        Range2_t<T> ret = Range2_t<T>(center - len * amount / 2, center + len * amount / 2);
        if (ret.is_regular()) return ret;
        else return Range2_t<T>();
    }

    template<typename U>
    Range2_t<T> grow(const U & amount){
        Range2_t<T> regular = *this.abs();
        Range2_t<T> ret = Range2_t<T>(regular.start - amount, regular.end + amount);
        if (ret.is_regular()) return ret;
        else return Range2_t<T>();
    }

    explicit operator bool() const{
        return start!= end;
    }
};

#endif
