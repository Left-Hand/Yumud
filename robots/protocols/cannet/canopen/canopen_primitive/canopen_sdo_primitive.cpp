#include "canopen_sdo_primitive.hpp"

namespace ymd::canopen::primitive{
//不要在外部使用这个函数 因为它有可能返回空指针
static constexpr const char * err_to_str(const SdoAbortError err){
    using Kind = SdoAbortError;
    switch(err){
        case Kind::ToggleBitNotAlternated:
            return "ToggleBitNotAlternated";
        case Kind::SdoProtocolTimedOut:
            return "SdoProtocolTimedOut";
        case Kind::InvalidClientServerCommandSpecifier:
            return "InvalidClientServerCommandSpecifier";
        case Kind::InvalidBlockSize:
            return "InvalidBlockSize";
        case Kind::InvalidSequenceNumber:
            return "InvalidSequenceNumber";
        case Kind::CrcError:
            return "CrcError";
        case Kind::OutOfMemory:
            return "OutOfMemory";
        case Kind::UnsupportedAccessToObject:
            return "UnsupportedAccessToObject";
        case Kind::AttemptToReadWriteOnlyObject:
            return "AttemptToReadWriteOnlyObject";
        case Kind::AttemptToWriteReadOnlyObject:
            return "AttemptToWriteReadOnlyObject";
        case Kind::ObjectNotInDictionary:
            return "ObjectNotInDictionary";
        case Kind::ObjectCannotBeMappedToPdo:
            return "ObjectCannotBeMappedToPdo";
        case Kind::ExceedPdoLength:
            return "ExceedPdoLength";
        case Kind::GeneralParameterIncompatibility:
            return "GeneralParameterIncompatibility";
        case Kind::GeneralInternalIncompatibility:
            return "GeneralInternalIncompatibility";
        case Kind::HardwareError:
            return "HardwareError";
        case Kind::DataTypeMismatchLengthMismatch:
            return "DataTypeMismatchLengthMismatch";
        case Kind::DataTypeMismatchLengthTooHigh:
            return "DataTypeMismatchLengthTooHigh";
        case Kind::DataTypeMismatchLengthTooLow:
            return "DataTypeMismatchLengthTooLow";
        case Kind::SubIndexDoesNotExist:
            return "SubIndexDoesNotExist";
        case Kind::InvalidValueForParameter:
            return "InvalidValueForParameter";
        case Kind::ValueTooHigh:
            return "ValueTooHigh";
        case Kind::ValueTooLow:
            return "ValueTooLow";
        case Kind::MaxLessThanMin:
            return "MaxLessThanMin";
        case Kind::ResourceNotAvailable:
            return "ResourceNotAvailable";
        case Kind::GeneralError:
            return "GeneralError";
        case Kind::DataTransferOrStorageFailed:
            return "DataTransferOrStorageFailed";
        case Kind::LocalControlPreventsDataTransfer:
            return "LocalControlPreventsDataTransfer";
        case Kind::DeviceStatePreventsDataTransfer:
            return "DeviceStatePreventsDataTransfer";
        case Kind::ObjectDictionaryGenerationFailed:
            return "ObjectDictionaryGenerationFailed";
        case Kind::NoDataAvailable:
            return "NoDataAvailable";
        default:
            return nullptr;
    }
}
OutputStream & operator<<(OutputStream & os, const SdoAbortError err){
    if(const auto str = err_to_str(err); str != nullptr)
        return os << str;

    {
        const auto err_bits = static_cast<uint32_t>(err);
        const auto gaurd = os.create_guard();
        return os << std::hex << std::showbase
            << os.field("Unknown")(err_bits);
    }
}
}