#include "canopen_sdo_primitive.hpp"

namespace ymd::canopen::primitive{
//不要在外部使用这个函数 因为它有可能返回空指针
static constexpr const char * err_to_str(const SdoAbortError err){
    switch(err){
        case SdoAbortError::ToggleBitNotAlternated      :
            return "ToggleBitNotAlternated";
        case SdoAbortError::SdoProtocolTimedOut         :
            return "SdoProtocolTimedOut";
        case SdoAbortError::CommandSpecifierNotValid    :
            return "CommandSpecifierNotValid";
        case SdoAbortError::InvalidBlockSize            :
            return "InvalidBlockSize";
        case SdoAbortError::InvalidSequenceNumber       :
            return "InvalidSequenceNumber";
        case SdoAbortError::CRCError                    :
            return "CRCError";
        case SdoAbortError::OutOfMemory                 :
            return "OutOfMemory";
        case SdoAbortError::UnsupportedAccess           :
            return "UnsupportedAccess";
        case SdoAbortError::ReadOnlyAccess              :
            return "ReadOnlyAccess";
        case SdoAbortError::WriteOnlyAccess             :
            return "WriteOnlyAccess";
        case SdoAbortError::ObjectDoesNotExist          :
            return "ObjectDoesNotExist";
        case SdoAbortError::ObjectCannotBeMapped        :
            return "ObjectCannotBeMapped";
        case SdoAbortError::PdoLengthExceeded           :
            return "PdoLengthExceeded";
        case SdoAbortError::ParameterIncompatibility    :
            return "ParameterIncompatibility";
        case SdoAbortError::InternalIncompatibility     :
            return "InternalIncompatibility";
        case SdoAbortError::HardwareError               :
            return "HardwareError";
        case SdoAbortError::ServiceParameterIncorrect   :
            return "ServiceParameterIncorrect";
        case SdoAbortError::ServiceParameterTooLong     :
            return "ServiceParameterTooLong";
        case SdoAbortError::ServiceParameterTooShort    :
            return "ServiceParameterTooShort";
        case SdoAbortError::SubIndexDoesNotExist        :
            return "SubIndexDoesNotExist";
        case SdoAbortError::InvalidValue                :
            return "InvalidValue";
        case SdoAbortError::ValueTooHigh                :
            return "ValueTooHigh";
        case SdoAbortError::ValueTooLow                 :
            return "ValueTooLow";
        case SdoAbortError::MaxLessThanMin              :
            return "MaxLessThanMin";
        case SdoAbortError::ResourceNotAvailable        :
            return "ResourceNotAvailable";
        case SdoAbortError::GeneralError                :
            return "GeneralError"; 
        case SdoAbortError::NoValidData                 :
            return "NoValidData";         // 无可用数据  
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