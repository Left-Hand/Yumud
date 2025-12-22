#pragma once

#include "primitive/can/can_id.hpp"
#include "primitive/can/bxcan_frame.hpp"


namespace ymd::canopen{

enum class [[nodiscard]] FunctionCodeKind:uint8_t{
    Nmt = 0x00,
    Sync = 0x01,
    Emergency = 0x01,
    TimeStamp = 0x02,
    TxPdo1 = 0x180 >> 7,
    RxPdo1 = 0x200 >> 7,
    TxPdo2 = 0x280 >> 7,
    RxPdo2 = 0x300 >> 7,
    TxPdo3 = 0x380 >> 7,
    RxPdo3 = 0x400 >> 7,
    TxPdo4 = 0x480 >> 7,
    RxPdo4 = 0x500 >> 7,
    RespSdo = 0x580 >> 7,
    ReqSdo = 0x600 >> 7,
    Heartbeat = 0x0e,
};

enum class [[nodiscard]] PdoOnlyFunctionCodeKind:uint8_t{
    TxPdo1 = 3,
    RxPdo1,
    TxPdo2,
    RxPdo2,
    TxPdo3,
    RxPdo3,
    TxPdo4,
    RxPdo4,
};

struct [[nodiscard]] PdoOnlyFunctionCode final{
    using Self = PdoOnlyFunctionCode;
    using Kind = PdoOnlyFunctionCodeKind;

    [[nodiscard]] constexpr 
    PdoOnlyFunctionCode(const PdoOnlyFunctionCodeKind kind):
        bits_(std::bit_cast<uint8_t>(kind)){
    }

    [[nodiscard]] static constexpr 
    PdoOnlyFunctionCode from_bits_unchecked(const uint8_t bits){
        #ifndef NDEBUG
        if(bits > static_cast<uint8_t>(FunctionCodeKind::RxPdo4))
            __builtin_trap();
        if(bits < static_cast<uint8_t>(FunctionCodeKind::TxPdo1))
            __builtin_trap();
        #endif
        return std::bit_cast<PdoOnlyFunctionCode>(bits);
    }

    [[nodiscard]] static constexpr 
    Option<PdoOnlyFunctionCode> try_from_fc_kind(const FunctionCodeKind fc){
        return try_from_bits(std::bit_cast<uint8_t>(fc));
    }

    [[nodiscard]] static constexpr 
    Option<PdoOnlyFunctionCode> try_from_bits(const uint8_t bits){
        if(bits > static_cast<uint8_t>(FunctionCodeKind::RxPdo4))
            return None;
        if(bits < static_cast<uint8_t>(FunctionCodeKind::TxPdo1))
            return None;
        return Some(PdoOnlyFunctionCode(std::bit_cast<PdoOnlyFunctionCodeKind>(bits)));
    }

    [[nodiscard]] constexpr 
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
        }
        return None;
    }

    [[nodiscard]] constexpr 
    Option<Self> conj() const {
        switch(bits_){
            case static_cast<uint8_t>(FunctionCodeKind::TxPdo1):
                return Some<Self>(Self(PdoOnlyFunctionCodeKind::RxPdo1));
            case static_cast<uint8_t>(FunctionCodeKind::RxPdo1):
                return Some<Self>(Self(PdoOnlyFunctionCodeKind::TxPdo1));
            case static_cast<uint8_t>(FunctionCodeKind::TxPdo2):
                return Some<Self>(Self(PdoOnlyFunctionCodeKind::RxPdo2));
            case static_cast<uint8_t>(FunctionCodeKind::RxPdo2):
                return Some<Self>(Self(PdoOnlyFunctionCodeKind::TxPdo2));
            case static_cast<uint8_t>(FunctionCodeKind::TxPdo3):
                return Some<Self>(Self(PdoOnlyFunctionCodeKind::RxPdo3));
            case static_cast<uint8_t>(FunctionCodeKind::RxPdo3):
                return Some<Self>(Self(PdoOnlyFunctionCodeKind::TxPdo3));
            case static_cast<uint8_t>(FunctionCodeKind::TxPdo4):
                return Some<Self>(Self(PdoOnlyFunctionCodeKind::RxPdo4));
            case static_cast<uint8_t>(FunctionCodeKind::RxPdo4):
                return Some<Self>(Self(PdoOnlyFunctionCodeKind::TxPdo4));
        }
        return None;
    }

    using enum Kind;
private:
    uint8_t bits_;
};



struct [[nodiscard]] FunctionCode final{

    using Kind = FunctionCodeKind;
    constexpr FunctionCode(const Kind kind) : kind_(kind){;}
    static constexpr FunctionCode from_bits(const uint8_t bits){
        return FunctionCode(static_cast<Kind>(bits));
    }

    [[nodiscard]] constexpr uint8_t to_u4() const {
        return static_cast<uint8_t>(static_cast<uint8_t>(kind_) & 0x0f);
    }

    [[nodiscard]] constexpr uint8_t to_bits() const {
        return static_cast<uint8_t>(kind_);
    }

    [[nodiscard]] constexpr bool operator ==(const FunctionCode &other) const {
        return kind_ == other.kind_;
    }

    [[nodiscard]] constexpr bool operator ==(const FunctionCodeKind &kind) const {
        return kind_ == kind;
    }
    [[nodiscard]] constexpr bool is_nmt() const {return kind_ == Kind::Nmt;}
    [[nodiscard]] constexpr bool is_sync() const {return kind_ == Kind::Sync;}
    [[nodiscard]] constexpr bool is_energency() const {return kind_ == Kind::Emergency;}
    [[nodiscard]] constexpr bool is_heartbeat() const {return kind_ == Kind::Heartbeat;}
    [[nodiscard]] constexpr bool is_resp_sdo() const { return (kind_ == Kind::RespSdo);}
    [[nodiscard]] constexpr bool is_req_sdo() const { return (kind_ == Kind::ReqSdo);}
    [[nodiscard]] constexpr bool is_tx_pdo() const {
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
    [[nodiscard]] constexpr bool is_rx_pdo() const {
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

    [[nodiscard]] constexpr 
    Option<uint8_t> pdo_rank() const {
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

    [[nodiscard]] constexpr 
    Option<PdoOnlyFunctionCode> try_to_pdo_fc() const {
        return PdoOnlyFunctionCode::try_from_fc_kind(kind_);
    }

    using enum Kind;
private:
    Kind kind_;
};

}