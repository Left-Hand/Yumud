#pragma once

#include "core/platform.hpp"

#include <optional>
#include <variant>

namespace ymd::hal{
struct BusError{
public:
    enum Kind:uint8_t{
        AlreadyUnderUse = 0,
        OccuipedByOther,
        AckTimeout,
        BusOverload,
        SelecterOutOfRange,
        NoSelecter,
        PayloadNoLength,
        VerifyFailed,
        LengthOverflow,
        Unspecified = 0x7f
    };
    
    static consteval BusError Ok(){
        return {std::nullopt};
    }

    constexpr BusError(std::nullopt_t):kind_(std::nullopt){;}
    constexpr BusError(const Kind & kind):kind_(kind){;}
    constexpr BusError(Kind && kind):kind_(kind){;}
    constexpr BusError(const BusError & other):kind_(other.kind_){;}
    constexpr BusError(BusError && other):kind_(other.kind_){;}
    constexpr BusError & operator = (const BusError & other) = default;
    constexpr BusError & operator = (BusError && other) = default;
    __fast_inline constexpr BusError & emplace(const BusError & other){kind_ = other.kind_; return *this;}
    __fast_inline constexpr BusError & emplace(BusError && other){kind_ = other.kind_; return *this;}

    constexpr bool operator ==(const Kind kind) const {return kind_.has_value() and (kind_.value() == kind);}
    constexpr bool operator !=(const Kind kind) const {return kind_.has_value() or (kind_.value() != kind);}


    constexpr bool operator ==(const BusError & other) const {return kind_ == other.kind_;}
    constexpr bool operator !=(const BusError & other) const {return kind_ != other.kind_;}


    __fast_inline constexpr bool is_err() const {return unlikely(kind_.has_value());}
    __fast_inline constexpr bool is_ok() const {return likely(!kind_.has_value());}

    // 链式处理
    template<typename Fn>
    BusError then(Fn && fn){
        if (is_ok()) return std::forward<Fn>(fn)();
        return *this;
    }

    BusError operator | (const BusError rhs) const{
        if(is_err()) return *this;
        else return rhs;
    }

    // constexpr explicit operator Kind() const {return kind_.value();}
    // constexpr Kind kind() const {return kind_.value();}

    constexpr Kind unwrap_err() const {return kind_.value();}

private:
    std::optional<Kind> kind_;
};

}