#pragma once

#include <array>
#include <algorithm>

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"

#include "core/utils/bits/bitfield_proxy.hpp"
#include <span>

namespace ymd::robots::dfrobot::cds55xx{

#define DEF_PROPERTY_BFPROXY(p_name, start_bit, stop_bit, p_type_name, bits)\
template <typename Self> [[nodiscard]] constexpr auto p_name(this Self && self) {\
    return ymd::make_bitfield_proxy<start_bit, stop_bit, p_type_name>(bits);}

#define DEF_PROPERTY_BIT(p_name, start_bit, bits) \
    DEF_PROPERTY_BFPROXY(p_name, start_bit, (start_bit + 1), bool, bits)

struct [[nodiscard]] StatusFlag final{


    using Self = StatusFlag;
    uint8_t bits;

    
    DEF_PROPERTY_BIT(is_over_voltage, 0, &self.bits); 
    DEF_PROPERTY_BIT(is_angle_outofrange, 1, &self.bits); 
    DEF_PROPERTY_BIT(is_overheat, 2, &self.bits); 
    DEF_PROPERTY_BIT(is_cmd_outofrange, 3, &self.bits); 
    DEF_PROPERTY_BIT(is_checksum_incorrect, 4, &self.bits); 

    // 位置模式运行时输出扭矩小于负载置1
    DEF_PROPERTY_BIT(is_override, 5, &self.bits); 

    // 如果收到一个未定义的指令或收到ACTION
    // 前未收到REG WRITE指令置1 
    DEF_PROPERTY_BIT(is_insc_invalid, 5, &self.bits); 

    [[nodiscard]] constexpr bool any() const {return bits == 0;}
};


struct [[nodiscard]] Instruction final{
public:
    enum struct [[nodiscard]] Kind:uint8_t{
        Ping = 0x01, 
        ReadData = 0x02,
        WriteData = 0x03,
        AsyncWrite = 0x04,
        InvokeAsync = 0x05,
        Reset = 0x06,
        SyncRead = 0x82,
        SyncWrite = 0x83
    };

    using enum Kind;

    constexpr Instruction(Kind kind):kind_(kind){}

    constexpr uint8_t to_u8() const noexcept {return static_cast<uint8_t>(kind_);}
private:
    Kind kind_;
};

static constexpr auto HEADER_TOKEN = std::to_array<uint8_t>({0xff, 0xff});

struct [[nodiscard]] ServoId final{
    uint8_t count;

    [[nodiscard]] constexpr bool is_boardcast() const {
        return count == 0xfe;
    }

    [[nodiscard]] constexpr bool is_unique() const {
        return count < 0xfe;
    }
};




namespace ins_msgs{


struct [[nodiscard]] WriteData final{
    static constexpr Instruction INSTRUCTION = Instruction::WriteData;
    static constexpr size_t PAYLOAD_LENGTH = 2;

    uint8_t addr;
    uint8_t val;
};


struct [[nodiscard]] ReadData final{
    static constexpr Instruction INSTRUCTION = Instruction::ReadData;
    static constexpr size_t PAYLOAD_LENGTH = 2;

    uint8_t base_addr;
    uint8_t len;
};

struct [[nodiscard]] AsyncWrite final{
    static constexpr Instruction INSTRUCTION = Instruction::AsyncWrite;
    static constexpr size_t PAYLOAD_LENGTH = 2;

    uint8_t addr;
    uint8_t val;
};

struct [[nodiscard]] InvokeAsync final{
    static constexpr Instruction INSTRUCTION = Instruction::InvokeAsync;
    static constexpr size_t PAYLOAD_LENGTH = 0;
};

struct [[nodiscard]] Ping final{
    static constexpr Instruction INSTRUCTION = Instruction::Ping;
    static constexpr size_t PAYLOAD_LENGTH = 0;
};

struct [[nodiscard]] Reset final{
    static constexpr Instruction INSTRUCTION = Instruction::Reset;
    static constexpr size_t PAYLOAD_LENGTH = 0;
};

struct [[nodiscard]] SyncWrite final{
    static constexpr Instruction INSTRUCTION = Instruction::SyncWrite;

    uint8_t base_addr;
    uint8_t len;
};


}


#undef DEF_PROPERTY_BFPROXY
}
