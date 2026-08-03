#pragma once

#include "core/utils/Result.hpp"
#include "core/utils/bits/bitfield_proxy.hpp"
#include "core/container/heapless_vector.hpp"
#include "primitive/arithmetic/angular.hpp"
#include "tamagawa_utils.hpp"

//多摩川编码器

// https://blog.csdn.net/qq_28149763/article/details/132718177



namespace ymd::drivers::tamagawa{

static constexpr size_t MAX_EEPROM_PAGE = 0x3c;


static constexpr size_t MAX_CONTEXT_SIZE = 8;



#define DEF_PROPERTY_BFPROXY(prop_name, start_bit, stop_bit, p_type_name, bits)\
[[nodiscard]] constexpr auto prop_name(this auto && self) {\
    return ymd::make_bitfield_proxy<start_bit, stop_bit, p_type_name>(bits);}

#define DEF_PROPERTY_BIT(prop_name, start_bit, bits) \
    DEF_PROPERTY_BFPROXY(prop_name, start_bit, (start_bit + 1), bool, bits)


enum class [[nodiscard]] CfCode:uint8_t{
    GetAbs = 0x02,
    GetAbm = 0x8a,
    GetVersion = 0x92,
    GetAllInfo = 0x1a,
    WriteEEprom = 0x32,
    ReadEEprom = 0xea,
    ClearAbs = 0xc2,
    ClearAbmAndFault = 0x62
};

struct [[nodiscard]] alignas(1) StatusField final{
    using Self = StatusField;

    uint8_t bits;
    
    DEF_PROPERTY_BIT(speed_err,              0, &self.bits);
    DEF_PROPERTY_BIT(comm_err   , 1, &self.bits);
    DEF_PROPERTY_BIT(battery_under_voltage         , 2, &self.bits);
    DEF_PROPERTY_BIT(supply_under_voltage       , 3, &self.bits);
    DEF_PROPERTY_BIT(install_err              , 4, &self.bits);
    DEF_PROPERTY_BIT(multi_turns_err          , 6, &self.bits);
    DEF_PROPERTY_BIT(temp_err          , 7, &self.bits);

    static constexpr Self from_u8(const uint8_t b){
        return std::bit_cast<Self>(b);
    }


    [[nodiscard]] constexpr bool is_none() const noexcept {
        return std::bit_cast<uint8_t>(*this) == 0;
    }

    [[nodiscard]] constexpr uint8_t to_u8() const noexcept {
        return std::bit_cast<uint8_t>(*this);
    }
};

static_assert(sizeof(StatusField));

struct [[nodiscard]] alignas(1) Almc final{
    uint8_t bits;

    DEF_PROPERTY_BIT(over_speed,              0, &self.bits);
    DEF_PROPERTY_BIT(full_absolute_status   , 1, &self.bits);
    DEF_PROPERTY_BIT(counting_error         , 2, &self.bits);
    DEF_PROPERTY_BIT(counter_overflow       , 3, &self.bits);
    DEF_PROPERTY_BIT(over_heat              , 4, &self.bits);
    DEF_PROPERTY_BIT(multiturn_error        , 5, &self.bits);
    DEF_PROPERTY_BIT(battery_error          , 6, &self.bits);
    DEF_PROPERTY_BIT(battery_alarm          , 7, &self.bits);
};

static_assert(sizeof(Almc));

struct [[nodiscard]] alignas(1) Abs24 final{
    using Self = Abs24;
    std::array<uint8_t, 3> bytes;

    constexpr uint32_t b24() const noexcept {
        uint32_t bits = 0;
        bits |= static_cast<uint32_t>(bytes[0]);
        bits |= static_cast<uint32_t>(bytes[1]) << 8;
        bits |= static_cast<uint32_t>(bytes[2]) << 16;
        return bits;
    }


    constexpr Angular<uq32> to_angle(size_t enc_resolution) const noexcept {
        const size_t shift_cnt = static_cast<size_t>(32u - enc_resolution);
        const uint32_t bits = static_cast<uint32_t>(b24()) << shift_cnt;
        return Angular<uq32>::from_turns(uq32::from_bits(bits));
    }

    constexpr void fill_bytes(std::span<uint8_t, 3> buf) const noexcept {
        buf[0] = bytes[0];
        buf[1] = bytes[1];
        buf[2] = bytes[2];
    }

    static constexpr Self from_bytes(std::span<const uint8_t, 3> bytes){
        return Self{
            .bytes = {bytes[0], bytes[1], bytes[2]}
        };
    }
};

static_assert(sizeof(Abs24));

struct [[nodiscard]] alignas(1) Abm24 final{
    using Self = Abm24;
    std::array<uint8_t, 3> bytes;

    constexpr uint32_t to_turns() const noexcept {
        uint32_t bits;
        bits |= static_cast<uint32_t>(bytes[0]);
        bits |= static_cast<uint32_t>(bytes[1]) << 8;
        bits |= static_cast<uint32_t>(bytes[2]) << 16;

        return bits;
    }

    constexpr void fill_bytes(std::span<uint8_t> other_bytes){
        other_bytes[0] = bytes[0];
        other_bytes[1] = bytes[1];
        other_bytes[2] = bytes[2];
    }

    static constexpr Self from_bytes(std::span<const uint8_t> bytes){
        return Self{
            .bytes = {bytes[0], bytes[1], bytes[2]}
        };
    }
};

static_assert(sizeof(Abm24));


#undef DEF_PROPERTY_BFPROXY
#undef DEF_PROPERTY_BIT

}