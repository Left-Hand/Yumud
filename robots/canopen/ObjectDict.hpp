#pragma once

#include "Entry.hpp"
#include <map>
#include "core/utils/Option.hpp"

namespace ymd::canopen{


class ObjectDictIntf{
public:
    using Index = OdIndex;
    using SubIndex = OdSubIndex;
protected:
    virtual SdoAbortCode _write_any(const void * pdata, const Didx didx) = 0;
    virtual SdoAbortCode _read_any(void * pdata, const Didx didx) const = 0;
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


    virtual SdoAbortCode write(const std::span<const uint8_t> pdata, const Didx didx) = 0;
    virtual SdoAbortCode read(const std::span<uint8_t> pdata, const Didx didx) const = 0;

    template<typename T>
    requires ((sizeof(T) <= 4))
    SdoAbortCode write_any(const T pdata, const Didx didx){return _write_any(&pdata, didx);}

    template<typename T>
    requires ((sizeof(T) <= 4))
    SdoAbortCode read_any(T & pdata, const Didx didx) const {return _read_any(&pdata, didx);}
    virtual StringView ename(const Didx didx) const = 0;
};

class ObjectDict:public ObjectDictIntf{
private:
    std::map<Index, OdEntry> dict_;
public:
    ObjectDict() = default;

    std::optional<OdEntry> operator [](const Index index){
        return (dict_[index]);
    }

    std::optional<SubEntry> operator [](const Didx id){
        const auto [index, subindex] = id;
        auto entry_opt = ((*this)[index]);
        if (entry_opt.has_value()){
            return entry_opt.value()[subindex];
        }else{
            return std::nullopt;
        }
    }

    SdoAbortCode write(const std::span<const uint8_t> pdata, const Didx didx){
        return SdoAbortCode::None;
    }
    
    SdoAbortCode read(const std::span<uint8_t> pdata, const Didx didx) const {
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


// #define ENTRY_NO_NAME

#ifdef ENTRY_NO_NAME
#define NAME(name) std::nullopt
#else
#define NAME(name) StringView(name, sizeof(name))
#endif

class StaticObjectDictBase:public ObjectDictIntf{
protected:
    using CobId = uint16_t;
    SdoAbortCode _write_any(const void * pdata, const Didx didx) final override;
    SdoAbortCode _read_any(void * pdata, const Didx didx) const final override;
public:
    StaticObjectDictBase() = default;
    StaticObjectDictBase(const StaticObjectDictBase & other) = delete;
    StaticObjectDictBase(StaticObjectDictBase && other) = delete;
    
    SdoAbortCode write(const std::span<const uint8_t> pdata, const Didx didx) override;
    
    SdoAbortCode read(const std::span<uint8_t> pdata, const Didx didx) const override;

    StringView ename(const Didx didx) const final override;
    
    virtual std::optional<SubEntry> find(const Didx didx) = 0;
};



template<typename T>
struct reg_decay{
    // using type = void;
};

template<typename T>
requires std::is_base_of_v<__RegBase, T>
struct reg_decay<T>{
    using type = T::value_type;
};


template<typename T>
using reg_decay_t = typename reg_decay<T>::type;


template<typename T>
constexpr SubEntry make_subentry_impl(
        const StringView name, 
        T & val, 
        EntryAccessType access_type = std::is_const_v<T> ? EntryAccessType::RO : EntryAccessType::RW, 
        EntryDataType data_type = EntryDataType::from<reg_decay_t<T>>()){
    return SubEntry{name, val, access_type, data_type};
}


#define make_subentry(val) make_subentry_impl(NAME(#val), val);
#define make_ro_subentry(val) make_subentry_impl(NAME(#val), val, EntryAccessType::RO);
#define make_subentry_spec(val, access_type, data_type) make_subentry_impl(NAME(#val), val, access_type, data_type);

}