#pragma once

#include "Entry.hpp"
#include <map>

namespace ymd::canopen{


class ObjectDictIntf{
public:
    using Index = OdIndex;
    using SubIndex = OdSubIndex;
protected:
    virtual EntryAccessError _write_any(const void * pdata, const std::pair<const Index, const SubIndex> didx) = 0;
    virtual EntryAccessError _read_any(void * pdata, const std::pair<const Index, const SubIndex> didx) const = 0;
public:

    template<typename T>
    requires ((sizeof(T) <= 4))
    EntryAccessError write(const auto data, const std::pair<const Index, const SubIndex> didx){
        static_assert(std::is_convertible_v<T, decltype(data)>, "type mismatch");
        return write({reinterpret_cast<const uint8_t *>(&data), sizeof(T)}, {didx.first, didx.second});
    }

    template<typename T, typename U>
    requires (sizeof(T) <= 4)
    EntryAccessError read(U & data, const std::pair<const Index, const SubIndex> didx) const {
        static_assert(std::is_same_v<T, U>, "type mismatch");
        return read({reinterpret_cast<uint8_t *>(&data), sizeof(T)}, {didx.first, didx.second});
    }


    virtual EntryAccessError write(const std::span<const uint8_t> pdata, const std::pair<const Index, const SubIndex> didx) = 0;
    virtual EntryAccessError read(const std::span<uint8_t> pdata, const std::pair<const Index, const SubIndex> didx) const = 0;

    template<typename T>
    requires ((sizeof(T) <= 4))
    EntryAccessError write_any(const T pdata, const std::pair<const Index, const SubIndex> didx){return _write_any(&pdata, didx);}

    template<typename T>
    requires ((sizeof(T) <= 4))
    EntryAccessError read_any(T & pdata, const std::pair<const Index, const SubIndex> didx) const {return _read_any(&pdata, didx);}
    virtual StringView ename(const std::pair<const Index, const SubIndex> didx) const = 0;
};

class ObjectDict:public ObjectDictIntf{
private:
    std::map<Index, OdEntry> dict_;
public:
    ObjectDict() = default;

    std::optional<OdEntry> operator [](const Index index){
        return (dict_[index]);
    }

    std::optional<SubEntry> operator [](const std::pair<const Index, const SubIndex> id){
        const auto [index, subindex] = id;
        auto entry_opt = ((*this)[index]);
        if (entry_opt.has_value()){
            return entry_opt.value()[subindex];
        }else{
            return std::nullopt;
        }
    }

    EntryAccessError write(const std::span<const uint8_t> pdata, const std::pair<const Index, const SubIndex> didx){
        return EntryAccessError::None;
    }
    
    EntryAccessError read(const std::span<uint8_t> pdata, const std::pair<const Index, const SubIndex> didx) const {
        return EntryAccessError::None;
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
    EntryAccessError _write_any(const void * pdata, const std::pair<const Index, const SubIndex> didx) final override;
    EntryAccessError _read_any(void * pdata, const std::pair<const Index, const SubIndex> didx) const final override;
public:
    StaticObjectDictBase() = default;
    StaticObjectDictBase(const StaticObjectDictBase & other) = delete;
    StaticObjectDictBase(StaticObjectDictBase && other) = delete;


    
    EntryAccessError write(const std::span<const uint8_t> pdata, const std::pair<const Index, const SubIndex> didx) final override;
    
    EntryAccessError read(const std::span<uint8_t> pdata, const std::pair<const Index, const SubIndex> didx) const final override;



    StringView ename(const std::pair<const Index, const SubIndex> didx) const final override;
    
    virtual std::optional<SubEntry> find(const std::pair<const Index, const SubIndex> didx) = 0;
};



template<typename T>
struct reg_decay{
    using type = std::conditional_t<
        std::is_base_of_v<__RegBase, T>,
        typename T::value_type, T>;
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