#include "ObjectDict.hpp"
#include "sys/string/string.hpp"


using namespace ymd::canopen;
using namespace ymd;

EntryAccessError StaticObjectDictBase::write(const std::span<const uint8_t> pdata, const std::pair<const Index, const SubIndex> didx){
    auto se_opt = find(didx);

    if(unlikely(!se_opt.has_value())) return EntryAccessError::InvalidIndex;

    auto & se = se_opt.value();

    return se.write(pdata);

    return EntryAccessError::None;
}

EntryAccessError StaticObjectDictBase::read(const std::span<uint8_t> pdata, const std::pair<const Index, const SubIndex> didx) const{
    auto se_opt = const_cast<StaticObjectDictBase *>(this)->find(didx);
    
    if(unlikely(!se_opt.has_value())) return EntryAccessError::InvalidIndex;
    
    auto & se = (se_opt.value());
    return se.read(pdata);
    return EntryAccessError::None;
}

StringView StaticObjectDictBase::ename(const std::pair<const Index, const SubIndex> didx) const{
    auto se_opt = const_cast<StaticObjectDictBase *>(this)->find(didx);

    if(!se_opt.has_value()) return std::nullopt;

    auto & se = (se_opt.value());

    return se.name();
}

