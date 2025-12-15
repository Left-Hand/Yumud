#pragma once

#include "core/math/real.hpp"
#include "core/math/realmath.hpp"

#include "core/stream/ostream.hpp"
#include "core/utils/zero.hpp"

namespace ymd::digipw{

template<typename T>
struct UvwCoord{
    static_assert(std::is_signed_v<T>);
    
    T u;
    T v;
    T w;

    static constexpr UvwCoord<T> ZERO = {T(0), T(0), T(0)};
    enum class Axis:uint8_t{U, V, W};

    template<Axis A1, Axis A2>
    requires (A1 != A2)
    [[nodiscard]] static constexpr UvwCoord from_existing_phase(const T p1, const T p2){
        constexpr Axis MISSING_AXIS = []() constexpr -> Axis {
            if constexpr (A1 != Axis::U && A2 != Axis::U) {
                return Axis::U;
            } else if constexpr (A1 != Axis::V && A2 != Axis::V) {
                return Axis::V;
            } else {
                return Axis::W;
            }
        }();


        UvwCoord<T> ret;
        ret.get_axis<A1>() = p1;
        ret.get_axis<A2>() = p1;
        ret.get_axis<MISSING_AXIS>() = ( - p1 - p2);
        return ret;
    }

    [[nodiscard]] constexpr T operator [](const size_t idx) const {
        return *(&u + idx);
    }

    [[nodiscard]] constexpr T & operator [](const size_t idx){
        return *(&u + idx);
    }

    [[nodiscard]] constexpr T numeric_sum() const {
        return u + v + w;
    }


    template<size_t I>
    requires (I < 3)
    [[nodiscard]] constexpr T & get(){
        if constexpr(I == 0){
            return this->u;
        } else if constexpr(I == 1){
            return this->v;
        }else if constexpr(I == 2){
            return this->w;
        }
    } 

    template<size_t I>
    requires (I < 3)
    [[nodiscard]] constexpr T get() const {
        if constexpr(I == 0){
            return this->u;
        } else if constexpr(I == 1){
            return this->v;
        }else if constexpr(I == 2){
            return this->w;
        }
    }

    template<Axis A>
    [[nodiscard]] constexpr T get_axis() const {
        if constexpr(A == Axis::U){
            return this->u;
        } else if constexpr(A == Axis::V){
            return this->v;
        }else if constexpr(A == Axis::W){
            return this->w;
        }
    }

    template<Axis A>
    [[nodiscard]] constexpr T & get_axis(){
        if constexpr(A == Axis::U){
            return this->u;
        } else if constexpr(A == Axis::V){
            return this->v;
        }else if constexpr(A == Axis::W){
            return this->w;
        }
    }

    friend OutputStream & operator << (OutputStream & os, const UvwCoord & self){
        return os << os.brackets<'('>() << 
            self.u << os.splitter() << 
            self.v <<  os.splitter() << 
            self.w << os.brackets<')'>();
    }
};

}
