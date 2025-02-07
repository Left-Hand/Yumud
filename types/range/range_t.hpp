#pragma once

#include "sys/math/real.hpp"
#include "sys/core/platform.h"
#include "sys/stream/ostream.hpp"

#include "type_traits"
#include <algorithm>


namespace ymd{
template<arithmetic T>
struct Range_t{
public:
    #pragma pack(push, 1)
    T from;
    T to;
    #pragma pack(pop)

    scexpr Range_t<T> INF = {std::numeric_limits<T>::min(), std::numeric_limits<T>::max()};
    scexpr Range_t<T> POS = {T(0), std::numeric_limits<T>::max()};
    scexpr Range_t<T> NEG = {std::numeric_limits<T>::min(), T(0)};
    // constexpr Range_t(): from(std::numeric_limits<T>::min()), to(std::numeric_limits<T>::max()) {;}

    constexpr Range_t() = default;

    constexpr Range_t(const auto & _from, const auto & _to): from(static_cast<T>(_from)), to(static_cast<T>(_to)) {
        *this = this -> abs();
    }

    constexpr Range_t(const Range_t<auto> & other): from(static_cast<T>(other.from)), to(static_cast<T>(other.to)) {
        *this = this -> abs(); 
    }

    constexpr Range_t(const std::pair<arithmetic auto, arithmetic auto> & other): 
    from(static_cast<T>(other.first)), to(static_cast<T>(other.second)) {
        *this = this -> abs(); 
    }

    constexpr Range_t(const std::tuple<auto, auto> & other):
    from(static_cast<T>(std::get<0>(other))), to(static_cast<T>(std::get<1>(other))) {
        *this = this -> abs(); 
    }

    T & operator [](const size_t index) { return *(&this->from + index);}

    const T & operator [](const size_t index) const {return *(&this->from + index);}

    const T * begin() const { return &this->from;}

    const T * end() const { return &this->to;}

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
        if(unlikely(from > to)) return Range_t<T>(to, from);
        else return *this;
    }

    // constexpr Range_t<T> operator + (const arithmetic auto & rvalue) const{
    //     Range_t<T> regular = this -> abs();
    //     return Range_t<T>(this->from + rvalue, this->to + rvalue);
    // }

    // constexpr Range_t<T> operator - (const arithmetic auto & rvalue) const{
    //     Range_t<T> regular = this -> abs();
    //     return Range_t<T>(this->from - rvalue, this->to - rvalue);
    // }

    constexpr Range_t<T> operator * (const arithmetic auto rvalue) const{
        return Range_t<T>(this->from * rvalue, this->to * rvalue);
    }

    __fast_inline constexpr Range_t<T> operator / (const arithmetic auto & rvalue) const{
        return Range_t<T>(this->from / rvalue, this->to / rvalue);
    }

    // constexpr Range_t<T> & operator += (const arithmetic auto & rvalue) {
    //     *this = *this + rvalue;
    //     return *this;
    // }

    // [[deprecated]] constexpr Range_t<T> & operator -= (const arithmetic auto & rvalue) {
    //     *this = *this - rvalue;
    //     return *this;
    // }

    constexpr Range_t<T> & operator *= (const arithmetic auto & rvalue) {
        *this = *this * rvalue;
        return *this;
    }

    constexpr Range_t<T> & operator /= (const arithmetic auto & rvalue){
        *this = *this / rvalue;
        return *this;
    }

    constexpr bool operator == (const Range_t<auto> & other) const {
        return (this->from == other.from && this->to == other.to);
    }

    constexpr bool operator!= (const Range_t<auto> & other) const {
        return !(*this == other);
    }

    constexpr bool intersects(const Range_t<auto> & other) const {
        return(this->has(other.from) || other.has(this->from));
    }

    constexpr bool contains(const Range_t<auto> & other) const {
        return (this->from <= other.from && this->to >= other.to);
    }

    constexpr bool inside(const Range_t<auto> & other) const {
        return other.contains(*this);
    }

    constexpr bool has(const arithmetic auto & value) const{
        return (this->from <= static_cast<T>(value) && static_cast<T>(value) < this->to);
    }

    constexpr bool has(const Range_t<auto> & other) const{return contains(other);}

    constexpr T padding(const Range_t<T> other) const {
        if(this->intersects(other)) return 0;
        return MIN((other.to - this->from), (other.from - this->to));
    }

    constexpr Range_t<T> intersection(const Range_t<auto> & other) const {
        if(false == this->intersects(other)) return Range_t<T>();
        return Range_t<T>(MAX(T(this->from), T(other.from)), MIN(T(this->to), T(other.to)));
    }

    constexpr T get_center()const{
        return (from + to) / 2;
    }

    scexpr Range_t<T> grid(const arithmetic auto & value, const auto & grid_size){
        T ret_from;
        if constexpr(std::is_integral_v<T>){
            ret_from = (value / grid_size) * grid_size;
        }else{
            ret_from = floor(value / grid_size) * grid_size;
        }
        return Range_t<T>(ret_from, ret_from + grid_size);
    }

    scexpr Range_t<T> grid_next(const arithmetic auto & value, const arithmetic auto & grid_size, const bool right = true){
        return grid(right ? (value + grid_size) : (value-grid_size), grid_size);
    }

    scexpr Range_t<T> grid_next_right(const arithmetic auto & value, const arithmetic auto & grid_size){
        return grid_next(value, grid_size, true);
    }

    scexpr Range_t<T> grid_next_left(const arithmetic auto & value, const arithmetic auto & grid_size){
        return grid_next(value, grid_size, false);
    }

    constexpr Range_t<T> gridfy(const arithmetic auto & grid_size) const {
        return Range_t<T>(grid(from, grid_size), grid(to, grid_size));
    }

    scexpr Range_t<T> part_in_grid(const arithmetic auto & value, const arithmetic auto & grid_size, const bool right_part = true){
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


    constexpr Range_t<T> room_left(const Range_t<auto> & content) const{
        return {this->from, MAX(content.from, this->from)};
    }

    constexpr Range_t<T> room_right(const Range_t<auto> & content) const{
        return {this->from, MIN(content.from, this->from)};
    }

    constexpr int rooms(const Range_t<auto> & _content) const{
        return bool(room_left(_content)) + bool(room_right(_content));
    }

    constexpr Range_t<T> grid_forward(const Range_t<auto> & before, const auto & grid_size) const{

        if(before.from == before.to && before.from == 0){//initial
            auto grid_field = grid(this->from, grid_size);
            if(grid_field.has(*this)) return *this;
            else return {this->from, grid_field.to};
        }


        if(part_right_in_grid(before.to, grid_size)){
            return {before.to, before.to};
        }

        auto ret = grid_next_right(before.from, grid_size);
        if(ret.has(this->to)){
            return ret.part_left_in_grid(this->to, grid_size);
        }
        return ret;
    }

    constexpr Range_t<T> scale(const auto & amount){
        const T len = this->length();
        const T center = this->get_center();
        const Range_t<T> ret = Range_t<T>(center - len * amount / 2, center + len * amount / 2);
        if (ret.is_regular()) return ret;
        else return Range_t<T>();
    }


    constexpr Range_t<T> grow(const arithmetic auto & value) const{
        const Range_t<T> ret = Range_t<T>(this->from - value, this->to + value);
        if (ret.is_regular()) return ret;
        else return Range_t<T>();
    }

    constexpr Range_t<T> merge(const Range_t<arithmetic auto> & other) const {
        return Range_t<T>(MIN(this->from, other.from), MAX(this->to, other.to));
    }

    constexpr Range_t<T> shift(const arithmetic auto & value) const{
        Range_t<T> ret = Range_t<T>(this->from + value, this->to + value);
        return ret;
    }

    constexpr Range_t<T> merge(const arithmetic auto & value) const{
        return Range_t<T>(MIN(T(this->from), T(value)), MAX(T(this->to), T(value)));
    }

    constexpr T invlerp(const arithmetic auto & value) const{
        return T((value - from) / (to - from));
    }

    constexpr T lerp(const arithmetic auto & value) const {
        return from + (value) * (to - from);
    }

    constexpr bool operator<(const arithmetic auto & value) const {
        return value < this->from;
    }

    constexpr bool operator<=(const arithmetic auto & value) const {
        return value <= this->to;
    }

    constexpr bool operator>(const arithmetic auto & value) const {
        return value > this->to;
    }
    constexpr bool operator>=(const arithmetic auto & value) const {
        return value >= this->from;
    }


    constexpr T clamp(const arithmetic auto & value) const{
        return CLAMP(value, this->from, this->to);
    }

    constexpr explicit operator bool() const{
        return from != to;
    }
};

constexpr bool operator<(const arithmetic auto & value, const Range_t<auto> & range){
    return range.abs() > value;
}

constexpr bool operator<=(const arithmetic auto & value, const Range_t<auto> & range){
    return range.abs() >= value;
}

constexpr bool operator>(const arithmetic auto & value, const Range_t<auto> & range){
    return range.abs() < value;
}

constexpr bool operator>=(const arithmetic auto & value, const Range_t<auto> & range){
    return range.abs() <= value; 
}


using Rangei = Range_t<int>;
using Range = Range_t<real_t>;

// namespace ymd{
__inline OutputStream & operator<<(OutputStream & os, const Range_t<auto> & value){
    return os << '[' << value.from << ',' << value.to << ')';
}

// };

};