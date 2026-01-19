#pragma once

#include "core/platform.hpp"

#include <optional>
#include <variant>

namespace ymd::hal{


class HalError{
public:
    enum class Kind:uint8_t{
        AlreadyUnderUse = 0,
        OccuipedByOther,
        WritePayloadAckTimeout,
        SlaveAddrAckTimeout,
        BusOverload,
        SelecterOutOfRange,
        NoSelecter,
        PayloadNoLength,
        VerifyFailed,
        LengthOverflow,
        InvalidArgument,
        TxLengthAndRxLengthNotMatch,
        Unspecified = 0x7f
    };

    using enum Kind;

    constexpr HalError(const HalError & other): kind_(other.kind_){;}
    constexpr HalError(HalError && other): kind_(other.kind_){;}

    constexpr HalError(const Kind & kind): kind_(kind){;}
    constexpr HalError(Kind && kind): kind_(kind){;}

    constexpr HalError & operator = (const HalError & other) = default;
    constexpr HalError & operator = (HalError && other) = default;

    constexpr bool operator ==(const HalError other) const {return (kind_ == other.kind_);}
    constexpr bool operator !=(const HalError other) const {return (kind_ != other.kind_);}
    constexpr bool operator ==(const Kind kind) const {return (kind_ == kind);}
    constexpr bool operator !=(const Kind kind) const {return (kind_ != kind);}

    constexpr Kind kind() const {return kind_;}
private:
    Kind kind_;
};


class HalResult{
private:
    constexpr HalResult(std::nullopt_t):err_(std::nullopt){;}
public:
    using Kind = HalError::Kind;
    using enum Kind;

    static consteval HalResult Ok(){
        return {std::nullopt};
    }
    constexpr HalResult(const Kind & kind):
        err_(HalError(kind)){;}
    constexpr HalResult(Kind && kind):
        err_(HalError(kind)){;}
    constexpr HalResult(const HalError & err):
        err_(err){;}
    constexpr HalResult(HalError && err):
        err_(err){;}
    constexpr HalResult(const HalResult & other):
        err_(other.err_){;}
    constexpr HalResult(HalResult && other):
        err_(other.err_){;}

    constexpr bool operator ==(const HalResult & other) const {
        return err_ == other.err_;}
    constexpr bool operator !=(const HalResult & other) const {
        return err_ != other.err_;}


    __fast_inline constexpr bool is_err() const {
        return not is_ok();
    }


    __fast_inline constexpr bool is_ok() const {
        if(err_.has_value()) [[unlikely]] 
            return false;
        else [[likely]]
            return true;
    }

    // 链式处理
    template<typename Fn>
    HalResult then(Fn && fn){
        if (is_ok()) return std::forward<Fn>(fn)();
        return *this;
    }

    HalResult operator | (const HalResult rhs) const{
        if(is_err()) return *this;
        else return rhs;
    }

    constexpr HalError unwrap_err() const {return err_.value();}

private:
    std::optional<HalError> err_;
};
}


namespace ymd{
    class OutputStream;
    
    OutputStream & operator << (OutputStream & os, const hal::HalResult & err);

    OutputStream & operator << (OutputStream & os, const hal::HalError & err);

    OutputStream & operator << (OutputStream & os, const hal::HalError::Kind & err_kind);
}