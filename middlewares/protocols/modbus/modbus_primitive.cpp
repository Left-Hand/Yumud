#include "modbus_primitive.hpp"

using namespace ymd;

namespace ymd::modbus{

static constexpr const char * liberr_to_str(const LibError err){
    switch(err){
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
    return nullptr;
}

static constexpr const char * modbus_exception_to_str(const ExceptionCode err){
    switch (err) {
        case ExceptionCode::IllegalFunction:
            return "IllegalFunction";
        case ExceptionCode::IllegalDataAddress:
            return "IllegalDataAddress";
        case ExceptionCode::IllegalDataValue:
            return "IllegalDataValue";
        case ExceptionCode::ServerDeviceFailure:
            return "ServerDeviceFailure";
        case ExceptionCode::Acknowledge:
            return "Acknowledge";
        case ExceptionCode::ServerDeviceBusy:
            return "ServerDeviceBusy";
        case ExceptionCode::MemoryParityError:
            return "MemoryParityError";
        case ExceptionCode::GatewayPathUnavailable:
            return "GatewayPathUnavailable";
        case ExceptionCode::GatewayTargetDevice:
            return "GatewayTargetDevice";
    }
    return nullptr;
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

OutputStream & operator<<(OutputStream & os, const ExceptionCode & err){
    if(const auto str = modbus_exception_to_str(err); str != nullptr) [[likely]]
        return os << str;
    return print_unkown_u8(os, static_cast<uint8_t>(err));
}

OutputStream & operator<<(OutputStream & os, const ModbusError & err){
    if(err.is_exception()) return os << err.unwrap_as_exception();
    return os << err.unwrap_as<LibError>();
}


OutputStream & operator<<(OutputStream & os, const FunctionCode::Kind kind){
    using Kind = FunctionCode::Kind;
    DeriveDebugDispatcher<Kind>::call(os, kind);
    return os << os.field("Unknown")(static_cast<uint8_t>(kind));
}


}
