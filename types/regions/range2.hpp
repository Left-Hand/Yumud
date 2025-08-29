#pragma once

#include "core/stream/ostream.hpp"
#include "core/utils/Option.hpp"
#include "core/math/real.hpp"

#include "type_traits"
#include <algorithm>


namespace ymd{
template<typename T>
struct Range2{
public:
    #pragma pack(push, 1)
    T start;
    T stop;
    #pragma pack(pop)

    static constexpr Range2<T> INF = {
        std::numeric_limits<T>::min(), std::numeric_limits<T>::max()};
    static constexpr Range2<T> POS = {T(0), std::numeric_limits<T>::max()};
    static constexpr Range2<T> NEG = {std::numeric_limits<T>::min(), T(0)};

    [[nodiscard]] __fast_inline constexpr Range2(){;}

    template<typename U, typename V>
    [[nodiscard]] __fast_inline constexpr Range2(const U _start, const V _stop): 
    start(floor_cast<T>(_start)), stop(ceil_cast<T>(_stop)) {
        if(stop < start) std::swap(start, stop);
    }

    [[nodiscard]] __fast_inline constexpr Range2(const Range2<T> & other): 
        start(static_cast<T>(other.start)), 
        stop(static_cast<T>(other.stop))
    {
        if(stop < start) std::swap(start, stop); 
    }

    [[nodiscard]] __fast_inline constexpr Range2(const std::pair<T, T> & other): 
        start(static_cast<T>(other.first)), 
        stop(static_cast<T>(other.second))
    {
        if(stop < start) std::swap(start, stop);
    }

    [[nodiscard]] __fast_inline constexpr Range2(const std::tuple<T, T> & other):
        start(static_cast<T>(std::get<0>(other))), 
        stop(static_cast<T>(std::get<1>(other)))
    {
        if(stop < start) std::swap(start, stop);
    }

    template<typename U, typename V>
    [[nodiscard]] __fast_inline static constexpr Range2<T> from_start_and_stop_unchecked(
        const U _start, const V _stop
    ){
        return Range2<T>{
            floor_cast<T>(_start),
            ceil_cast<T>(_stop)
        };
    };


    template<typename U, typename V>
    [[nodiscard]] __fast_inline static constexpr Range2<T> from_start_and_length(
        const U start, const V length)
    {
        return {
            floor_cast<T>(start), 
            ceil_cast<T>(start + length)
        };
    } 

    template<typename U>
    [[nodiscard]] __fast_inline static constexpr Range2<T> from_center_and_length(
        const U center, const U length)
    {
        if constexpr(std::is_integral_v<U>){
            const auto half_length = length >> 1;
            return from_start_and_stop_unchecked(center - half_length, center + half_length);
        }else{
            constexpr U HALF_ONE = static_cast<U>(0.5);
            const auto half_length = (length * HALF_ONE);
            return from_start_and_stop_unchecked(center - half_length, center + half_length);
        }
    } 

    [[nodiscard]] __fast_inline static constexpr Range2<T> from_center_and_half_length(
        const T center, const T half_length)
    {
        return {static_cast<T>(center - half_length), 
                static_cast<T>(center + half_length)};
    } 

    [[nodiscard]] __fast_inline static constexpr Range2<T> from_center(const T center){
        return {center, center};
    } 


    [[nodiscard]] __fast_inline constexpr Range2<T> swap() const {
        return {stop, start};
    }

    [[nodiscard]] __fast_inline constexpr Range2<T> swap_if_inversed() const {
        return start > stop ? swap() : *this;
    }
    
    [[nodiscard]] __fast_inline static constexpr Range2<T> from_start_and_gridsize(
        const T start, const T grid_size)
    {
        const auto resi = [&]{
            if constexpr(std::is_integral_v<T>){
                return (start % grid_size);
            }else{
                return fposmod(start, grid_size);
            }
        }();

        const auto ret_from = start - resi;
        return Range2<T>(ret_from, ret_from + grid_size);
    }

    [[nodiscard]] __fast_inline constexpr T & operator [](const size_t index) 
        { return *(&this->start + index);}

    [[nodiscard]] __fast_inline constexpr const T & operator [](const size_t index) const 
        {return *(&this->start + index);}

    [[nodiscard]] __fast_inline constexpr const T * begin() const { return &this->start;}

    [[nodiscard]] __fast_inline constexpr const T * end() const { return &this->stop;}

    [[nodiscard]] __fast_inline constexpr Range2<T> & operator=(const Range2<auto> & other) {
        this->start = static_cast<T>(other.start);
        this->stop = static_cast<T>(other.stop);
        return *this;
    }

    [[nodiscard]] __fast_inline constexpr bool is_regular() const {
        return start <= stop;
    }

    [[nodiscard]] __fast_inline constexpr T length() const{
        return ABS(stop - start);
    }

    [[nodiscard]] __fast_inline constexpr T length_signed() const{
        return (stop - start);
    }

    [[nodiscard]] constexpr Range2<T> abs() const{
        if((start > stop)) return Range2<T>(stop, start);
        return *this;
    }

    [[nodiscard]] constexpr Range2<T> operator * (const arithmetic auto rhs) const{
        return Range2<T>(this->start * rhs, this->stop * rhs);
    }

    [[nodiscard]] constexpr Range2<T> operator / (const arithmetic auto & rhs) const{
        if constexpr(std::is_integral_v<T>){
            return {this->start / rhs, this->stop / rhs};
        }else{
            const auto inv_rhs = 1 / rhs;
            return {this->start * inv_rhs, this->stop * inv_rhs};
        }
    }


    [[nodiscard]] constexpr bool operator == (const Range2<auto> & other) const {
        return (this->start == other.start && this->stop == other.stop);
    }

    [[nodiscard]] constexpr bool operator != (const Range2<auto> & other) const {
        return !(*this == other);
    }

    [[nodiscard]] constexpr bool intersects(const Range2<auto> & other) const {
        return(this->has(other.start) || other.has(this->start));
    }

    [[nodiscard]] constexpr bool contains(const Range2<auto> & other) const {
        return (this->start <= other.start && this->stop >= other.stop);
    }

    [[nodiscard]] constexpr bool is_inside(const Range2<auto> & other) const {
        return other.contains(*this);
    }

    [[nodiscard]] constexpr bool contains(const arithmetic auto & value) const{
        return (this->start <= static_cast<T>(value) 
            && static_cast<T>(value) < this->stop);
    }

    [[nodiscard]] constexpr T padding(const Range2<T> other) const {
        if(this->intersects(other)) return 0;
        return MIN((other.stop - this->start), (other.start - this->stop));
    }

    [[nodiscard]] constexpr Range2<T> intersection(const Range2<auto> & other) const {
        if(false == this->intersects(other)) return Range2<T>();
        return Range2<T>(MAX(T(this->start), T(other.start)), MIN(T(this->stop), T(other.stop)));
    }

    [[nodiscard]] constexpr T get_center()const{
        return (start + stop) / 2;
    }


    [[nodiscard]] constexpr Range2<T> scale_around_center(const auto & amount){
        const T len = this->length();
        const T center = this->get_center();
        const Range2<T> ret = Range2<T>(center - len * amount / 2, center + len * amount / 2);
        if (ret.is_regular()) return ret;
        else return Range2<T>();
    }


    [[nodiscard]] constexpr Range2<T> expand(const arithmetic auto & value) const{
        const Range2<T> ret = Range2<T>(this->start - value, this->stop + value);
        if (ret.is_regular()) return ret;
        else return Range2<T>();
    }

    [[nodiscard]] constexpr Range2<T> merge(const Range2<arithmetic auto> & other) const {
        return Range2<T>(MIN(this->start, other.start), MAX(this->stop, other.stop));
    }

    [[nodiscard]] constexpr Range2<T> shift(const arithmetic auto & value) const{
        Range2<T> ret = Range2<T>(this->start + value, this->stop + value);
        return ret;
    }

    [[nodiscard]] constexpr Range2<T> merge(const arithmetic auto & value) const{
        return Range2<T>(MIN(T(this->start), T(value)), MAX(T(this->stop), T(value)));
    }

    [[nodiscard]] constexpr T invlerp(const arithmetic auto & value) const{
        return T((value - start) / (stop - start));
    }

    [[nodiscard]] constexpr T lerp(const arithmetic auto & value) const {
        return start + (value) * (stop - start);
    }

    [[nodiscard]] constexpr bool is_complete_less_than(const arithmetic auto & value) const {
        return this->stop < value;
    }

    [[nodiscard]] constexpr bool is_partical_less_than(const arithmetic auto & value) const {
        return this->start < value && this->stop >= value;
    }

    [[nodiscard]] constexpr bool is_complete_great_than(const arithmetic auto & value) const {
        return this->start > value;
    }
    [[nodiscard]] constexpr bool is_partical_great_than(const arithmetic auto & value) const {
        return this->start <= value && this->stop > value;
    }


    [[nodiscard]] constexpr T clamp(const arithmetic auto & value) const{
        return CLAMP(value, this->start, this->stop);
    }

    [[nodiscard]] constexpr T max() const {return MAX(this->start, this->stop);}
    [[nodiscard]] constexpr T min() const {return MIN(this->start, this->stop);}
};

using Range2i = Range2<int>;
using Range2u = Range2<uint>;
using Range2u8 = Range2<uint8_t>;
using Range2u16 = Range2<uint16_t>;
using Range2u32 = Range2<uint32_t>;

template<typename T>
__inline OutputStream & operator<<(
    OutputStream & os, 
    const Range2<T> & value
){
    return os << os.brackets<'['>() << value.start << os.splitter() << value.stop << os.brackets<')'>();
}



struct RangeGridIter{

    constexpr RangeGridIter(
        const Range2u range,
        const uint32_t gsize
    ):
        range_targ_(range),
        gsize_(gsize){;}

    constexpr Option<Range2u> next(
        const Range2u range_in
    )const{
        return next_of_range(range_in, range_targ_, gsize_);
    }

    constexpr Option<Range2u> prev(
        const Range2u range_in
    )const{
        return prev_of_range(range_in, range_targ_, gsize_);
    }

    constexpr Range2u begin()const{
        return begin_of_range(range_targ_, gsize_);
    }

    constexpr Range2u end()const{
        return end_of_range(range_targ_, gsize_);
    }

    template<typename T>
    constexpr std::span<T> subspan(const std::span<T> pbuf, const Range2u range) const{
        const auto offset = range.start - range_targ_.start;
        return pbuf.subspan(offset, range.length());
    }

    constexpr Range2u whole() const {
        return range_targ_;
    }
private:

    static constexpr Range2u begin_of_range(
        const Range2u range,
        const uint gsize
    ){
        const auto grid_remaining = gsize - range.start % gsize;
        return Range2u(range.start, MIN(range.start + grid_remaining, range.stop));
    }

    static constexpr Range2u end_of_range(
        const Range2u range,
        const uint gsize
    ){
        const auto grid_begin = range.stop - range.stop % gsize;
        return Range2u(MAX(range.start, grid_begin), range.stop);
    }
    static constexpr Option<Range2u> next_of_range(
        const Range2u range_in, 
        const Range2u range_targ, 
        const uint gsize
    ){
        const auto end_range = end_of_range(range_targ, gsize);
        if(range_in.stop >= end_range.stop) return None;
        const auto next_stop = MIN(range_in.stop + gsize, range_targ.stop);
        const auto remainder = next_stop % gsize;
        if(remainder == 0) return Some(Range2u{next_stop - gsize , next_stop});
        else return Some(Range2u{next_stop - remainder , next_stop});
    }

    static constexpr Option<Range2u> prev_of_range(
        const Range2u range_in, 
        const Range2u range_targ, 
        const uint gsize
    ){
        const auto begin_range = begin_of_range(range_targ, gsize);
        if(range_in.start <= begin_range.start) return None;
        const auto prev_start = MAX(range_in.start - gsize, range_targ.start);
        const auto remainder = prev_start % gsize;
        return Some(Range2u(prev_start, prev_start - remainder + gsize));
    }

    const Range2u range_targ_;
    const uint32_t gsize_;
};
};