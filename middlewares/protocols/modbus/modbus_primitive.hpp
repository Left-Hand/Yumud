#pragma once

#include "core/utils/Result.hpp"
#include "core/utils/sumtype.hpp"
#include "core/stream/ostream.hpp"
#include "core/container/bitarray.hpp"
#include "core/container/heapless_vector.hpp"

// https://blog.csdn.net/u011041241/article/details/109240555

namespace ymd::modbus{

namespace primitive{

enum class [[nodiscard]] LibError:uint8_t{
    InvalidRequst,
    InvalidUnitId,
    InvalidTcpMbap,
    InvalidCrc,
    Transport,
    Timeout,
    InvalidResponse,
    InvalidArgument,
};

enum class [[nodiscard]] ExceptionCode:uint8_t{
    IllegalFunction = 1,      /**< Modbus exception 1 */
    IllegalDataAddress = 2,  /**< Modbus exception 2 */
    IllegalDataValue = 3,    /**< Modbus exception 3 */
    ServerDeviceFailure = 4, /**< Modbus exception 4 */
    Acknowledge,
    ServerDeviceBusy,
    MemoryParityError,
    GatewayPathUnavailable,
    GatewayTargetDevice,
};

struct [[nodiscard]]FunctionCode final{

    enum class [[nodiscard]] Kind:uint8_t{
        ReadCoils = 1,
        ReadDiscreteInputs = 2,
        ReadHoldingRegisters = 3,
        ReadInputRegisters = 4,
        WriteSingleCoil = 5,
        WriteSingleRegister = 6,
        WriteMultipleCoils = 15,
        WriteMultipleRegisters = 16,
        ReadFileRecord = 20,
        WriteFileRecord = 21,
        ReadWriteMultipleRegisters = 23,
        ReadDeviceIdentification = 43
    };

    constexpr FunctionCode(const Kind kind): bits(static_cast<uint8_t>(kind)){;}
    constexpr FunctionCode(const _None_t): bits(0){;}

    uint8_t bits;

    constexpr bool is_none() const {return bits == 0u;}
    constexpr bool is_some() const {return bits != 0u;}

    constexpr Kind unwrap() const{
        if(is_none()) 
            __builtin_trap();
        return std::bit_cast<Kind>(bits);
    }

    friend OutputStream & operator<<(OutputStream & os, const Kind kind){
        switch(kind) {
            case Kind::ReadCoils:
                return os << "ReadCoils";
            case Kind::ReadDiscreteInputs:
                return os << "ReadDiscreteInputs";
            case Kind::ReadHoldingRegisters:
                return os << "ReadHoldingRegisters";
            case Kind::ReadInputRegisters:
                return os << "ReadInputRegisters";
            case Kind::WriteSingleCoil:
                return os << "WriteSingleCoil";
            case Kind::WriteSingleRegister:
                return os << "WriteSingleRegister";
            case Kind::WriteMultipleCoils:
                return os << "WriteMultipleCoils";
            case Kind::WriteMultipleRegisters:
                return os << "WriteMultipleRegisters";
            case Kind::ReadFileRecord:
                return os << "ReadFileRecord";
            case Kind::WriteFileRecord:
                return os << "WriteFileRecord";
            case Kind::ReadWriteMultipleRegisters:
                return os << "ReadWriteMultipleRegisters";
            case Kind::ReadDeviceIdentification:
                return os << "ReadDeviceIdentification";
        }
        return os << "UnknownExceptionCode(" << 
            std::hex << std::setfill('0') << std::setw(2) << static_cast<uint8_t>(self.bits) 
            << ")";
    }
    friend OutputStream & operator<<(OutputStream & os, const FunctionCode & self){
        if(self.is_none()) return os << "None";
        return os << self.unwrap();
    }
};



struct [[nodiscard]] ModbusError:public Sumtype<LibError,ExceptionCode>{
    using enum LibError;
    using enum ExceptionCode;
    [[nodiscard]] bool is_exception() const{
        return is<ExceptionCode>();
    }

    [[nodiscard]] ExceptionCode unwrap_as_exception() const{
        return unwrap_as<ExceptionCode>();
    }

    friend OutputStream & operator<<(OutputStream & os, const LibError & err);
    friend OutputStream & operator<<(OutputStream & os, const ExceptionCode & err);
    friend OutputStream & operator<<(OutputStream & os, const ModbusError & err);

};

static constexpr Err<ModbusError> make_err(auto err){
    return Err<ModbusError>(err);
}

struct [[nodiscard]] Address{
    using Self = Address;

    static constexpr Self from_bits(const uint8_t bits){
        return std::bit_cast<Self>(bits);
    } 

    [[nodiscard]] constexpr uint8_t to_bits() const {
        return bits_;
    }

    [[nodiscard]] constexpr bool is_boardcast() const{
        return bits_ == 0;
    }

    [[nodiscard]] constexpr bool is_preserved() const {
        return bits_ >= 248;
    }
private:
    uint8_t bits_;
};  


template<typename T = void>
using IResult = Result<T, ModbusError>;


enum class [[nodiscard]] Transport:uint8_t{
    RTU = 1,
    TCP = 2,
};


struct Address{
    uint16_t bits;
};

struct Quantity{
    uint16_t bits;
};

struct BoolInterator{

};

struct reqs{


struct ReadCoils{
    Address address, 
    Quantity
};
struct ReadDiscreteInputs{
    Address address, 
    Quantity
};
struct WriteSingleCoil{
    Address address, 
    bool
};
struct WriteMultipleCoils{
    Address address, 
    BoolInterator
};
struct ReadInputRegisters{
    Address address, 
    Quantity
};
struct ReadHoldingRegisters{
    Address address, 
    Quantity
};
struct WriteSingleRegister{
    Address address, 
    uint16_t
};
struct WriteMultipleRegisters{
    Address address, 
    std::
    span<uint16_t>};
struct ReportServerId{};
struct MaskWriteRegister{
    Address address, 
    uint16_t
    , uint16_t};
struct ReadWriteMultipleRegisters{
    Address address, 
    Quantity
    , Address, std::span<uint16_t>};
struct ReadDeviceIdentification{ReadCode, ObjectId};
// struct Custom{u8, Cow<'a, [u8]>};
}
}
}