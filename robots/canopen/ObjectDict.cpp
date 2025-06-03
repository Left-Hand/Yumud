#include "ObjectDict.hpp"
#include "core/string/string.hpp"


using namespace ymd::canopen;
using namespace ymd;

SdoAbortCode StaticObjectDictBase::write(const std::span<const uint8_t> pbuf, const Didx didx){
    auto se_opt = find(didx);

    if(unlikely(!se_opt.has_value())) return SdoAbortCode::ServiceParameterIncorrect;

    auto & se = se_opt.value();

    return se.write(pbuf);
}

SdoAbortCode StaticObjectDictBase::read(const std::span<uint8_t> pbuf, const Didx didx) const{
    auto se_opt = const_cast<StaticObjectDictBase *>(this)->find(didx);
    
    if(unlikely(!se_opt.has_value())) return SdoAbortCode::ServiceParameterIncorrect;
    
    auto & se = (se_opt.value());
    return se.read(pbuf);
}

SdoAbortCode StaticObjectDictBase::_write_any(const void * pbuf, const Didx didx){
    auto se_opt = find(didx);

    if(unlikely(!se_opt.has_value())) return SdoAbortCode::ServiceParameterIncorrect;

    auto & se = se_opt.value();

    return se.write_any(pbuf);

}
SdoAbortCode StaticObjectDictBase::_read_any(void * pbuf, const Didx didx) const{
    auto se_opt = const_cast<StaticObjectDictBase *>(this)->find(didx);
    
    if(unlikely(!se_opt.has_value())) return SdoAbortCode::ServiceParameterIncorrect;
    
    auto & se = (se_opt.value());
    return se.read_any(pbuf);
}

StringView StaticObjectDictBase::ename(const Didx didx) const{
    auto se_opt = const_cast<StaticObjectDictBase *>(this)->find(didx);

    if(!se_opt.has_value()) return std::nullopt;

    auto & se = (se_opt.value());

    return se.name();
}

