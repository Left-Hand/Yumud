#pragma once

#include "Entry.hpp"
#include <map>
#include "core/utils/Option.hpp"
#include "utils.hpp"

namespace ymd::canopen{


class ObjectDictIntf{
public:
    using Index = OdIndex;
    using SubIndex = OdSubIndex;
protected:
    virtual SdoAbortCode _write_any(const void * pbuf, const Didx didx) = 0;
    virtual SdoAbortCode _read_any(void * pbuf, const Didx didx) const = 0;
public:

    // template<typename T>
    // requires ((sizeof(T) <= 4))
    // SdoAbortCode write(const auto data, const Didx didx){
    //     static_assert(std::is_convertible_v<T, decltype(data)>, "type mismatch");
    //     return write({reinterpret_cast<const uint8_t *>(&data), sizeof(T)}, {didx.idx, didx.subidx});
    // }

    // template<typename T, typename U>
    // requires (sizeof(T) <= 4)
    // SdoAbortCode read(U & data, const Didx didx) const {
    //     static_assert(std::is_same_v<T, U>, "type mismatch");
    //     return read({reinterpret_cast<uint8_t *>(&data), sizeof(T)}, {didx.idx, didx.subidx});
    // }


    
    // virtual Option<OdEntry> operator [](const Index index) = 0;

    // virtual Option<SubEntry> operator [](const Didx id) = 0;

    virtual SdoAbortCode write(const std::span<const uint8_t> pbuf, const Didx didx) = 0;
    virtual SdoAbortCode read(const std::span<uint8_t> pbuf, const Didx didx) const = 0;

    virtual Option<OdEntry> get_entry(const OdIndex idx) = 0;
    virtual Option<SubEntry> get_sub_entry(const Didx didx) = 0;

    template<typename T>
    requires ((sizeof(T) <= 4))
    SdoAbortCode write_any(const T pbuf, const Didx didx){return _write_any(&pbuf, didx);}

    template<typename T>
    requires ((sizeof(T) <= 4))
    SdoAbortCode read_any(T & pbuf, const Didx didx) const {return _read_any(&pbuf, didx);}
    virtual StringView ename(const Didx didx) const = 0;
};


#if 0
class ObjectDict:public ObjectDictIntf{
private:
    std::map<Index, OdEntry> dict_;
public:
    ObjectDict() = default;

    Option<OdEntry> operator [](const Index index){
        return Some(dict_[index]);
    }

    Option<SubEntry> operator [](const Didx id){
        auto & self = *this
        const auto [index, subindex] = id;
        const auto may_entry = (self[index]);
        if (may_entry.is_some()){
            return may_entry.unwrap()[subindex];
        }else{
            return None;
        }
    }

    SdoAbortCode write(const std::span<const uint8_t> pbuf, const Didx didx){
        return SdoAbortCode::None;
    }
    
    SdoAbortCode read(const std::span<uint8_t> pbuf, const Didx didx) const {
        return SdoAbortCode::None;
    }

	void insert(OdEntry && odEntry, const Index idx){
		dict_[idx] = std::move(odEntry);
	}

	void insert(const OdEntry & odEntry, const Index idx){
		dict_[idx] = odEntry;
	}

	void insert(SubEntry && se, const Index idx){
		dict_[idx].add(std::move(se));
	}
};

#endif


// #define ENTRY_NO_NAME

#ifdef ENTRY_NO_NAME
#define CANOPEN_NAME(name) std::nullopt
#else
#define CANOPEN_NAME(name) ({static constexpr StringView str = StringView(name, sizeof(name) - 1); str;})
#endif

class StaticObjectDictBase:public ObjectDictIntf{
protected:

    SdoAbortCode _write_any(const void * pbuf, const Didx didx) ;
    SdoAbortCode _read_any(void * pbuf, const Didx didx) const ;
public:
    StaticObjectDictBase() = default;
    StaticObjectDictBase(const StaticObjectDictBase & other) = delete;
    StaticObjectDictBase(StaticObjectDictBase && other) = delete;
    
    SdoAbortCode write(const std::span<const uint8_t> pbuf, const Didx didx) ;
    
    SdoAbortCode read(const std::span<uint8_t> pbuf, const Didx didx) const;

    StringView ename(const Didx didx) const ;
};



template<typename T>
constexpr SubEntry make_subentry_impl(
        const StringView name, 
        T & val, 
        EntryAccessAuthority access_type = std::is_const_v<T> ? EntryAccessAuthority::RO : EntryAccessAuthority::RW, 
        EntryDataType data_type = EntryDataType::from<reg_decay_t<T>>()){
    return SubEntry{name, val, access_type, data_type};
}

template<typename T>
constexpr SubEntry make_ro_subentry_impl(
        const StringView name, 
        const T & val, 
        EntryAccessAuthority access_type = std::is_const_v<T> ? EntryAccessAuthority::RO : EntryAccessAuthority::RW, 
        EntryDataType data_type = EntryDataType::from<reg_decay_t<T>>()){
    return SubEntry{name, val, access_type, data_type};
}


#define make_subentry(val) make_subentry_impl(NAME(#val), val);
#define make_ro_subentry(val) make_ro_subentry_impl(NAME(#val), val, EntryAccessAuthority::RO);
#define make_subentry_spec(val, access_type, data_type) make_subentry_impl(NAME(#val), val, access_type, data_type);

}