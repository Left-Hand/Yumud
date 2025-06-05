#include "Entry.hpp"

using namespace ymd::canopen;

SdoAbortCode SubEntry::read(std::span<uint8_t> pbuf) const{
    if(unlikely(!is_readable())) return SdoAbortCode::ReadOnlyAccess;
    if(unlikely(pbuf.size() != dsize())) return SdoAbortCode::GeneralError;
    if(unlikely(pbuf.size() > 4)) return SdoAbortCode::GeneralError;
    memcpy(pbuf.data(), obj_ref_.data(), pbuf.size());
    return SdoAbortCode::None;
}

SdoAbortCode SubEntry::write(const std::span<const uint8_t> pbuf){
    if(unlikely(!is_writeable())) return SdoAbortCode::WriteOnlyAccess;
    if(unlikely(pbuf.size() != dsize())) return SdoAbortCode::GeneralError;
    if(unlikely(pbuf.size() > 4)) return SdoAbortCode::GeneralError;
    memcpy(obj_ref_.data(), pbuf.data(), pbuf.size());
    return SdoAbortCode::None;
}


SdoAbortCode SubEntry::read_any(void * pbuf){
    memcpy(pbuf, obj_ref_.data(), dsize());
    return SdoAbortCode::None;
}

SdoAbortCode SubEntry::write_any(const void * pbuf){
    memcpy(obj_ref_.data(), pbuf, dsize());
    return SdoAbortCode::None;
}

SubEntry::operator int() const {
    return obj_ref_.read<int>();
}
