#pragma once

#include "primitive/can/can_id.hpp"
#include "primitive/can/can_msg.hpp"


namespace ymd::canopen{

enum class FunctionCodeKind:uint8_t{
    Nmt = 0x00,
    Sync = 0x01,
    Emergency = 0x02,
    Time = 0x03,
    TxPdo1 = 0x04,
    RxPdo1 = 0x05,
    TxPdo2 = 0x06,
    RxPdo2 = 0x07,
    TxPdo3 = 0x08,
    RxPdo3 = 0x09,
    TxPdo4 = 0x0a,
    RxPdo4 = 0x0b,
    TxSdo = 0x0c,
    RxSdo = 0x0d,
    Heartbeat = 0x0e,
};

enum class PdoFunctionCodeKind:uint8_t{
    TxPdo1 = 0x04,
    RxPdo1 = 0x05,
    TxPdo2 = 0x06,
    RxPdo2 = 0x07,
    TxPdo3 = 0x08,
    RxPdo3 = 0x09,
    TxPdo4 = 0x0a,
    RxPdo4 = 0x0b,
};

struct [[nodiscard]] PdoFunctionCode{
    using Self = PdoFunctionCode;
    using Kind = PdoFunctionCodeKind;

    [[nodiscard]] __always_inline constexpr 
    PdoFunctionCode(const PdoFunctionCodeKind kind):
        bits_(std::bit_cast<uint8_t>(kind)){
    }

    [[nodiscard]] __always_inline static constexpr 
    PdoFunctionCode from_bits_unchecked(const uint8_t bits){
        #ifndef NDEBUG
        if(bits > static_cast<uint8_t>(FunctionCodeKind::RxPdo4))
            __builtin_trap();
        if(bits < static_cast<uint8_t>(FunctionCodeKind::TxPdo1))
            __builtin_trap();
        #endif
        return std::bit_cast<PdoFunctionCode>(bits);
    }

    [[nodiscard]] __always_inline static constexpr 
    Option<PdoFunctionCode> from_fc_kind(const FunctionCodeKind fc){
        return from_bits(std::bit_cast<uint8_t>(fc));
    }

    [[nodiscard]] __always_inline static constexpr 
    Option<PdoFunctionCode> from_bits(const uint8_t bits){
        #ifndef NDEBUG
        if(bits > static_cast<uint8_t>(FunctionCodeKind::RxPdo4))
            return None;
        if(bits < static_cast<uint8_t>(FunctionCodeKind::TxPdo1))
            return None;
        #endif
        return Some(PdoFunctionCode(std::bit_cast<PdoFunctionCodeKind>(bits)));
    }

    [[nodiscard]] __always_inline constexpr 
    Option<uint8_t> rank() const {
        switch(bits_){
            case static_cast<uint8_t>(FunctionCodeKind::TxPdo1):
            case static_cast<uint8_t>(FunctionCodeKind::RxPdo1):
                return Some<uint8_t>(1);
            case static_cast<uint8_t>(FunctionCodeKind::TxPdo2):
            case static_cast<uint8_t>(FunctionCodeKind::RxPdo2):
                return Some<uint8_t>(2);
            case static_cast<uint8_t>(FunctionCodeKind::TxPdo3):
            case static_cast<uint8_t>(FunctionCodeKind::RxPdo3):
                return Some<uint8_t>(3);
            case static_cast<uint8_t>(FunctionCodeKind::TxPdo4):
            case static_cast<uint8_t>(FunctionCodeKind::RxPdo4):
                return Some<uint8_t>(4);
            default:
                __builtin_trap();
        }
    }

    [[nodiscard]] __always_inline constexpr 
    Option<Self> conj() const {
        switch(bits_){
            case static_cast<uint8_t>(FunctionCodeKind::TxPdo1):
                return Some<Self>(Self(PdoFunctionCodeKind::RxPdo1));
            case static_cast<uint8_t>(FunctionCodeKind::RxPdo1):
                return Some<Self>(Self(PdoFunctionCodeKind::TxPdo1));
            case static_cast<uint8_t>(FunctionCodeKind::TxPdo2):
                return Some<Self>(Self(PdoFunctionCodeKind::RxPdo2));
            case static_cast<uint8_t>(FunctionCodeKind::RxPdo2):
                return Some<Self>(Self(PdoFunctionCodeKind::TxPdo2));
            case static_cast<uint8_t>(FunctionCodeKind::TxPdo3):
                return Some<Self>(Self(PdoFunctionCodeKind::RxPdo3));
            case static_cast<uint8_t>(FunctionCodeKind::RxPdo3):
                return Some<Self>(Self(PdoFunctionCodeKind::TxPdo3));
            case static_cast<uint8_t>(FunctionCodeKind::TxPdo4):
                return Some<Self>(Self(PdoFunctionCodeKind::RxPdo4));
            case static_cast<uint8_t>(FunctionCodeKind::RxPdo4):
                return Some<Self>(Self(PdoFunctionCodeKind::TxPdo4));
            default:
                __builtin_trap();
        }
    }

    using enum Kind;
private:
    uint8_t bits_;
};



struct [[nodiscard]] FunctionCode{

    using Kind = FunctionCodeKind;
    constexpr FunctionCode(const Kind kind) : kind_(kind){;}
    static constexpr FunctionCode from_bits(const uint8_t bits){
        return FunctionCode(static_cast<Kind>(bits));
    }

    [[nodiscard]] constexpr uint8_t as_u4() const {
        return static_cast<uint8_t>(kind_);
    }

    [[nodiscard]] constexpr uint8_t as_bits() const {
        return static_cast<uint8_t>(kind_);
    }

    [[nodiscard]] __always_inline constexpr bool is_nmt() const {
        return kind_ == Kind::Nmt;}
    [[nodiscard]] __always_inline constexpr bool is_sync() const {
        return kind_ == Kind::Sync;}
    [[nodiscard]] __always_inline constexpr bool is_energency() const {
        return kind_ == Kind::Emergency;}
    [[nodiscard]] __always_inline constexpr bool is_time() const {
        return kind_ == Kind::Time;}
    [[nodiscard]] __always_inline constexpr bool is_heartbeat() const {
        return kind_ == Kind::Heartbeat;}
    [[nodiscard]] __always_inline constexpr bool is_sdo() const {
        return (kind_ == Kind::TxSdo) or (kind_ == Kind::RxSdo);}
    [[nodiscard]] __always_inline constexpr bool is_tx_pdo() const {
        switch(kind_){
            case Kind::TxPdo1:
            case Kind::TxPdo2:
            case Kind::TxPdo3:
            case Kind::TxPdo4:
                return true;
            default:
                return false;
        }
    }

    [[nodiscard]] __always_inline constexpr bool is_rx_pdo() const {
        switch(kind_){
            case Kind::RxPdo1:
            case Kind::RxPdo2:
            case Kind::RxPdo3:
            case Kind::RxPdo4:
                return true;
            default:
                return false;
        }
    }

    [[nodiscard]] __always_inline constexpr Option<uint8_t> pdo_rank() const {
        switch(kind_){
            case Kind::RxPdo1:
                return Some<uint8_t>(1);
            case Kind::RxPdo2:
                return Some<uint8_t>(2);
            case Kind::RxPdo3:
                return Some<uint8_t>(3);
            case Kind::RxPdo4:
                return Some<uint8_t>(4);
            default:
                return None;
        }
    }

    [[nodiscard]] __always_inline constexpr Option<PdoFunctionCode> to_pdo_fc() const {
        return PdoFunctionCode::from_fc_kind(kind_);
    }

    using enum Kind;
private:
    Kind kind_;
};

}