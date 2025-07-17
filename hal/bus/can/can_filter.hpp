#pragma once

#include <initializer_list>
#include <bitset>

#include "core/io/regs.hpp"
#include "can_utils.hpp"


namespace ymd{
    class StringView;
}

namespace ymd::hal{

class Can;

struct CanStdIdMask{
    #pragma pack(push, 1)
    const uint16_t __resv1__:3 = 0;
    const uint16_t ide:1 = 0;
    uint16_t rtr:1;
    uint16_t id:11;
    #pragma pack(pop)

    static constexpr CanStdIdMask from_accept_all(){
        return {0, CanRemoteSpec::Data};
    }

    static constexpr  CanStdIdMask from_reject_all(){
        return {0xffff, CanRemoteSpec::Remote};
    }

    static constexpr CanStdIdMask from_ignore_high(const size_t len, const CanRemoteSpec rmt){
        return {uint16_t((1 << len) - 1), rmt};
    }

    static constexpr CanStdIdMask from_ignore_low(const size_t len, const CanRemoteSpec rmt){
        return {uint16_t(~(uint16_t(1 << len) - 1)), rmt};
    }

    constexpr uint16_t as_u16() const{
        return std::bit_cast<uint16_t>(*this);
    }
    constexpr CanStdIdMask(const uint16_t _id, const CanRemoteSpec rmt):
        rtr(uint8_t(rmt)), id(_id){;}

    constexpr CanStdIdMask(const CanStdIdMask & other) = default;
    constexpr CanStdIdMask(CanStdIdMask && other) = default;
private:

    


};

struct CanExtIdMask{
    #pragma pack(push, 1)
    const uint32_t __resv1__:1 = 0;
    uint32_t rtr:1;
    const uint32_t ide:1 = 1;
    uint32_t id:29;
    #pragma pack(pop)

    static constexpr CanExtIdMask from_accept_all(){
        return {0, CanRemoteSpec::Data};
    }

    static constexpr CanExtIdMask from_reject_all(){
        return {0xffffffff, CanRemoteSpec::Remote};
    }

    static constexpr CanExtIdMask from_ignore_high(const size_t len, const CanRemoteSpec rmt){
        return {uint32_t((1 << len) - 1), rmt};
    }

    static constexpr CanExtIdMask from_ignore_low(const size_t len, const CanRemoteSpec rmt){
        return {~uint32_t(uint32_t(1 << len) - 1), rmt};
    }



    constexpr uint32_t as_u32() const{
        return std::bit_cast<uint32_t>(*this);
    }
    constexpr CanExtIdMask(const uint32_t _id, const CanRemoteSpec rmt):
        rtr(uint8_t(rmt)), id(_id){;}
    constexpr CanExtIdMask(const CanExtIdMask & other) = default;
    constexpr CanExtIdMask(CanExtIdMask && other) = default;
private:



};

class CanFilter{
protected:

    CAN_TypeDef * can_;
    
    union{
        uint16_t id16[2];
        uint32_t id32;
    };
    
    union{
        uint16_t mask16[2];
        uint32_t mask32;
    };
    
    uint8_t idx_:6;
    uint8_t is32_:1;
    uint8_t islist_:1;

    void apply();

    bool is_remote();
    CanFilter(CAN_TypeDef * can, const uint16_t idx):can_(can), idx_(idx){};
    CanFilter(const CanFilter & other) = delete;
    CanFilter(CanFilter && other) = delete;

    friend class Can;
public:
    struct StdIdMaskPair{
        CanStdIdMask id;
        CanStdIdMask mask;
    };

    struct ExtIdMaskPair{
        CanStdIdMask id;
        CanStdIdMask mask;
    };

    static constexpr StdIdMaskPair STD_ALL_PASS = {
        CanStdIdMask{0xffff,CanRemoteSpec::Remote}, 
        CanStdIdMask{0xffff,CanRemoteSpec::Remote}
    };

    void mask(const StdIdMaskPair & pair){
        this->mask(pair, STD_ALL_PASS);
    }

    void mask(const StdIdMaskPair & pair1, const StdIdMaskPair & pair2);
    void list(const std::initializer_list<CanStdIdMask> & list);
    void mask(const std::initializer_list<CanStdIdMask> & masks);
    void all();


    void mask(const CanExtIdMask & id, const CanExtIdMask & mask);
    void list(const std::initializer_list<CanExtIdMask> & list);
    void mask(const std::initializer_list<CanExtIdMask> & masks);

    bool bystr(const StringView & str);
    void deinit();

};

}