#ifndef __RANGE_HPP__

#define __RANGE_HPP__

#include "src/platform.h"

template<typename T>
struct Range_t{
public:
    T start = T(0);
    T end = T(0);

    Range_t() = default;

    __fast_inline constexpr Range_t(const auto & _start, const auto & _end): start(static_cast<T>(_start)), end(static_cast<T>(_end)) {;}

    __fast_inline constexpr auto & operator=(const Range_t<auto> & other) {
        this->start = other.start;
        this->end = other.end;
        return *this;
    }

    constexpr static Range_t<T> from_center(const auto & center, const auto & length){
        return Range_t<T>(center - length / T(2), center + length / T(2));
    }

    constexpr bool is_regular() const {
        return start <= end;
    }

    constexpr T get_length() const{
        return ABS(end - start);
    }

    constexpr Range_t<T> abs() const{
        if(start > end) return Range_t<T>(end, start);
        else return *this;
    }

    constexpr Range_t<T> operator + (const auto & value) const{
        Range_t<T> regular = this -> abs();
        return (regular.start + value, regular.end + value);
    }

    constexpr Range_t<T> operator - (const auto & value) const{
        Range_t<T> regular = this -> abs();
        return Range_t<T>(regular.start - value, regular.end - value);
    }

    constexpr Range_t<T> operator * (const auto & value) const{
        Range_t<T> regular = this -> abs();
        return Range_t<T>(regular.start * value, regular.end * value);
    }


    constexpr Range_t<T> operator / (const auto & value) const{
        Range_t<T> regular = this -> abs();
        return Range_t<T>(regular.start / value, regular.end / value);
    }


    constexpr bool has_value(const auto & value){
        Range_t<T> regular = this -> abs();
        return (regular.start <= value && value < regular.end);
    }

    constexpr bool intersects(const Range_t<auto> & _other) const {
        Range_t<T> regular = this -> abs();
        Range_t<T> other_regular = _other.abs();
        return(other_regular.end >= regular.start || regular.end >= other_regular.start);
    }

    constexpr bool contains(const Range_t<auto> & _other) const {
        Range_t<T> regular = this -> abs();
        Range_t<T> other_regular = _other.abs();
        return (regular.start <= other_regular.start && regular.end >= other_regular.end);
    }

    constexpr bool inside(const Range_t<auto> & _other) const {
        return _other.contains(*this);
    }

    constexpr Range_t<T> intersection(const Range_t<auto> & _other) const {
        Range_t<T> regular = this -> abs();
        Range_t<T> other_regular = _other.abs();
        if(!regular.intersects(other_regular)) return Range_t<T>();
        return Range_t<T>(MAX(regular.start, other_regular.start), MIN(regular.end, other_regular.end));
    }

    constexpr T get_center()const{
        return (start + end) / 2;
    }

    constexpr Range_t<T> scale(const auto & amount){
        Range_t<T> regular = *this.abs();
        T len = regular.get_length();
        T center = regular.get_center();
        Range_t<T> ret = Range_t<T>(center - len * amount / 2, center + len * amount / 2);
        if (ret.is_regular()) return ret;
        else return Range_t<T>();
    }

    constexpr Range_t<T> grow(const auto & amount){
        Range_t<T> regular = this -> abs();
        Range_t<T> ret = Range_t<T>(regular.start - amount, regular.end + amount);
        if (ret.is_regular()) return ret;
        else return Range_t<T>();
    }

    constexpr Range_t<T> merge(const Range_t<auto> & other){
        Range_t<T> regular = this -> abs();
        Range_t<T> other_regular = other -> abs();
        return Range_t<T>(MIN(regular.start, other_regular.start), MAX(regular.end, other_regular.end));
    }

    // template<typename U>
    constexpr Range_t<T> shift(const auto & amount){
        Range_t<T> regular = this -> abs();
        Range_t<T> ret = Range_t<T>(regular.start + amount, regular.end + amount);
        return ret;
    }

    // template<typename U>
    constexpr Range_t merge(const auto & value){
        Range_t<T> regular = this -> abs();
        return Range_t<T>(MIN(regular.start, value), MAX(regular.end, value));
    }

    // template<typename U>
    constexpr T invlerp(const auto & value){
        return (value - start) / (end - start);
    }

    // template<typename U>
    constexpr T lerp(const auto & value){
        return start + (value) * (end - start);
    }

    constexpr T clamp(const auto & value){
        Range_t<T> regular = this -> abs();
        return MIN(MAX(value, regular.start), regular.end);
    }

    constexpr explicit operator bool() const{
        return start!= end;
    }
};

typedef Range_t<int> Rangei;
#endif
