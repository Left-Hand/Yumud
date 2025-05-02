#include "Entry.hpp"

using namespace ymd::canopen;

// size_t EntryDataType::dsize() const{

//     using enum Enum;
//     switch(e_){
//         default: return 0;
//         case bit:return 1;
//         case int8: return 1;
//         case int16: return 2;
//         case int32: return 4;
//         case uint8: return 1;
//         case uint16: return 2;
//         case uint32: return 4;
//         case real32: return 4;
//     }
// }


SdoAbortCode SubEntry::read(std::span<uint8_t> pdata) const{
    if(unlikely(!is_readable())) return SdoAbortCode::ReadOnlyAccess;
    if(unlikely(pdata.size() != dsize())) return SdoAbortCode::GeneralError;
    if(unlikely(pdata.size() > 4)) return SdoAbortCode::GeneralError;
    memcpy(pdata.data(), obj_.data(), pdata.size());
    return SdoAbortCode::None;
}

SdoAbortCode SubEntry::write(const std::span<const uint8_t> pdata){
    if(unlikely(!is_writeable())) return SdoAbortCode::WriteOnlyAccess;
    if(unlikely(pdata.size() != dsize())) return SdoAbortCode::GeneralError;
    if(unlikely(pdata.size() > 4)) return SdoAbortCode::GeneralError;
    memcpy(obj_.data(), pdata.data(), pdata.size());
    return SdoAbortCode::None;
}


SdoAbortCode SubEntry::read_any(void * pdata){
    memcpy(pdata, obj_.data(), dsize());
    return SdoAbortCode::None;
}

SdoAbortCode SubEntry::write_any(const void * pdata){
    memcpy(obj_.data(), pdata, dsize());
    return SdoAbortCode::None;
}

SubEntry::operator int() const {
    return obj_.read<int>();
}
