#pragma once

#include <cstdint>
#include "core/math/real.hpp"
#include "core/utils/Result.hpp"

namespace ymd::details{

template<typename Derived, typename D>
struct [[nodiscard]] _PerUnitCrtpBase{

    constexpr explicit _PerUnitCrtpBase(const auto count):count_(static_cast<D>(
        D(INVLERP(min(), max(), CLAMP(count, min(), max())) * std::numeric_limits<D>::max()))){;}

    template<typename T>
    [[nodiscard]] constexpr T to() const {
        return LERP(
            min(), 
            max(), 
            T(count_) / std::numeric_limits<D>::max()
        );
    } 

    [[nodiscard]] static constexpr std::weak_ordering static_validate(const auto other){
        if(unlikely(other > max())) return std::weak_ordering::greater;
        if(unlikely(other < min())) return std::weak_ordering::less;
        return std::weak_ordering::equivalent;
    }

    [[nodiscard]] constexpr std::weak_ordering validate() const {
        return static_validate(count());
    }

    [[nodiscard]] constexpr auto operator<=>(const auto other) const {
        return count_ <=> other.count_;
    }

    [[nodiscard]] constexpr D count() const {return count_;}
    
private:
    D count_;
    static constexpr D min() {return std::get<0>(Derived::range());}
    static constexpr D max() {return std::get<1>(Derived::range());}
};

}

#define DEF_PER_UNIT(name, dtype, min, max)\
struct [[nodiscard]] name:public ::ymd::details::_PerUnitCrtpBase<name, dtype>{\
    using _PerUnitCrtpBase<name, dtype>::_PerUnitCrtpBase;\
    [[nodiscard]] static constexpr std::tuple<dtype, dtype> range(){\
        return {static_cast<dtype>(min),static_cast<dtype>(max)};}\
    [[nodiscard]] static constexpr Result<name, std::weak_ordering>with_validation(const auto count){\
        const auto validation = _PerUnitCrtpBase<name, dtype>::static_validate(count);\
        if(validation != std::weak_ordering::equivalent) return Err(validation);\
        return Ok(name(count));\
    }\
};
