#pragma once

#include "core/stream/ostream.hpp"

#include "type_traits"
#include <algorithm>


namespace ymd{
template<arithmetic T>
struct Range2_t{
public:
    #pragma pack(push, 1)
    T from;
    T to;
    #pragma pack(pop)

    static constexpr Range2_t<T> INF = {
        std::numeric_limits<T>::min(), std::numeric_limits<T>::max()};
    static constexpr Range2_t<T> POS = {T(0), std::numeric_limits<T>::max()};
    static constexpr Range2_t<T> NEG = {std::numeric_limits<T>::min(), T(0)};

    [[nodiscard]] __fast_inline constexpr Range2_t():
        from(static_cast<T>(0)),
        to(static_cast<T>(0)){;}

    [[nodiscard]] __fast_inline constexpr Range2_t(const T _from, const T _to): from(_from), to(_to) {
        if(to > from) std::swap(from, to);
    }

    [[nodiscard]] __fast_inline constexpr Range2_t(const Range2_t<T> & other): 
        from(static_cast<T>(other.from)), 
        to(static_cast<T>(other.to))
    {
        *this = this -> abs(); 
    }

    [[nodiscard]] __fast_inline constexpr Range2_t(const std::pair<T, T> & other): 
        from(static_cast<T>(other.first)), 
        to(static_cast<T>(other.second))
    {
        *this = this -> abs(); 
    }

    [[nodiscard]] __fast_inline constexpr Range2_t(const std::tuple<T, T> & other):
        from(static_cast<T>(std::get<0>(other))), 
        to(static_cast<T>(std::get<1>(other)))
    {
        *this = this -> abs(); 
    }

    template<typename U = T>
    [[nodiscard]] __fast_inline static constexpr Range2_t<T> from_dipole(const U dipole){
        return Range2_t<T>{dipole, dipole};
    } 

    [[nodiscard]] __fast_inline static constexpr Range2_t<T> from_center_and_length(
        const arithmetic auto center, const arithmetic auto length)
    {
        if constexpr(std::is_integral_v<T>){
            const auto half_length = length / 2;
            return {center - half_length, center + half_length};
        }else{
            constexpr T HALF_ONE = static_cast<T>(0.5);
            const auto half_length = (length * HALF_ONE);
            return {center - half_length, center + half_length};
        }
    } 

    template<typename U = T>
    [[nodiscard]] __fast_inline static constexpr Range2_t<T> from_center_and_half_length(
        const arithmetic auto center, const arithmetic auto half_length)
    {
        return {static_cast<T>(center - half_length), 
                static_cast<T>(center + half_length)};
    } 

    template<typename U = T>
    [[nodiscard]] __fast_inline static constexpr Range2_t<T> from_start_and_length(
        const arithmetic auto start, const arithmetic auto length)
    {
        return {start, static_cast<T>(start + length)};
    } 
    
    template<typename U = T>
    [[nodiscard]] __fast_inline static constexpr Range2_t<T> from_start_and_gridsize(
        const arithmetic auto start, const arithmetic auto grid_size)
    {
        const auto resi = [&]{
            if constexpr(std::is_integral_v<T>){
                return (start % grid_size);
            }else{
                return fposmodp(start, grid_size);
            }
        }();

        const auto ret_from = start - resi;
        return Range2_t<T>(ret_from, ret_from + grid_size);
    }

    [[nodiscard]] __fast_inline constexpr T & operator [](const size_t index) 
        { return *(&this->from + index);}

    [[nodiscard]] __fast_inline constexpr const T & operator [](const size_t index) const 
        {return *(&this->from + index);}

    [[nodiscard]] __fast_inline constexpr const T * begin() const { return &this->from;}

    [[nodiscard]] __fast_inline constexpr const T * end() const { return &this->to;}

    [[nodiscard]] __fast_inline constexpr Range2_t<T> & operator=(const Range2_t<auto> & other) {
        this->from = static_cast<T>(other.from);
        this->to = static_cast<T>(other.to);
        return *this;
    }

    [[nodiscard]] __fast_inline constexpr bool is_regular() const {
        return from <= to;
    }

    [[nodiscard]] __fast_inline constexpr T length() const{
        return ABS(to - from);
    }

    [[nodiscard]] __fast_inline constexpr T length_signed() const{
        return (to - from);
    }

    [[nodiscard]] constexpr Range2_t<T> abs() const{
        if((from > to)) return Range2_t<T>(to, from);
        return *this;
    }

    [[nodiscard]] constexpr Range2_t<T> operator * (const arithmetic auto rhs) const{
        return Range2_t<T>(this->from * rhs, this->to * rhs);
    }

    [[nodiscard]] constexpr Range2_t<T> operator / (const arithmetic auto & rhs) const{
        if constexpr(std::is_integral_v<T>){
            return {this->from / rhs, this->to / rhs};
        }else{
            const auto inv_rhs = 1 / rhs;
            return {this->from * inv_rhs, this->to * inv_rhs};
        }
    }


    [[nodiscard]] constexpr bool operator == (const Range2_t<auto> & other) const {
        return (this->from == other.from && this->to == other.to);
    }

    [[nodiscard]] constexpr bool operator != (const Range2_t<auto> & other) const {
        return !(*this == other);
    }

    [[nodiscard]] constexpr bool intersects(const Range2_t<auto> & other) const {
        return(this->has(other.from) || other.has(this->from));
    }

    [[nodiscard]] constexpr bool contains(const Range2_t<auto> & other) const {
        return (this->from <= other.from && this->to >= other.to);
    }

    [[nodiscard]] constexpr bool is_inside(const Range2_t<auto> & other) const {
        return other.contains(*this);
    }

    [[nodiscard]] constexpr bool has(const arithmetic auto & value) const{
        return (this->from <= static_cast<T>(value) 
            && static_cast<T>(value) < this->to);
    }

    [[nodiscard]] constexpr bool has(const Range2_t<auto> & other) const{
        return contains(other);}

    [[nodiscard]] constexpr T padding(const Range2_t<T> other) const {
        if(this->intersects(other)) return 0;
        return MIN((other.to - this->from), (other.from - this->to));
    }

    [[nodiscard]] constexpr Range2_t<T> intersection(const Range2_t<auto> & other) const {
        if(false == this->intersects(other)) return Range2_t<T>();
        return Range2_t<T>(MAX(T(this->from), T(other.from)), MIN(T(this->to), T(other.to)));
    }

    [[nodiscard]] constexpr T get_center()const{
        return (from + to) / 2;
    }

    [[nodiscard]] static constexpr Range2_t<T> grid_next_right(
        const arithmetic auto & value, const arithmetic auto & grid_size){
        return from_start_and_gridsize((value + grid_size), grid_size);
    }

    [[nodiscard]] static constexpr Range2_t<T> grid_next_left(
        const arithmetic auto & value, const arithmetic auto & grid_size){
        return from_start_and_gridsize((value - grid_size), grid_size);
    }

    [[nodiscard]] constexpr Range2_t<T> gridfy(const arithmetic auto & grid_size) const {
        return Range2_t<T>(from_start_and_gridsize(from, grid_size), from_start_and_gridsize(to, grid_size));
    }

    [[nodiscard]] static constexpr Range2_t<T> part_in_grid(
        const arithmetic auto & value, const arithmetic auto & grid_size, const bool right_part = true){
        if constexpr(std::is_integral<T>::value){
            if(value % grid_size == 0) return {value, value};
            auto gridfied = from_start_and_gridsize(value, grid_size);
            if(right_part){
                return {value, gridfied.to};
            }else{
                return {gridfied.from, value};
            }
        }
    }

    [[nodiscard]] constexpr Range2_t<T> part_right_in_grid(const arithmetic auto & value, const arithmetic auto & grid_size) const{
        return part_in_grid(value, grid_size, true);
    }

    [[nodiscard]] constexpr Range2_t<T> part_left_in_grid(const arithmetic auto & value, const arithmetic auto & grid_size) const{
        return part_in_grid(value, grid_size, false);
    }


    [[nodiscard]] constexpr Range2_t<T> room_left(const Range2_t<auto> & content) const{
        return {this->from, MAX(content.from, this->from)};
    }

    [[nodiscard]] constexpr Range2_t<T> room_right(const Range2_t<auto> & content) const{
        return {this->from, MIN(content.from, this->from)};
    }

    [[nodiscard]] constexpr int rooms(const Range2_t<auto> & _content) const{
        return bool(room_left(_content)) + bool(room_right(_content));
    }

    [[nodiscard]] constexpr Range2_t<T> grid_forward(const Range2_t<auto> & before, const auto & grid_size) const{

        if(before.from == before.to && before.from == 0){//initial
            auto grid_field = from_start_and_gridsize(this->from, grid_size);
            if(grid_field.has(*this)) return *this;
            else return {this->from, grid_field.to};
        }


        if(part_right_in_grid(before.to, grid_size).length() != 0){
            return {before.to, before.to};
        }

        auto ret = grid_next_right(before.from, grid_size);
        if(ret.has(this->to)){
            return ret.part_left_in_grid(this->to, grid_size);
        }
        return ret;
    }

    [[nodiscard]] constexpr Range2_t<T> scale_around_center(const auto & amount){
        const T len = this->length();
        const T center = this->get_center();
        const Range2_t<T> ret = Range2_t<T>(center - len * amount / 2, center + len * amount / 2);
        if (ret.is_regular()) return ret;
        else return Range2_t<T>();
    }


    [[nodiscard]] constexpr Range2_t<T> expand(const arithmetic auto & value) const{
        const Range2_t<T> ret = Range2_t<T>(this->from - value, this->to + value);
        if (ret.is_regular()) return ret;
        else return Range2_t<T>();
    }

    [[nodiscard]] constexpr Range2_t<T> merge(const Range2_t<arithmetic auto> & other) const {
        return Range2_t<T>(MIN(this->from, other.from), MAX(this->to, other.to));
    }

    [[nodiscard]] constexpr Range2_t<T> shift(const arithmetic auto & value) const{
        Range2_t<T> ret = Range2_t<T>(this->from + value, this->to + value);
        return ret;
    }

    [[nodiscard]] constexpr Range2_t<T> merge(const arithmetic auto & value) const{
        return Range2_t<T>(MIN(T(this->from), T(value)), MAX(T(this->to), T(value)));
    }

    [[nodiscard]] constexpr T invlerp(const arithmetic auto & value) const{
        return T((value - from) / (to - from));
    }

    [[nodiscard]] constexpr T lerp(const arithmetic auto & value) const {
        return from + (value) * (to - from);
    }

    // [[nodiscard]] constexpr bool operator<(const arithmetic auto & value) const {
    //     return value < this->from;
    // }

    // [[nodiscard]] constexpr bool operator<=(const arithmetic auto & value) const {
    //     return value <= this->to;
    // }

    // [[nodiscard]] constexpr bool operator>(const arithmetic auto & value) const {
    //     return value > this->to;
    // }
    // [[nodiscard]] constexpr bool operator>=(const arithmetic auto & value) const {
    //     return value >= this->from;
    // }


    [[nodiscard]] constexpr T clamp(const arithmetic auto & value) const{
        return CLAMP(value, this->from, this->to);
    }

    [[nodiscard]] constexpr T max() const {return MAX(this->from, this->to);}
    [[nodiscard]] constexpr T min() const {return MIN(this->from, this->to);}
};

// constexpr bool operator<(const arithmetic auto & value, const Range2_t<auto> & range){
//     return range.abs() > value;
// }

// constexpr bool operator<=(const arithmetic auto & value, const Range2_t<auto> & range){
//     return range.abs() >= value;
// }

// constexpr bool operator>(const arithmetic auto & value, const Range2_t<auto> & range){
//     return range.abs() < value;
// }

// constexpr bool operator>=(const arithmetic auto & value, const Range2_t<auto> & range){
//     return range.abs() <= value; 
// }


using Range2i = Range2_t<int>;
using Range2i = Range2_t<int>;
using Range2u = Range2_t<uint>;

__inline OutputStream & operator<<(OutputStream & os, const Range2_t<auto> & value){
    return os << os.brackets<'['>() << value.from << os.splitter() << value.to << os.brackets<')'>();
}


};