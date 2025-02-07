#pragma once

#include "Entry.hpp"
#include <map>

namespace ymd::canopen{


class ObjectDictIntf{
public:
    using Index = OdIndex;
    using SubIndex = OdSubIndex;



    virtual EntryAccessError write(const std::span<const uint8_t> pdata, const std::pair<const Index, const SubIndex> didx) = 0;
    virtual EntryAccessError read(const std::span<uint8_t> pdata, const std::pair<const Index, const SubIndex> didx) const = 0;
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
#define NAME(name) ""
#else
#define NAME(name) StringView(name, sizeof(name))
#endif

class StaticObjectDictBase:public ObjectDictIntf{
protected:
    using CobId = uint16_t;
public:
    StaticObjectDictBase() = default;
    StaticObjectDictBase(const StaticObjectDictBase & other) = delete;
    StaticObjectDictBase(StaticObjectDictBase && other) = delete;

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
    
    EntryAccessError write(const std::span<const uint8_t> pdata, const std::pair<const Index, const SubIndex> didx) final override;
    
    EntryAccessError read(const std::span<uint8_t> pdata, const std::pair<const Index, const SubIndex> didx) const final override;
    
    StringView ename(const std::pair<const Index, const SubIndex> didx) const final override;
    
    virtual std::optional<SubEntry> find(const std::pair<const Index, const SubIndex> didx) = 0;
};



class SdoObjectDict:public StaticObjectDictBase{
protected:
    struct{
    #pragma pack(push, 1)
        CobId c2s_cobid_ = 0;
        CobId s2c_cobid_ = 0;
        uint16_t heartbeat_time_ = 0;
        uint16_t node_guarding_time_ = 0;
        uint32_t sync_period_ = 0;
        uint32_t sync_window_length_ = 0;
        uint32_t emergency_consumer_cobid_ = 0;
        uint32_t emergency_producer_cobid_ = 0;
    #pragma pack(pop)
    };
public:
    SdoObjectDict() = default;

    std::optional<SubEntry> find(const std::pair<const Index, const SubIndex> didx) final override;
};

}