#include "modbus_primitive.hpp"

using namespace ymd;

namespace ymd::modbus::primitive{

static constexpr const char * liberr_to_str(const LibError err){
    switch(err){
        default:
            return nullptr;
        case LibError::InvalidRequst:
            return "InvalidRequst";
        case LibError::InvalidUnitId:
            return "InvalidUnitId";
        case LibError::InvalidTcpMbap:
            return "InvalidTcpMbap";
        case LibError::InvalidCrc:
            return "InvalidCrc";
        case LibError::Transport:
            return "Transport";
        case LibError::Timeout:
            return "Timeout";
        case LibError::InvalidResponse:
            return "InvalidResponse";
        case LibError::InvalidArgument:
            return "InvalidArgument";
    }
}

static constexpr const char * modbus_exception_to_str(const ModbusException err){
    switch (err) {
        default:
            return nullptr;
        case ModbusException::IllegalFunction:
            return "IllegalFunction";
        case ModbusException::IllegalDataAddress:
            return "IllegalDataAddress";
        case ModbusException::IllegalDataValue:
            return "IllegalDataValue";
        case ModbusException::ServerDeviceFailure:
            return "ServerDeviceFailure";
    }
}

static constexpr const char * function_code_to_str(const FunctionCode fc){
    switch (fc) {
        default:
            return nullptr;
        case FunctionCode::None:
            return "None";
        case FunctionCode::ReadCoils:
            return "ReadCoils";
        case FunctionCode::ReadDiscreteInputs:
            return "ReadDiscreteInputs";
        case FunctionCode::ReadHoldingRegisters:
            return "ReadHoldingRegisters";
        case FunctionCode::ReadInputRegisters:
            return "ReadInputRegisters";
        case FunctionCode::WriteSingleCoil:
            return "WriteSingleCoil";
        case FunctionCode::WriteSingleRegister:
            return "WriteSingleRegister";
        case FunctionCode::WriteMultipleCoils:
            return "WriteMultipleCoils";
        case FunctionCode::WriteMultipleRegisters:
            return "WriteMultipleRegisters";
        case FunctionCode::ReadFileRecord:
            return "ReadFileRecord";
        case FunctionCode::WriteFileRecord:
            return "WriteFileRecord";
        case FunctionCode::ReadWriteRegisters:
            return "ReadWriteRegisters";
        case FunctionCode::ReadDeviceIdentification:
            return "ReadDeviceIdentification";
    }
}

__always_inline OutputStream & print_unkown_u8(OutputStream & os, uint8_t int_val){
    return os << "Unknown" << os.brackets<'('>() 
        << std::bit_cast<uint8_t>(int_val) << os.brackets<')'>();
}

OutputStream & operator<<(OutputStream & os, const LibError & err){
    if(const auto str = liberr_to_str(err); str != nullptr) [[likely]]
        return os << str;
    return print_unkown_u8(os, static_cast<uint8_t>(err));
}

OutputStream & operator<<(OutputStream & os, const ModbusException & err){
    if(const auto str = modbus_exception_to_str(err); str != nullptr) [[likely]]
        return os << str;
    return print_unkown_u8(os, static_cast<uint8_t>(err));
}

OutputStream & operator<<(OutputStream & os, const ModbusError & err){
    if(err.is_exception()) return os << err.unwrap_as_exception();
    return os << err.unwrap_as<LibError>();
}

OutputStream & operator<<(OutputStream & os, const FunctionCode & fc){
    if(const auto str = function_code_to_str(fc); str != nullptr) [[likely]]
        return os << str;
    return print_unkown_u8(os, static_cast<uint8_t>(fc));
}

}