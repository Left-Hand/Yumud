#pragma once

#include "core/math/real.hpp"
#include "core/math/iq/fixed_t.hpp"
#include "core/utils/Result.hpp"


namespace ymd::math{

//为高精度定点卷积而生
//普通定点卷积算式时可能有多次精度丢失与溢出风险
//中间量使用拓展量则能避免这个问题
template<typename ET>
struct [[nodiscard]] AccumulationBuilder final{
    using Self = AccumulationBuilder<ET>;
    explicit constexpr AccumulationBuilder(ET value) : value_(value){}

    constexpr AccumulationBuilder add(const auto value) const {
        return AccumulationBuilder(value_ + static_cast<ET>(value));
    }
    constexpr AccumulationBuilder madd(const auto lhs, const auto rhs) const {
        return AccumulationBuilder(value_ + math::extended_mul(lhs, rhs));
    }

    constexpr ET get() const{
        return value_;
    }

    template<typename T>
    constexpr Result<T, std::strong_ordering> try_get() const {
        constexpr auto E_TARGET_MAX = static_cast<ET>(std::numeric_limits<T>::max());
        constexpr auto E_TARGET_MIN = static_cast<ET>(std::numeric_limits<T>::min());
        if(value_ > E_TARGET_MAX) return Err(std::strong_ordering::greater);
        if(value_ < E_TARGET_MIN) return Err(std::strong_ordering::less);
        return Ok(static_cast<T>(value_));
    }

    template<typename T>
    constexpr T get_saturated() const {
        constexpr T TARGET_MAX = std::numeric_limits<T>::max();
        constexpr T TARGET_MIN = std::numeric_limits<T>::min();
        constexpr ET E_TARGET_MAX = static_cast<ET>(TARGET_MAX);
        constexpr ET E_TARGET_MIN = static_cast<ET>(TARGET_MIN);
        if(value_ > E_TARGET_MAX) return TARGET_MAX;
        if(value_ < E_TARGET_MIN) return TARGET_MIN;
        return static_cast<T>(value_);
    }

    template<typename T = ET>
    constexpr T get_clamped(const auto min, const auto max) const{
        if(value_ > max) return static_cast<T>(max);
        if(value_ < min) return static_cast<T>(min);
        return static_cast<T>(value_);
    }

    // 检查当前值（不修改）
    template<typename Fn>
    requires std::is_invocable_v<Fn, ET>
    constexpr const AccumulationBuilder& inspect(Fn&& fn) const noexcept {
        std::invoke(std::forward<Fn>(fn), value_);
        return *this;
    }
    
    // 算术运算符（可选）
    constexpr AccumulationBuilder operator+(ET rhs) const noexcept {
        return AccumulationBuilder(value_ + rhs);
    }
    
    constexpr AccumulationBuilder operator*(ET rhs) const noexcept {
        return AccumulationBuilder(value_ * rhs);
    }

    // 变换当前值
    template<typename Fn>
    requires std::is_invocable_v<Fn, ET> && 
                std::is_convertible_v<std::invoke_result_t<Fn, ET>, ET>
    constexpr AccumulationBuilder& transform(Fn&& fn) noexcept {
        value_ = std::invoke(std::forward<Fn>(fn), value_);
        return *this;
    }

    // 比较运算符
    [[nodiscard]] constexpr bool operator==(const AccumulationBuilder&) const = default;
    

    // 重置累加器
    constexpr void reset(ET new_value = 0) noexcept {
        value_ = new_value;
    }

    // 转换为其他累加器类型
    template<typename NewExtendedT>
    [[nodiscard]] constexpr AccumulationBuilder<NewExtendedT> recast() const {
        return AccumulationBuilder<NewExtendedT>(
            static_cast<NewExtendedT>(value_)
        );
    }
private:
    ET value_;
};
}