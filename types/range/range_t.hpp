#ifndef __RANGE_HPP__

#define __RANGE_HPP__

#include "real.hpp"

#include "../sys/core/platform.h"
#include "../types/string/String.hpp"
#include "type_traits"

#include <algorithm>

template<arithmetic T>
struct Range_t{
public:
    struct {
        T from;
        T to;
    }__packed;

    constexpr Range_t(): from(T(0)), to(T(0)) {;}

    constexpr Range_t(const arithmetic auto & _from, const arithmetic auto & _to): from(static_cast<T>(_from)), to(static_cast<T>(_to)) {;}

    constexpr Range_t(const Range_t<auto> & other): from(static_cast<T>(other.from)), to(static_cast<T>(other.to)) {;}

    constexpr Range_t(const std::pair<arithmetic auto, arithmetic auto> & other): from(static_cast<T>(other.first)), to(static_cast<T>(other.second)) {;}

    constexpr Range_t(const std::tuple<arithmetic auto, arithmetic auto> & other): from(static_cast<T>(std::get<0>(other))), to(static_cast<T>(std::get<1>(other))) {;}

    T & operator [](const size_t index) { return *(&this->from + index);}

    const T & operator [](const size_t index) const {return *(&this->from + index);}

    constexpr Range_t<T> & operator=(const Range_t<auto> & other) {
        this->from = static_cast<T>(other.from);
        this->to = static_cast<T>(other.to);
        return *this;
    }

    constexpr static Range_t<T> from_center(const arithmetic auto & center, const arithmetic auto & length){
        return Range_t<T>(center - length / 2, center + length / 2);
    }

    constexpr bool is_regular() const {
        return from <= to;
    }

    constexpr T length() const{
        return ABS(to - from);
    }
    constexpr T length_signed() const{
        return (to - from);
    }

    constexpr Range_t<T> abs() const{
        if(from > to) return Range_t<T>(to, from);
        else return *this;
    }

    constexpr Range_t<T> operator + (const arithmetic auto & rvalue) const{
        Range_t<T> regular = this -> abs();
        return Range_t<T>(regular.from + rvalue, regular.to + rvalue);
    }

    constexpr Range_t<T> operator - (const arithmetic auto & rvalue) const{
        Range_t<T> regular = this -> abs();
        return Range_t<T>(regular.from - rvalue, regular.to - rvalue);
    }

    constexpr Range_t<T> operator * (const arithmetic auto & rvalue) const{
        Range_t<T> regular = this -> abs();
        return Range_t<T>(regular.from * rvalue, regular.to * rvalue);
    }

    __fast_inline constexpr Range_t<T> operator / (const arithmetic auto & rvalue) const{
        Range_t<T> regular = this -> abs();
        return Range_t<T>(regular.from / rvalue, regular.to / rvalue);
    }

    constexpr Range_t<T> & operator += (const arithmetic auto & rvalue) {
        *this = *this + rvalue;
        return *this;
    }

    [[deprecated]] constexpr Range_t<T> & operator -= (const arithmetic auto & rvalue) {
        *this = *this - rvalue;
        return *this;
    }

    constexpr Range_t<T> & operator *= (const arithmetic auto & rvalue) {
        *this = *this * rvalue;
        return *this;
    }

    constexpr Range_t<T> & operator /= (const arithmetic auto & rvalue){
        *this = *this / rvalue;
        return *this;
    }

    constexpr bool operator == (const Range_t<auto> & other) const {
        Range_t<T> regular = this -> abs();
        Range_t<T> other_regular = other.abs();
        return (regular.from == other_regular.from && regular.to == other_regular.to);
    }

    constexpr bool operator!= (const Range_t<auto> & other) const {
        return !(*this == other);
    }

    constexpr bool intersects(const Range_t<auto> & other) const {
        Range_t<T> regular = this -> abs();
        Range_t<T> other_regular = other.abs();
        return(regular.has(other_regular.from) || other_regular.has(regular.from));
    }

    constexpr bool contains(const Range_t<auto> & other) const {
        Range_t<T> regular = this -> abs();
        Range_t<T> other_regular = other.abs();
        return (regular.from <= other_regular.from && regular.to >= other_regular.to);
    }

    constexpr bool inside(const Range_t<auto> & other) const {
        return other.contains(*this);
    }

    constexpr bool has(const arithmetic auto & value) const{
        Range_t<T> regular = this -> abs();
        return (regular.from <= static_cast<T>(value) && static_cast<T>(value) < regular.to);
    }

    constexpr bool has(const Range_t<auto> & other) const{return contains(other);}

    constexpr T padding(const Range_t<T> other) const {
        Range_t<T> regular = this -> abs();
        Range_t<T> other_regular = other.abs();
        if(regular.intersects(other_regular)) return 0;
        return MIN((other_regular.to - regular.from), (other_regular.from - regular.to));
    }

    constexpr Range_t<T> intersection(const Range_t<auto> & other) const {
        Range_t<T> regular = this -> abs();
        Range_t<T> other_regular = other.abs();
        if(not regular.intersects(other_regular)) return Range_t<T>();
        return Range_t<T>(MAX(regular.from, other_regular.from), MIN(regular.to, other_regular.to));
    }

    constexpr T get_center()const{
        return (from + to) / 2;
    }

    static constexpr Range_t<T> grid(const arithmetic auto & value, const auto & grid_size){
        T ret_from;
        if constexpr(std::is_integral_v<T>){
            ret_from = (value / grid_size) * grid_size;
        }else{
            ret_from = floor(value / grid_size) * grid_size;
        }
        return Range_t<T>(ret_from, ret_from + grid_size);
    }

    static constexpr Range_t<T> grid_next(const arithmetic auto & value, const arithmetic auto & grid_size, const bool right = true){
        return grid(right ? (value + grid_size) : (value-grid_size), grid_size);
    }

    static constexpr Range_t<T> grid_next_right(const arithmetic auto & value, const arithmetic auto & grid_size){
        return grid_next(value, grid_size, true);
    }

    static constexpr Range_t<T> grid_next_left(const arithmetic auto & value, const arithmetic auto & grid_size){
        return grid_next(value, grid_size, false);
    }

    constexpr Range_t<T> gridfy(const arithmetic auto & grid_size) const {
        return Range_t<T>(grid(from, grid_size), grid(to, grid_size));
    }

    static constexpr Range_t<T> part_in_grid(const arithmetic auto & value, const arithmetic auto & grid_size, const bool right_part = true){
        if constexpr(std::is_integral<T>::value){
            if(value % grid_size == 0) return {value, value};
            auto gridfied = grid(value, grid_size);
            if(right_part){
                return {value, gridfied.to};
            }else{
                return {gridfied.from, value};
            }
        }
    }

    constexpr Range_t<T> part_right_in_grid(const arithmetic auto & value, const arithmetic auto & grid_size) const{
        return part_in_grid(value, grid_size, true);
    }

    constexpr Range_t<T> part_left_in_grid(const arithmetic auto & value, const arithmetic auto & grid_size) const{
        return part_in_grid(value, grid_size, false);
    }


    constexpr Range_t<T> room_left(const Range_t<auto> & _content) const{
        Range_t<T> content = _content.abs();
        Range_t<T> regular = this -> abs();
        return {regular.from, MAX(content.from, regular.from)};
    }

    constexpr Range_t<T> room_right(const Range_t<auto> & _content) const{
        Range_t<T> content = _content.abs();
        Range_t<T> regular = this -> abs();
        return {regular.from, MIN(content.from, regular.from)};
    }

    constexpr int rooms(const Range_t<auto> & _content) const{
        return bool(room_left(_content)) + bool(room_right(_content));
    }

    constexpr Range_t<T> grid_forward(const Range_t<auto> & _before, const auto & grid_size) const{
        Range_t<T> before = _before.abs();
        Range_t<T> regular = this -> abs();

        if(before.from == before.to && before.from == 0){//initial
            auto grid_field = grid(regular.from, grid_size);
            if(grid_field.has(regular)) return regular;
            else return {regular.from, grid_field.to};
        }


        if(part_right_in_grid(before.to, grid_size)){
            return {before.to, before.to};
        }

        auto ret = grid_next_right(before.from, grid_size);
        if(ret.has(regular.to)){
            return ret.part_left_in_grid(regular.to, grid_size);
        }
        return ret;
    }

    constexpr Range_t<T> scale(const auto & amount){
        Range_t<T> regular = *this.abs();
        T len = regular.length();
        T center = regular.get_center();
        Range_t<T> ret = Range_t<T>(center - len * amount / 2, center + len * amount / 2);
        if (ret.is_regular()) return ret;
        else return Range_t<T>();
    }


    constexpr Range_t<T> grow(const arithmetic auto & value) const{
        Range_t<T> regular = this -> abs();
        Range_t<T> ret = Range_t<T>(regular.from - value, regular.to + value);
        if (ret.is_regular()) return ret;
        else return Range_t<T>();
    }

    constexpr Range_t<T> merge(const Range_t<arithmetic auto> & other) const {
        Range_t<T> regular = this -> abs();
        Range_t<T> other_regular = other.abs();
        return Range_t<T>(MIN(regular.from, other_regular.from), MAX(regular.to, other_regular.to));
    }

    constexpr Range_t<T> shift(const arithmetic auto & value) const{
        Range_t<T> regular = this -> abs();
        Range_t<T> ret = Range_t<T>(regular.from + value, regular.to + value);
        return ret;
    }

    constexpr Range_t<T> merge(const arithmetic auto & value) const{
        Range_t<T> regular = this -> abs();
        return Range_t<T>(MIN(regular.from, value), MAX(regular.to, value));
    }

    constexpr T invlerp(const arithmetic auto & value) const{
        return T((value - from) / (to - from));
    }

    constexpr T lerp(const arithmetic auto & value) const {
        return from + (value) * (to - from);
    }

    constexpr bool operator<(const arithmetic auto & value) const {
        Range_t<T> regular = this -> abs();
        return value < regular.from;
    }

    constexpr bool operator<=(const arithmetic auto & value) const {
        Range_t<T> regular = this -> abs();
        return value <= regular.to;
    }

    constexpr bool operator>(const arithmetic auto & value) const {
        Range_t<T> regular = this -> abs();
        return value > regular.to;
    }
    constexpr bool operator>=(const arithmetic auto & value) const {
        Range_t<T> regular = this -> abs();
        return value >= regular.from;
    }


    constexpr T clamp(const arithmetic auto & value) const{
        Range_t<T> regular = this -> abs();
        return CLAMP(value, regular.from, regular.to);
    }

    constexpr explicit operator bool() const{
        return from != to;
    }

    constexpr  explicit operator String() const{
        return toString();
    }

    String toString(unsigned char decimalPlaces = 2) const{
        if constexpr(std::is_integral<T>::value){
            return ('[' + String(from) + ',' + String(to) + ')');
        }else{
            return ('[' + ::toString(from, decimalPlaces) + ',' + ::toString(to, decimalPlaces) + ')');
        }
    }
}__packed;

using Rangei = Range_t<int>;
using Range = Range_t<real_t>;

__inline OutputStream & operator<<(OutputStream & os, const Range_t<auto> & value){
    return os << '[' << value.from << ',' << value.to << ')';
}

#endif
