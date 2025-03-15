#include "ObjectDict.hpp"
#include "sys/string/string.hpp"


using namespace ymd::canopen;
using namespace ymd;

SdoAbortCode StaticObjectDictBase::write(const std::span<const std::byte> pdata, const Didx didx){
    auto se_opt = find(didx);

    if(unlikely(!se_opt.has_value())) return SdoAbortCode::ServiceParameterIncorrect;

    auto & se = se_opt.value();

    return se.write(pdata);
}

SdoAbortCode StaticObjectDictBase::read(const std::span<std::byte> pdata, const Didx didx) const{
    auto se_opt = const_cast<StaticObjectDictBase *>(this)->find(didx);
    
    if(unlikely(!se_opt.has_value())) return SdoAbortCode::ServiceParameterIncorrect;
    
    auto & se = (se_opt.value());
    return se.read(pdata);
}

SdoAbortCode StaticObjectDictBase::_write_any(const void * pdata, const Didx didx){
    auto se_opt = find(didx);

    if(unlikely(!se_opt.has_value())) return SdoAbortCode::ServiceParameterIncorrect;

    auto & se = se_opt.value();

    return se.write_any(pdata);

}
SdoAbortCode StaticObjectDictBase::_read_any(void * pdata, const Didx didx) const{
    auto se_opt = const_cast<StaticObjectDictBase *>(this)->find(didx);
    
    if(unlikely(!se_opt.has_value())) return SdoAbortCode::ServiceParameterIncorrect;
    
    auto & se = (se_opt.value());
    return se.read_any(pdata);
}

StringView StaticObjectDictBase::ename(const Didx didx) const{
    auto se_opt = const_cast<StaticObjectDictBase *>(this)->find(didx);

    if(!se_opt.has_value()) return std::nullopt;

    auto & se = (se_opt.value());

    return se.name();
}

