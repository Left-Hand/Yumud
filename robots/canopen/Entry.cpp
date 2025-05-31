#include "Entry.hpp"

using namespace ymd::canopen;

SdoAbortCode SubEntry::read(std::span<uint8_t> pdata) const{
    if(unlikely(!is_readable())) return SdoAbortCode::ReadOnlyAccess;
    if(unlikely(pdata.size() != dsize())) return SdoAbortCode::GeneralError;
    if(unlikely(pdata.size() > 4)) return SdoAbortCode::GeneralError;
    memcpy(pdata.data(), obj_ref_.data(), pdata.size());
    return SdoAbortCode::None;
}

SdoAbortCode SubEntry::write(const std::span<const uint8_t> pdata){
    if(unlikely(!is_writeable())) return SdoAbortCode::WriteOnlyAccess;
    if(unlikely(pdata.size() != dsize())) return SdoAbortCode::GeneralError;
    if(unlikely(pdata.size() > 4)) return SdoAbortCode::GeneralError;
    memcpy(obj_ref_.data(), pdata.data(), pdata.size());
    return SdoAbortCode::None;
}


SdoAbortCode SubEntry::read_any(void * pdata){
    memcpy(pdata, obj_ref_.data(), dsize());
    return SdoAbortCode::None;
}

SdoAbortCode SubEntry::write_any(const void * pdata){
    memcpy(obj_ref_.data(), pdata, dsize());
    return SdoAbortCode::None;
}

SubEntry::operator int() const {
    return obj_ref_.read<int>();
}
