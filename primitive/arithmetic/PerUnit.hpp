#pragma once

#include <cstdint>
#include "core/math/real.hpp"
#include "core/utils/Result.hpp"

namespace ymd::details{

template<typename Derived, typename D>
struct [[nodiscard]] _PerUnitCrtpBase{

    constexpr explicit _PerUnitCrtpBase(const auto count):count_(static_cast<D>(
        D(INVLERP(_get_min(), _get_max(), CLAMP(count, _get_min(), _get_max())) * std::numeric_limits<D>::max()))){;}

    template<typename T>
    [[nodiscard]] constexpr T to() const {
        return LERP(
            _get_min(), 
            _get_max(), 
            T(count_) / std::numeric_limits<D>::max()
        );
    } 

    [[nodiscard]] static constexpr std::weak_ordering static_validate(const auto other){
        if((other > _get_max())) [[unlikely]]
            return std::weak_ordering::greater;
        if((other < _get_min())) [[unlikely]]
            return std::weak_ordering::less;
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
    static constexpr D _get_min() {return std::get<0>(Derived::sweep());}
    static constexpr D _get_max() {return std::get<1>(Derived::sweep());}
};

}

#define DEF_PER_UNIT(name, dtype, min, max)\
struct [[nodiscard]] name:public ::ymd::details::_PerUnitCrtpBase<name, dtype>{\
    using _PerUnitCrtpBase<name, dtype>::_PerUnitCrtpBase;\
    [[nodiscard]] static constexpr std::tuple<dtype, dtype> sweep(){\
        return {static_cast<dtype>(min),static_cast<dtype>(max)};}\
    [[nodiscard]] static constexpr Result<name, std::weak_ordering>with_validation(const auto count){\
        const auto validation = _PerUnitCrtpBase<name, dtype>::static_validate(count);\
        if(validation != std::weak_ordering::equivalent) return Err(validation);\
        return Ok(name(count));\
    }\
};
