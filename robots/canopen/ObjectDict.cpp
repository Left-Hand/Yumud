#include "ObjectDict.hpp"
#include "core/string/string.hpp"


using namespace ymd::canopen;
using namespace ymd;

SdoAbortCode StaticObjectDictBase::write(
    const std::span<const uint8_t> pbuf, const Didx didx
){
    auto may_se = get_sub_entry(didx);

    if(unlikely(!may_se.is_some()))
        return SdoAbortCode::ServiceParameterIncorrect;

    auto & se = may_se.unwrap();

    return se.write(pbuf);
}

SdoAbortCode StaticObjectDictBase::read(
    const std::span<uint8_t> pbuf, const Didx didx
) const{
    auto may_se = const_cast<StaticObjectDictBase *>(this)->get_sub_entry(didx);
    
    if(unlikely(!may_se.is_some()))
        return SdoAbortCode::ServiceParameterIncorrect;
    
    auto & se = (may_se.unwrap());
    return se.read(pbuf);
}

SdoAbortCode StaticObjectDictBase::_write_any(
    const void * pbuf, const Didx didx
){
    auto may_se = get_sub_entry(didx);

    if(unlikely(!may_se.is_some()))
        return SdoAbortCode::ServiceParameterIncorrect;

    auto & se = may_se.unwrap();

    return se.write_any(pbuf);

}
SdoAbortCode StaticObjectDictBase::_read_any(
    void * pbuf, const Didx didx
) const{
    auto may_se = const_cast<StaticObjectDictBase *>(this)->get_sub_entry(didx);
    
    if(unlikely(!may_se.is_some()))
        return SdoAbortCode::ServiceParameterIncorrect;
    
    auto & se = (may_se.unwrap());
    return se.read_any(pbuf);
}

StringView StaticObjectDictBase::ename(const Didx didx) const{
    auto may_se = const_cast<StaticObjectDictBase *>(this)->get_sub_entry(didx);

    if(!may_se.is_some()) return nullptr;

    auto & se = (may_se.unwrap());

    return se.name();
}

