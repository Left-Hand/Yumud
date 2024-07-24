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
    union{
        T from;
        T a;
    }__packed;

    union{
        T to;
        T b;
    }__packed;

    __fast_inline_constexpr Range_t(): from(T(0)), to(T(0)) {;}
    __fast_inline_constexpr Range_t(const auto & _from, const auto & _to): from(static_cast<T>(_from)), to(static_cast<T>(_to)) {;}

    __fast_inline_constexpr Range_t(const Range_t<auto> & other): from(static_cast<T>(other.from)), to(static_cast<T>(other.to)) {;}

    template<typename U, typename V>
    requires std::is_arithmetic_v<U> && std::is_arithmetic_v<V>
    __fast_inline_constexpr Range_t(std::pair<U, V> && other): from(static_cast<T>(other.first)), to(static_cast<T>(other.second)) {;}

    template<typename U, typename V>
    requires std::is_arithmetic_v<U> && std::is_arithmetic_v<V>
    __fast_inline_constexpr Range_t(std::tuple<U, V> && other): from(static_cast<T>(std::get<0>(other))), to(static_cast<T>(std::get<1>(other))) {;}
    __fast_inline_constexpr Range_t<T> & operator=(const Range_t<auto> & other) {
        this->from = static_cast<T>(other.from);
        this->to = static_cast<T>(other.to);
        return *this;
    }

    template<typename U, typename V>
    requires std::is_arithmetic_v<U> and std::is_arithmetic_v<V>
    constexpr static Range_t<T> from_center(const U & center, const V & length){
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

    __fast_inline_constexpr Range_t<T> abs() const{
        if(from > to) return Range_t<T>(to, from);
        else return *this;
    }

    __fast_inline_constexpr Range_t<T> operator + (const auto & value) const{
        Range_t<T> regular = this -> abs();
        return Range_t<T>(regular.from + value, regular.to + value);
    }

    __fast_inline_constexpr Range_t<T> operator - (const auto & value) const{
        Range_t<T> regular = this -> abs();
        return Range_t<T>(regular.from - value, regular.to - value);
    }

    __fast_inline_constexpr Range_t<T> operator * (const auto & value) const{
        Range_t<T> regular = this -> abs();
        return Range_t<T>(regular.from * value, regular.to * value);
    }

    __fast_inline constexpr Range_t<T> operator / (const auto & value) const{
        Range_t<T> regular = this -> abs();
        return Range_t<T>(regular.from / value, regular.to / value);
    }

    __fast_inline_constexpr Range_t<T> & operator += (const auto & value) {
        *this = *this + value;
        return *this;
    }

    [[deprecated]] __fast_inline_constexpr Range_t<T> & operator -= (const auto & value) {
        *this = *this - value;
        return *this;
    }

    __fast_inline_constexpr Range_t<T> & operator *= (const auto & value) {
        *this = *this * value;
        return *this;
    }

    __fast_inline_constexpr Range_t<T> & operator /= (const auto & value){
        *this = *this / value;
        return *this;
    }

    __fast_inline_constexpr bool operator == (const Range_t<auto> & _other) const {
        Range_t<T> regular = this -> abs();
        Range_t<T> other_regular = _other.abs();
        return (regular.from == other_regular.from && regular.to == other_regular.to);
    }

    __fast_inline_constexpr bool operator!= (const Range_t<auto> & _other) const {
        return !(*this == _other);
    }

    __fast_inline_constexpr bool intersects(const Range_t<auto> & _other) const {
        Range_t<T> regular = this -> abs();
        Range_t<T> other_regular = _other.abs();
        return(regular.has(other_regular.from) || other_regular.has(regular.from));
    }

    __fast_inline_constexpr bool contains(const Range_t<auto> & _other) const {
        Range_t<T> regular = this -> abs();
        Range_t<T> other_regular = _other.abs();
        return (regular.from <= other_regular.from && regular.to >= other_regular.to);
    }

    __fast_inline_constexpr bool inside(const Range_t<auto> & _other) const {
        return _other.contains(*this);
    }

    template<arithmetic U>
    constexpr bool has(const U & value) const{
        Range_t<T> regular = this -> abs();
        return (regular.from <= value && value < regular.to);
    }

    constexpr bool has(const Range_t<auto> & _other) const{return contains(_other);}

    constexpr T padding(const Range_t<T> other) const {
        Range_t<T> regular = this -> abs();
        Range_t<T> other_regular = other.abs();
        if(regular.intersects(other_regular)) return 0;
        return MIN((other_regular.to - regular.from), (other_regular.from - regular.to));
    }

    constexpr Range_t<T> intersection(const Range_t<auto> & _other) const {
        Range_t<T> regular = this -> abs();
        Range_t<T> other_regular = _other.abs();
        if(!regular.intersects(other_regular)) return Range_t<T>();
        return Range_t<T>(MAX(regular.from, other_regular.from), MIN(regular.to, other_regular.to));
    }

    constexpr T get_center()const{
        return (from + to) / 2;
    }

    static constexpr Range_t<T> grid(const auto & value, const auto & grid_size){
        T ret_from;
        if constexpr(std::is_integral<T>::value){
            ret_from = (value / grid_size) * grid_size;
        }else{
            ret_from = floor(value / grid_size) * grid_size;
        }
        return Range_t<T>(ret_from, ret_from + grid_size);
    }

    static constexpr Range_t<T> grid_next(const auto & value, const auto & grid_size, const bool & right = true){
        return grid(right ? (value + grid_size) : (value-grid_size), grid_size);
    }

    static constexpr Range_t<T> grid_next_right(const auto & value, const auto & grid_size){
        return grid_next(value, grid_size, true);
    }

    static constexpr Range_t<T> grid_next_left(const auto & value, const auto & grid_size){
        return grid_next(value, grid_size, false);
    }

    constexpr Range_t<T> gridfy(const auto & grid_size){
        return Range_t<T>(grid(from, grid_size), grid(to, grid_size));
    }

    static constexpr Range_t<T> part_in_grid(const auto & value, const auto & grid_size, const bool & right_part = true){
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

    constexpr Range_t<T> part_right_in_grid(const auto & value, const auto & grid_size) const{
        return part_in_grid(value, grid_size, true);
    }

    constexpr Range_t<T> part_left_in_grid(const auto & value, const auto & grid_size) const{
        return part_in_grid(value, grid_size, false);
    }


    __fast_inline_constexpr Range_t<T> room_left(const Range_t<auto> & _content) const{
        Range_t<T> content = _content.abs();
        Range_t<T> regular = this -> abs();
        return {regular.from, MAX(content.from, regular.from)};
    }

    __fast_inline_constexpr Range_t<T> room_right(const Range_t<auto> & _content) const{
        Range_t<T> content = _content.abs();
        Range_t<T> regular = this -> abs();
        return {regular.from, MIN(content.from, regular.from)};
    }

    __fast_inline_constexpr int rooms(const Range_t<auto> & _content) const{
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


    template<arithmetic U>
    constexpr Range_t<T> grow(const U amount) const{
        Range_t<T> regular = this -> abs();
        Range_t<T> ret = Range_t<T>(regular.from - amount, regular.to + amount);
        if (ret.is_regular()) return ret;
        else return Range_t<T>();
    }

    template<arithmetic U>
    constexpr Range_t<T> merge(const Range_t<U> & other) const {
        Range_t<T> regular = this -> abs();
        Range_t<T> other_regular = other.abs();
        return Range_t<T>(std::min(regular.from, other_regular.from), std::max(regular.to, other_regular.to));
    }

    template<arithmetic U>
    constexpr Range_t<T> shift(const U amount){
        Range_t<T> regular = this -> abs();
        Range_t<T> ret = Range_t<T>(regular.from + amount, regular.to + amount);
        return ret;
    }

    template<arithmetic U>
    constexpr Range_t<T> merge(const U & value){
        Range_t<T> regular = this -> abs();
        return Range_t<T>(MIN(regular.from, value), MAX(regular.to, value));
    }

    template<arithmetic U>
    __fast_inline_constexpr T invlerp(const U & value) const{
        return T((value - from) / (to - from));
    }

    template<arithmetic U>
    __fast_inline_constexpr T lerp(const U & value) const {
        return from + (value) * (to - from);
    }

    template<arithmetic U>
    __fast_inline_constexpr bool operator<(const U & value) const {
        Range_t<T> regular = this -> abs();
        return value < regular.from;
    }

    template<arithmetic U>
    __fast_inline_constexpr bool operator<=(const U & value) const {
        Range_t<T> regular = this -> abs();
        return value <= regular.to;
    }

    template<arithmetic U>
    __fast_inline_constexpr bool operator>(const U & value) const {
        Range_t<T> regular = this -> abs();
        return value > regular.to;
    }
    template<arithmetic U>
    __fast_inline_constexpr bool operator>=(const U & value) const {
        Range_t<T> regular = this -> abs();
        return value >= regular.from;
    }


    __fast_inline_constexpr T clamp(const auto & value) const{
        Range_t<T> regular = this -> abs();
        return CLAMP(value, regular.from, regular.to);
    }

    constexpr explicit operator bool() const{
        return from!= to;
    }

    constexpr  __no_inline explicit operator String() const{
        return toString();
    }

    __no_inline String toString(unsigned char decimalPlaces = 2) const{
        if constexpr(std::is_integral<T>::value){
            return ('[' + String(from) + ',' + String(to) + ')');
        }else{
            return ('[' + ::toString(from, decimalPlaces) + ',' + ::toString(to, decimalPlaces) + ')');
        }
    }
}__packed;

using Rangei = Range_t<int>;
using Range = Range_t<real_t>;

__fast_inline OutputStream & operator<<(OutputStream & os, const Range_t<auto> & value){
    return os << '[' << value.from << ',' << value.to << ')';
}

#endif
