#pragma once

#include <cstdint>


namespace cpu{
    struct sp{};
    struct t0{};
    struct mepc{};
    struct mstatus{};

    struct x1{};
    struct x3{};
    struct x4{};
    struct x5{};
    struct x6{};
    struct x7{};
    struct x8{};
    struct x9{};
    struct x10{};
    struct x11{};
    struct x12{};
    struct x13{};
    struct x14{};
    struct x15{};
    struct x16{};
    struct x17{};
    struct x18{};
    struct x19{};
    struct x20{};
    struct x21{};
    struct x22{};
    struct x23{};
    struct x24{};
    struct x25{};
    struct x26{};
    struct x27{};
    struct x28{};
    struct x29{};
    struct x30{};
    struct x31{};

    #ifdef __riscv_f
    struct f1{};
    struct f2{};
    struct f3{};
    struct f4{};
    struct f5{};
    struct f6{};
    struct f7{};
    struct f8{};
    struct f9{};
    struct f10{};
    struct f11{};
    struct f12{};
    struct f13{};
    struct f14{};
    struct f15{};
    struct f16{};
    struct f17{};
    struct f18{};
    struct f19{};
    struct f20{};
    struct f21{};
    struct f22{};
    struct f23{};
    struct f24{};
    struct f25{};
    struct f26{};
    struct f27{};
    struct f28{};
    struct f29{};
    struct f30{};
    struct f31{};
    #endif
}


#define QKV4_REG_DEF(name) static inline volatile name##_Def * name = reinterpret_cast<name##_Def *>(name##_Def::ADDRESS);

namespace QingKeV4{
    struct PFIC_CFGR_Def{
        static constexpr uint32_t ADDRESS = 0xE000E04C;
        uint32_t NESTSTA:8;
        uint32_t GACTSTA:1;
        uint32_t GPENDSTA:1;
        uint32_t:22;
    };

    QKV4_REG_DEF(PFIC_CFGR)


}