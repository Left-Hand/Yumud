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


EntryAccessError SubEntry::read(std::span<uint8_t> pdata) const{
    if(unlikely(!is_readable())) return EntryAccessError::ReadOnlyAccess;
    if(unlikely(pdata.size() != dsize())) return EntryAccessError::InvalidLength;
    if(unlikely(pdata.size() > 4)) return EntryAccessError::InvalidLength;
    memcpy(pdata.data(), obj_.data(), pdata.size());
    return EntryAccessError::None;
}

EntryAccessError SubEntry::write(const std::span<const uint8_t> pdata){
    if(unlikely(!is_writeable())) return EntryAccessError::WriteOnlyAccess;
    if(unlikely(pdata.size() != dsize())) return EntryAccessError::InvalidLength;
    if(unlikely(pdata.size() > 4)) return EntryAccessError::InvalidLength;
    memcpy(obj_.data(), pdata.data(), pdata.size());
    return EntryAccessError::None;
}


EntryAccessError SubEntry::read_any(void * pdata){
    memcpy(pdata, obj_.data(), dsize());
    return EntryAccessError::None;
}

EntryAccessError SubEntry::write_any(const void * pdata){
    memcpy(obj_.data(), pdata, dsize());
    return EntryAccessError::None;
}

SubEntry::operator int() const {
    return obj_.read<int>();
}
