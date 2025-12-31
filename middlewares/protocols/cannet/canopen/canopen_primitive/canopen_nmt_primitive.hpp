#pragma once


#include "canopen_primitive_base.hpp"
#include "core/tmp/bits/width.hpp"
#include "core/tmp/implfor.hpp"
#include "core/utils/Option.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/convert.hpp"



namespace ymd::canopen::primitive{

enum class [[nodiscard]] NodeState:uint8_t{
    BootUp = 0x00,
    Stopped = 0x04,
    PreOperational = 0x05,
    Operating = 0x07,
};

// https://docs.rs/canopeners/latest/canopeners/enum.NmtFunction.html
enum class [[nodiscard]] NmtCommand:uint8_t{
    StartRemoteNode = 0x01,
    StopRemoteNode = 0x02,
    EnterPreOperational = 0x80,
    ResetNode = 0x81,
    ResetCommunication = 0x82
};


// 同步报文是不带数据的，但是紧急报文是带8个字节的数据的，8个字节拉满

// 紧急报文的8个字节数据被分为了3个部分 
// 应急错误代码[0-1]
// 错误寄存器[2]
// 自定义错误区域[3-7]

enum class [[nodiscard]] EmcyError:uint16_t{
    Generic = 0x1000,
    Current = 0x2000,
    InputCurrent = 0x2100,
    InternalCurrent = 0x2200,
    OutputCurrent = 0x2300,
    Voltage = 0x3000,
    BusbarVoltage = 0x3100,
    InternalVoltage = 0x3200,
    OutputVoltage = 0x3300,
    Temperature = 0x4000,
    SituationTemperature = 0x4100,
    InternalTemperature = 0x4200,
    Hardware = 0x5000,

    Software = 0x6000,
    InternalSoftware = 0x6100,
    UserSoftware = 0x6200,
    SetterSoftware = 0x6300,
    AdjuntDevice = 0x7000,
    Monitor = 0x8000,
    Communication = 0x8100,
    CommunicationOverload = 0x8110,
    Passive = 0x8120,
    NodeProtect = 0x8130,
    BusRecovery = 0x8140,
    Protocol = 0x8200,
    UnhandledPdo = 0x8210,
    OutOfRange = 0x8220,
    External = 0x9000,
    AddictiveFunction = 0xf000,
    Specified = 0xf100,
};

struct [[nodiscard]] EmcyErrorCode{
    using Kind = EmcyError; 
    using Self = EmcyErrorCode;
    static constexpr EmcyError OK = static_cast<EmcyError>(0x0000);
    constexpr EmcyErrorCode(const EmcyError kind) : kind_(kind) {}

    static constexpr Option<Self> try_from_bits(const uint16_t bits){
        if(bits == 0) return Some(Self(OK));
        if(const auto * str = err_to_str(static_cast<EmcyError>(bits)); str != nullptr)
            return Some(Self(static_cast<EmcyError>(bits)));
        return None;
    }
    constexpr Kind kind() const { return kind_; }
    [[nodiscard]] constexpr bool operator ==(const Kind kind) const { return kind_ == kind; }
    [[nodiscard]] constexpr bool is_ok() const {
        return kind_ == OK;
    }

    [[nodiscard]] constexpr bool is_err() const {
        return !is_ok();
    }

    constexpr void fill_bytes(std::span<uint8_t, 2> pbuf) const {
        pbuf[0] = static_cast<uint8_t>(std::bit_cast<uint16_t>(kind_) & 0xff);
        pbuf[1] = static_cast<uint8_t>(std::bit_cast<uint16_t>(kind_) >> 8);
    }

    [[nodiscard]] constexpr uint16_t to_bits() const {
        return std::bit_cast<uint16_t>(kind_);
    }

    [[nodiscard]] static constexpr const char * err_to_str(const EmcyError err){
        using enum EmcyError;
        switch(err){
            default: return nullptr;
            case Generic: return "Generic";
            case Current: return "Current";
            case InputCurrent: return "InputCurrent";
            case InternalCurrent: return "InternalCurrent";
            case OutputCurrent: return "OutputCurrent";
            case Voltage: return "Voltage";
            case BusbarVoltage: return "BusbarVoltage";
            case InternalVoltage: return "InternalVoltage";
            case OutputVoltage: return "OutputVoltage";
            case Temperature: return "Temperature";
            case SituationTemperature: return "SituationTemperature";
            case InternalTemperature: return "InternalTemperature";
            case Hardware: return "Hardware";

            case Software: return "Software";
            case InternalSoftware: return "InternalSoftware";
            case UserSoftware: return "UserSoftware";
            case SetterSoftware: return "SetterSoftware";
            case AdjuntDevice: return "AdjuntDevice";
            case Monitor: return "Monitor";
            case Communication: return "Communication";
            case CommunicationOverload: return "CommunicationOverload";
            case Passive: return "Passive";
            case NodeProtect: return "NodeProtect";
            case BusRecovery: return "BusRecovery";
            case Protocol: return "Protocol";
            case UnhandledPdo: return "UnhandledPdo";
            case OutOfRange: return "OutOfRange";
            case External: return "External";
            case AddictiveFunction: return "AddictiveFunction";
            case Specified: return "Specified";
        }
    }

    using enum Kind;
private:
    Kind kind_;


    inline friend OutputStream & operator <<(OutputStream & os, const EmcyError & err){ 
        if(const auto * str = err_to_str(err); str != nullptr)
            return os << str;
        const auto guard = os.create_guard();
        return os << "Unknown" << os.brackets<'('>() 
            << std::hex << std::showbase 
            << static_cast<uint16_t>(err) << os.brackets<')'>();
    }

    inline friend OutputStream & operator <<(OutputStream & os, const EmcyErrorCode & self){ 
        if(self.is_ok()) 
            return os << "Ok";
        return os << self.kind();
    }
};

static_assert(sizeof(EmcyErrorCode) == 2);

}

namespace ymd{

template<>
struct ImplFor<convert::TryFrom<uint8_t>, canopen::primitive::NodeState>{
    using Self = canopen::primitive::NodeState;
    using Error = void;
    static constexpr Result<Self, Error> try_from(uint8_t int_val){
        switch(int_val){
            case static_cast<uint8_t>(Self::BootUp): 
            case static_cast<uint8_t>(Self::Stopped): 
            case static_cast<uint8_t>(Self::PreOperational): 
            case static_cast<uint8_t>(Self::Operating): 
                return Ok(static_cast<Self>(int_val));
        }
        return Err();
    }
};

template<>
struct ImplFor<convert::TryFrom<uint8_t>, canopen::primitive::NmtCommand>{
    using Self = canopen::primitive::NmtCommand;
    using Error = void;
    static constexpr Result<Self, Error> try_from(uint8_t int_val){
        switch(static_cast<Self>(int_val)){
            case Self::StartRemoteNode:   
            case Self::StopRemoteNode:    
            case Self::EnterPreOperational:   
            case Self::ResetNode: 
            case Self::ResetCommunication:    
                return Ok(static_cast<Self>(int_val));
        }
        return Err();
    }
};

}

namespace ymd{


// EmcyError
template<>
struct ImplFor<convert::TryFrom<uint16_t>, canopen::primitive::EmcyError>{
    using Self = canopen::primitive::EmcyError;
    using Error = void;
    static constexpr Result<Self, Error> try_from(uint16_t int_val){
        if(canopen::primitive::EmcyErrorCode::err_to_str(static_cast<Self>(int_val)) != nullptr)
            return Ok(static_cast<Self>(int_val));
        else
            return Err();
    }
};

template<>
struct ImplFor<convert::TryFrom<uint16_t>, canopen::primitive::EmcyErrorCode>{
    using Self = canopen::primitive::EmcyErrorCode;
    using Error = void;
    static constexpr Result<Self, Error> try_from(uint16_t int_val){
        const auto may_code = Self::try_from_bits(int_val);
        if(may_code.is_some())
            return Ok(may_code.unwrap());
        else
            return Err();
    }
};

}