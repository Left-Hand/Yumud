#pragma once

#include "sys/io/regs.hpp"
#include <cstdint>
#include <initializer_list>

namespace ymd{
struct CanID16:public Reg16{
    const uint32_t __resv1__:3 = 0;
    const uint32_t ide:1 = 0;
    uint32_t rtr:1;
    uint32_t id:11;

    constexpr CanID16(const uint16_t _id, const bool rmt = false):
        rtr(rmt), id(_id){;}

    constexpr CanID16(const CanID16 & other) = default;
    constexpr CanID16(CanID16 && other) = default;

    CanID16 & remote(bool rmt){rtr = rmt; return *this;}
};

struct CanID32:public Reg32{
    const uint32_t __resv1__:1 = 0;
    uint32_t rtr:1;
    const uint32_t ide:1 = 1;
    uint32_t id:29;

    constexpr CanID32(const uint32_t _id, const bool rmt = false):
        rtr(rmt), id(_id){;}

    constexpr CanID32(const CanID32 & other) = default;
    constexpr CanID32(CanID32 && other) = default;

    CanID32 &  remote(bool rmt){rtr = rmt; return *this;}
};

class CanFilter{
private:
    using ID16 = CanID16;
    using ID32 = CanID32;
    void list16(const uint16_t id0, const uint16_t id1, const uint16_t id2, const uint16_t id3);
    void list32(const uint32_t id0, const uint32_t id1);
    void gate16();
    void gate32();

    bool is_remote();
protected:
    CAN_TypeDef * can;
    uint16_t idx;
    bool is32;
    bool islist;

    union{
        uint16_t id16[2];
        uint32_t id32;
    };

    union{
        uint16_t mask16[2];
        uint32_t mask32;
    };

    // CanFilter conv_copy(const bool rmt) const;
    void apply();
public:
    CanFilter(CAN_TypeDef * can_, const uint16_t idx_):can(can_), idx(idx_){};
    CanFilter(const CanFilter & other) = delete;
    CanFilter(CanFilter && other) = delete;

    void set(const ID16 & id, const ID16 & mask);
    void set(const ID16 & id1, const ID16 & mask1, const ID16 & id2, const ID16 & mask2);
    void set(const std::initializer_list<ID16> & list);

    void set(const ID32 & id, const ID32 & mask);
    void set(const std::initializer_list<ID32> & list);

    void init();
    static void init(const CanFilter & filter);

    // CanFilter copy() const{
    //     return *this;
    // }
    
    // CanFilter as_remote() const {
    //     return conv_copy(true);
    // }

    // CanFilter as_data() const {
    //     return conv_copy(false);
    // }
};

}