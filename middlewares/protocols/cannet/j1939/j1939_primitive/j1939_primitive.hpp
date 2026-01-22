#pragma once

#include "primitive/can/can_id.hpp"
#include "core/utils/bits/BitField.hpp"

// https://blog.csdn.net/geek_liyang/article/details/148498704
namespace ymd::j1939{

namespace primitive{

struct [[nodiscard]] SourceAddr{
    using Self = SourceAddr;
    uint8_t bits;
};

struct [[nodiscard]] PsField{
    using Self = PsField;
    uint8_t bits;
};

struct [[nodiscard]] PduFormat{
    using Self = PduFormat;
    uint8_t bits;

    [[nodiscard]] constexpr bool is_pdu1() const {
        return bits < 240;
    }

    [[nodiscard]] constexpr bool is_pdu2() const {
        return bits & 0b11001000;
    }
};

enum class [[nodiscard]] DataPage:uint8_t{
    Page0 = 0b00,
    Page1 = 0b01,
    Presvered = 0b10,
    Iso15763_3 = 0b11
};

enum class [[nodiscard]] Priority:uint8_t{

};


struct alignas(4) [[nodiscard]] Pdn final{
    using Self = Pdn;

    uint32_t bits;

    // SourceAddr sa;
    // PsField ps;
    // PduFormat pf;
    // DataPage dp:2;

    // // 占3bit，根据CAN2.0B 的仲裁机制,ID越小优先级越高，
    // // 按照J1939协议的划分，优先级在整个ID的最前面，实际上依然控制着CAN报文的优先级。
    // // 只不过在J1939协议中优先级仅仅用于优化发送数据时的报文延迟，接收报文时则完全忽略优先级。 
    // // J1939中的优先级可以从最高的0(000b)到最低优先级7(111b)。
    // // 默认情况下控制类报文的优先级为3,其他报文的优先级为6。 
    // // 当分配新的PGN或总线上流量改变时，允许提高或者降低优先级。
    // Priority p:3;

    static constexpr Self from_bits(const uint32_t bits) {
        return std::bit_cast<Pdn>(bits);
    }
    
    static constexpr Self from_can_id(const hal::CanExtId id) {
        return Self::from_bits(id.to_bits());
    }

    [[nodiscard]] constexpr uint32_t to_bits() const {
        return std::bit_cast<uint32_t>(*this);
    }

    constexpr hal::CanExtId to_can_id() const {
        return hal::CanExtId::from_bits(to_bits());
    }

    [[nodiscard]] constexpr auto sa() const {
        return make_bitfield_proxy<0, 8, SourceAddr>(bits);
    }

    [[nodiscard]] constexpr auto ps() const {
        return make_bitfield_proxy<8, 16, PsField>(bits);
    }

    [[nodiscard]] constexpr auto pf() const {
        return make_bitfield_proxy<16, 24, PduFormat>(bits);
    }

    [[nodiscard]] constexpr auto dp() const {
        return make_bitfield_proxy<24, 26, DataPage>(bits);
    }

    [[nodiscard]] constexpr auto priority() const {
        return make_bitfield_proxy<26, 29, Priority>(bits);
    }
};


static_assert(sizeof(Pdn) == 4);


struct alignas(8) [[nodiscard]]EcuName final{
    using Self = EcuName;

    std::array<uint8_t, 8> bytes;

    static constexpr Self from_bytes(std::span<const uint8_t, 8> bytes){
        Self self;
        std::copy(bytes.begin(), bytes.end(), self.bytes.begin());
        return self;
    }

    static constexpr Self from_u64(const uint64_t bits){
        return std::bit_cast<EcuName>(bits);
    }
};

static_assert(sizeof(EcuName) == 8);

}
}