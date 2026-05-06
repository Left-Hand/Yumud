#pragma once

#include "core/stream/ostream.hpp"
#include "core/utils/Option.hpp"
#include "core/math/real.hpp"
#include "type_traits"
#include <algorithm>


namespace ymd::math{

template<typename T>
struct alignas(sizeof(T) * 2) [[nodiscard]] Range2{
public:
    T start;
    T stop;

    using Tsigned = std::make_signed_t<T>;

    static constexpr Range2<T> INF = {
        std::numeric_limits<T>::min(), std::numeric_limits<T>::max()};
    static constexpr Range2<T> POS = {T(0), std::numeric_limits<T>::max()};
    static constexpr Range2<T> NEG = {std::numeric_limits<T>::min(), T(0)};


    template<typename U, typename V>
    [[nodiscard]] __attribute__((always_inline)) constexpr 
    Range2(const U _start, const V _stop):
        start(math::floor_cast<T>(_start)),
        stop(math::ceil_cast<T>(_stop))
    {
        if(stop < start) std::swap(start, stop);
    }

    template<typename U>
    [[nodiscard]] __attribute__((always_inline)) constexpr 
    Range2(const Range2<U> & other):
        Range2(other.start, other.stop){;}

    [[nodiscard]] __attribute__((always_inline)) constexpr 
    Range2(const std::pair<T, T> & other):
        Range2(other.first, other.second){;}

    [[nodiscard]] __attribute__((always_inline)) constexpr 
    Range2(const std::tuple<T, T> & other):
        Range2(std::get<0>(other), std::get<1>(other)) {;}
    static constexpr Range2<T> from_uninitialized(){
        return Range2();
    }

    template<typename U, typename V>
    [[nodiscard]] __attribute__((always_inline)) static 
    constexpr Range2<T> from_start_and_stop_unchecked(
        const U _start, const V _stop
    ){
        auto ret = Range2<T>::from_uninitialized();
        ret.start = math::floor_cast<T>(_start);
        ret.stop = math::ceil_cast<T>(_stop);
        return ret;
    };

    template<typename U, typename V>
    [[nodiscard]] __attribute__((always_inline)) static 
    constexpr Range2<T> from_start_and_stop(
        const U _start, const V _stop
    ){
        return Range2<T>{_start, _stop};
    };


    template<typename U, typename V>
    [[nodiscard]] __attribute__((always_inline)) static 
    constexpr Range2<T> from_start_and_length(
        const U start, const V length)
    {
        return Range2<T>{
            start,
            start + length
        };
    }

    template<typename U>
    [[nodiscard]] __attribute__((always_inline)) static 
    constexpr Range2<T> from_center_and_length(
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

    [[nodiscard]] __attribute__((always_inline)) static 
    constexpr Range2<T> from_center_and_half_length(
        const T center, const T half_length)
    {
        return {static_cast<T>(center - half_length),
                static_cast<T>(center + half_length)};
    }

    [[nodiscard]] __attribute__((always_inline)) static 
    constexpr Range2<T> from_center(const T center){
        return {center, center};
    }


    [[nodiscard]] __attribute__((always_inline)) constexpr 
    Range2<T> swap() const noexcept {
        return {stop, start};
    }

    [[nodiscard]] __attribute__((always_inline)) constexpr 
    Range2<T> swap_if_inversed() const noexcept {
        return start > stop ? swap() : *this;
    }

    [[nodiscard]] __attribute__((always_inline)) static 
    constexpr Range2<T> from_start_and_gridsize(
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

    [[nodiscard]] __attribute__((always_inline)) constexpr 
    T & operator [](const size_t index)
        { return *(&this->start + index);}

    [[nodiscard]] __attribute__((always_inline)) constexpr 
    const T & operator [](const size_t index) const
        {return *(&this->start + index);}

    [[nodiscard]] __attribute__((always_inline)) constexpr 
    const T * begin() const noexcept { return &this->start;}

    [[nodiscard]] __attribute__((always_inline)) constexpr 
    const T * end() const noexcept { return &this->stop;}

    template<typename U>
    [[nodiscard]] __attribute__((always_inline)) constexpr 
    Range2<T> & operator=(const Range2<U> & other) {
        this->start = static_cast<T>(other.start);
        this->stop = static_cast<T>(other.stop);
        return *this;
    }

    [[nodiscard]] __attribute__((always_inline)) constexpr 
    bool is_regular() const noexcept {
        return start <= stop;
    }

    [[nodiscard]] __attribute__((always_inline)) constexpr 
    T length() const noexcept {
        if(stop > start) return static_cast<T>(stop - start);
        return static_cast<T>(start - stop);
        // return std::abs(stop - start);
    }

    [[nodiscard]] __attribute__((always_inline)) constexpr 
    T length_unchecked() const noexcept {
        return stop - start;
    }

    [[nodiscard]] __attribute__((always_inline)) constexpr 
    T half_length_unchecked() const noexcept {
        return (stop - start) >> 1;
    }

    [[nodiscard]] __attribute__((always_inline)) constexpr 
    T length_signed() const noexcept {
        return (stop - start);
    }

    [[nodiscard]] constexpr Range2<T> abs() const noexcept {
        if((start > stop)) return Range2<T>(stop, start);
        return *this;
    }

    [[nodiscard]] constexpr Range2<T> operator * (const arithmetic auto rhs) const noexcept {
        return Range2<T>(this->start * rhs, this->stop * rhs);
    }

    [[nodiscard]] constexpr Range2<T> operator / (const arithmetic auto & rhs) const noexcept {
        if constexpr(std::is_integral_v<T>){
            return {this->start / rhs, this->stop / rhs};
        }else{
            const auto inv_rhs = 1 / rhs;
            return {this->start * inv_rhs, this->stop * inv_rhs};
        }
    }


    template<typename U>
    [[nodiscard]] constexpr bool operator == (const Range2<U> & other) const noexcept {
        return (this->start == other.start && this->stop == other.stop);
    }

    template<typename U>
    [[nodiscard]] constexpr bool operator != (const Range2<U> & other) const noexcept {
        return !(*this == other);
    }

    template<typename U>
    [[nodiscard]] constexpr bool intersects(const Range2<U> & other) const noexcept {
        return(this->contains(other.start) || other.contains(this->start));
    }

    template<typename U>
    [[nodiscard]] constexpr bool contains(const Range2<U> & other) const noexcept {
        return (this->start <= other.start && this->stop >= other.stop);
    }

    template<typename U>
    [[nodiscard]] constexpr bool is_inside(const Range2<U> & other) const noexcept {
        return other.contains(*this);
    }

    [[nodiscard]] constexpr bool contains(const arithmetic auto & value) const noexcept {
        return (this->start <= static_cast<T>(value)
            && static_cast<T>(value) < this->stop);
    }

    [[nodiscard]] constexpr T padding(const Range2<T> other) const noexcept {
        if(this->intersects(other)) return 0;
        return MIN((other.stop - this->start), (other.start - this->stop));
    }

    template<typename U>
    [[nodiscard]] constexpr Range2<T> intersection(const Range2<U> & other) const noexcept {
        if(false == this->intersects(other)) return Range2<T>();
        return Range2<T>(MAX(T(this->start), T(other.start)), MIN(T(this->stop), T(other.stop)));
    }

    [[nodiscard]] constexpr T center() const noexcept {
        return (start + stop) / 2;
    }


    [[nodiscard]] constexpr Range2<T> scale_around_center(const auto & amount){
        const T len = this->length();
        const T center = this->center();
        const Range2<T> ret = Range2<T>(center - len * amount / 2, center + len * amount / 2);
        if (ret.is_regular()) return ret;
        else return Range2<T>();
    }


    [[nodiscard]] constexpr Option<Range2<T>> expand(const arithmetic auto & value) const noexcept {
        const auto next_start = this->start - value;
        const auto next_stop = this->stop + value;

        if(next_start > next_stop) return None;
        return Some(Range2<T>::from_start_and_stop_unchecked(next_start, next_stop));
    }

    [[nodiscard]] constexpr Option<Range2<T>> shrink(const arithmetic auto & value) const noexcept {

        const auto next_start = this->start + value;
        const auto next_stop = this->stop - value;

        if(next_start > next_stop) return None;
        return Some(Range2<T>::from_start_and_stop_unchecked(next_start, next_stop));
    }

    [[nodiscard]] constexpr Range2<T> merge(const Range2<arithmetic auto> & other) const noexcept {
        return Range2<T>(MIN(this->start, other.start), MAX(this->stop, other.stop));
    }

    [[nodiscard]] constexpr Range2<T> shift(const arithmetic auto & value) const noexcept {
        Range2<T> ret = Range2<T>(this->start + value, this->stop + value);
        return ret;
    }

    [[nodiscard]] constexpr Range2<T> merge(const arithmetic auto & value) const noexcept {
        return Range2<T>(MIN(T(this->start), T(value)), MAX(T(this->stop), T(value)));
    }

    [[nodiscard]] constexpr T invlerp(const arithmetic auto & value) const noexcept {
        return T((value - start) / (stop - start));
    }

    [[nodiscard]] constexpr T lerp(const arithmetic auto & value) const noexcept {
        return start + (value) * (stop - start);
    }

    [[nodiscard]] constexpr bool is_complete_less_than(const arithmetic auto & value) const noexcept {
        return this->stop < value;
    }

    [[nodiscard]] constexpr bool is_partical_less_than(const arithmetic auto & value) const noexcept {
        return this->start < value && this->stop >= value;
    }

    [[nodiscard]] constexpr bool is_complete_great_than(const arithmetic auto & value) const noexcept {
        return this->start > value;
    }
    [[nodiscard]] constexpr bool is_partical_great_than(const arithmetic auto & value) const noexcept {
        return this->start <= value && this->stop > value;
    }


    [[nodiscard]] constexpr T clamp(const arithmetic auto & value) const noexcept {
        return CLAMP(value, this->start, this->stop);
    }

    [[nodiscard]] constexpr T max() const noexcept {return MAX(this->start, this->stop);}
    [[nodiscard]] constexpr T min() const noexcept {return MIN(this->start, this->stop);}

private:
    [[nodiscard]] __attribute__((always_inline)) constexpr 
    Range2(){;}


};

using Range2i = Range2<int>;
using Range2u = Range2<size_t>;

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



struct [[nodiscard]] RangeGridIter{

    constexpr RangeGridIter(
        const Range2u32 range,
        const uint32_t gsize
    ):
        range_targ_(range),
        gsize_(gsize){;}

    constexpr Option<Range2u32> next(
        const Range2u32 range_in
    ) const noexcept {
        return next_of_range(range_in, range_targ_, gsize_);
    }

    constexpr Option<Range2u32> prev(
        const Range2u32 range_in
    ) const noexcept {
        return prev_of_range(range_in, range_targ_, gsize_);
    }

    constexpr Range2u32 begin() const noexcept {
        return begin_of_range(range_targ_, gsize_);
    }

    constexpr Range2u32 end() const noexcept {
        return end_of_range(range_targ_, gsize_);
    }

    template<typename T>
    constexpr std::span<T> subspan(const std::span<T> pbuf, const Range2u32 range) const noexcept {
        const auto offset = range.start - range_targ_.start;
        return pbuf.subspan(offset, range.length());
    }

    constexpr Range2u32 whole() const noexcept {
        return range_targ_;
    }
private:

    static constexpr Range2u32 begin_of_range(
        const Range2u32 range,
        const size_t gsize
    ){
        const auto grid_remaining = gsize - range.start % gsize;
        return Range2u32(range.start, std::min(range.start + grid_remaining, range.stop));
    }

    static constexpr Range2u32 end_of_range(
        const Range2u32 range,
        const size_t gsize
    ){
        const uint32_t grid_begin = range.stop - range.stop % gsize;
        return Range2u32(std::max(range.start, grid_begin), range.stop);
    }
    static constexpr Option<Range2u32> next_of_range(
        const Range2u32 range_in,
        const Range2u32 range_targ,
        const size_t gsize
    ){
        const auto end_range = end_of_range(range_targ, gsize);
        if(range_in.stop >= end_range.stop) return None;
        const auto next_stop = std::min(range_in.stop + gsize, range_targ.stop);
        const auto remainder = next_stop % gsize;
        if(remainder == 0) return Some(Range2u32{next_stop - gsize , next_stop});
        else return Some(Range2u32{next_stop - remainder , next_stop});
    }

    static constexpr Option<Range2u32> prev_of_range(
        const Range2u32 range_in,
        const Range2u32 range_targ,
        const size_t gsize
    ){
        const auto begin_range = begin_of_range(range_targ, gsize);
        if(range_in.start <= begin_range.start) return None;
        const auto prev_start = std::max(range_in.start - gsize, range_targ.start);
        const auto remainder = prev_start % gsize;
        return Some(Range2u32(prev_start, prev_start - remainder + gsize));
    }

    const Range2u32 range_targ_;
    const uint32_t gsize_;
};
};
