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

    static constexpr Range_t<T> grid(const auto & value, const auto & grid_size){
        T ret_start;
        if constexpr(std::is_integral<T>::value){
            ret_start = (value / grid_size) * grid_size;
        }else{
            ret_start = floor(value / grid_size) * grid_size;
        }
        return Range_t<T>(ret_start, ret_start + grid_size);
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
        return Range_t<T>(grid(start, grid_size), grid(end, grid_size));
    }

    static constexpr Range_t<T> part_in_grid(const auto & value, const auto & grid_size, const bool & right_part = true){
        if constexpr(std::is_integral<T>::value){
            if(value % grid_size == 0) return {value, value};
            auto gridfied = grid(value, grid_size);
            if(right_part){
                return {value, gridfied.end};
            }else{
                return {gridfied.start, value};
            }
        }
    }

    constexpr Range_t<T> part_right_in_grid(const auto & value, const auto & grid_size) const{
        return part_in_grid(value, grid_size, true);
    }

    constexpr Range_t<T> part_left_in_grid(const auto & value, const auto & grid_size) const{
        return part_in_grid(value, grid_size, false);
    }



    // constexpr bool lean_left_outside(const const Range_t<T> src){
    //     Range_t<T> regular = this -> abs();
    //     Range_t<T> src_regular = src.abs();

    //     // f | src.f | t | src.t
    //     return src_regular.has_value(regular.start) && regular.has_value(src_regular.end);
    // }

    // constexpr bool lean_right_outside(const const Range_t<T> src){
    //     Range_t<T> regular = this -> abs();
    //     Range_t<T> src_regular = src.abs();
    //     // src.f | f | src.t | 

    // }
    // __fast_inline_constexpr Range_t<T> room_left(const Range_t<auto> & _content) const{
    //     Range_t<T> content = _content.abs();
    //     Range_t<T> regular = this -> abs();
    //     return {regular.start, MAX(content.start, regular.start)};
    // }

    __fast_inline_constexpr Range_t<T> room_left(const Range_t<auto> & _content) const{
        Range_t<T> content = _content.abs();
        Range_t<T> regular = this -> abs();
        return {regular.start, MAX(content.start, regular.start)};
    }

    __fast_inline_constexpr Range_t<T> room_right(const Range_t<auto> & _content) const{
        Range_t<T> content = _content.abs();
        Range_t<T> regular = this -> abs();
        return {regular.start, MIN(content.start, regular.start)};
    }

    __fast_inline_constexpr int rooms(const Range_t<auto> & _content) const{
        return bool(room_left(_content)) + bool(room_right(_content));
    }



    constexpr Range_t<T> grid_forward(const Range_t<auto> & _before, const auto & grid_size) const{
        Range_t<T> before = _before.abs();
        Range_t<T> regular = this -> abs();

        if(before.start == before.end && before.start == 0){//initial
            auto grid_field = grid(regular.start, grid_size);
            if(grid_field.has(regular)) return regular;
            else return {regular.start, grid_field.end};
        }

        if(part_right_in_grid(before.end, grid_size)){
            return {before.end, before.end};
        }

        if(part_left_in_grid(before.start, grid_size)){
            return grid_next_right(before.start, grid_size);
        }
        else{
            auto ret = grid_next_right(before.start, grid_size);
            if(ret.has(regular.end)){
                return ret.part_left_in_grid(regular.end, grid_size);
            }
            return ret;
        }

        // should never reach here
        return{0, 0};
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
