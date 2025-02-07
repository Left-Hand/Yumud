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


class NmtObjectDict:public StaticObjectDictBase{
protected:
    struct{
    #pragma pack(push, 1)
        uint32_t device_type_;
        uint8_t error_register_;
        uint32_t manufacturer_status_;
        uint8_t num_errors_;
        uint32_t error_code_1_;
        uint32_t error_code_2_;
        uint32_t manufacturer_specific_1_;
        uint32_t manufacturer_specific_2_;
    #pragma pack(pop)
    };
public:
    NmtObjectDict() = default;

    std::optional<SubEntry> find(const std::pair<const Index, const SubIndex> didx) final override;
};


class Cia402ObjectDict:public StaticObjectDictBase{
protected:
    struct{
    #pragma pack(push, 1)
        uint16_t control_word_;
        uint16_t status_word_;
        int8_t modes_of_operation_;
        int8_t modes_of_operation_display_;
        int32_t position_actual_value_;
        int32_t velocity_actual_value_;
        int16_t torque_actual_value_;
        int32_t position_demand_value_;
        int32_t velocity_demand_value_;
        int16_t torque_demand_value_;
        int32_t target_position_;
        int32_t target_velocity_;
        int16_t target_torque_;
        int32_t max_profile_velocity_;
        int16_t max_motor_current_;
        int32_t max_profile_acceleration_;
        int32_t max_profile_deceleration_;
        int32_t software_position_limit_;
        int32_t software_velocity_limit_;
        int16_t software_torque_limit_;
    #pragma pack(pop)
    };
public:
    Cia402ObjectDict() = default;

    std::optional<SubEntry> find(const std::pair<const Index, const SubIndex> didx) final override;
};


}