#pragma once

#include <initializer_list>
#include <bitset>

#include "sys/io/regs.hpp"
#include "CanUtils.hpp"


namespace ymd{
    class StringView;
}

namespace ymd::hal{

class Can;

struct CanID16{
    #pragma pack(push, 1)
    const uint16_t __resv1__:3 = 0;
    const uint16_t ide:1 = 0;
    uint16_t rtr:1;
    uint16_t id:11;
    #pragma pack(pop)

    scexpr CanID16 ACCEPT_ALL(){
        return {0, CanRemoteSpec::Data};
    }

    scexpr  CanID16 REJETC_ALL(){
        return {0xffff, CanRemoteSpec::Remote};
    }

    scexpr CanID16 IGNORE_HIGH(const size_t len, const CanRemoteSpec rmt){
        return {uint16_t((1 << len) - 1), rmt};
    }

    scexpr CanID16 IGNORE_LOW(const size_t len, const CanRemoteSpec rmt){
        return {uint16_t(~(uint16_t(1 << len) - 1)), rmt};
    }

    constexpr CanID16(const uint16_t _id, const CanRemoteSpec rmt):
        rtr(uint8_t(rmt)), id(_id){;}

    CanID16(const std::bitset<11> & bits, const CanRemoteSpec rmt):
        rtr(uint8_t(rmt)), 
        id(uint16_t(bits.to_ulong()))
    {;}
    
    constexpr CanID16(const CanID16 & other) = default;
    constexpr CanID16(CanID16 && other) = default;


    CanID16 & remote(bool rmt){rtr = rmt; return *this;}

    operator uint16_t() const{
        return *reinterpret_cast<const uint16_t *>(this);
    }
};

struct CanID32{
    #pragma pack(push, 1)
    const uint32_t __resv1__:1 = 0;
    uint32_t rtr:1;
    const uint32_t ide:1 = 1;
    uint32_t id:29;
    #pragma pack(pop)

    scexpr CanID32 ACCEPT_ALL(){
        return {0, CanRemoteSpec::Data};
    }

    scexpr  CanID32 REJETC_ALL(){
        return {0xffffffff, CanRemoteSpec::Remote};
    }

    scexpr CanID32 IGNORE_HIGH(const size_t len, const CanRemoteSpec rmt){
        return {uint32_t((1 << len) - 1), rmt};
    }

    scexpr CanID32 IGNORE_LOW(const size_t len, const CanRemoteSpec rmt){
        return {~uint32_t(uint32_t(1 << len) - 1), rmt};
    }


    constexpr CanID32(const uint32_t _id, const CanRemoteSpec rmt):
        rtr(uint8_t(rmt)), id(_id){;}

    CanID32(
        const std::bitset<29> & bits,
        const CanRemoteSpec rmt):
        
        rtr(uint8_t(rmt)), 
        id(uint32_t(bits.to_ulong())){}

    constexpr CanID32(const CanID32 & other) = default;
    constexpr CanID32(CanID32 && other) = default;
    
    CanID32 &  remote(bool rmt){rtr = rmt; return *this;}

    operator uint32_t() const{
        return *reinterpret_cast<const uint32_t *>(this);
    }
};

class CanFilter{
protected:
    using ID16 = CanID16;
    using ID32 = CanID32;

    CAN_TypeDef * can;
    
    union{
        uint16_t id16[2];
        uint32_t id32;
    };
    
    union{
        uint16_t mask16[2];
        uint32_t mask32;
    };
    
    uint8_t idx:6;
    uint8_t is32:1;
    uint8_t islist:1;

    void apply();

    bool is_remote();
    CanFilter(CAN_TypeDef * can_, const uint16_t idx_):can(can_), idx(idx_){};
    CanFilter(const CanFilter & other) = delete;
    CanFilter(CanFilter && other) = delete;

    friend class Can;
public:
    void mask(const ID16 & id, const ID16 & mask){
        this->mask(id, mask, ID16{0xffff,CanRemoteSpec::Remote}, ID16{0xffff,CanRemoteSpec::Remote});
    }
    void mask(const ID16 & id1, const ID16 & mask1, const ID16 & id2, const ID16 & mask2);
    void list(const std::initializer_list<ID16> & list);
    void mask(const std::initializer_list<ID16> & masks);
    void all();

    void mask(const ID32 & id, const ID32 & mask);
    void list(const std::initializer_list<ID32> & list);
    void mask(const std::initializer_list<ID32> & masks);

    bool bystr(const StringView & str);
    void deinit();

};

}