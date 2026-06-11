#pragma once

#include "m8010_prelude.hpp"
#include "m8010_utils.hpp"


namespace ymd::robots::unitree::m8010 {

enum class [[nodiscard]] TxPacketBuildStep : uint8_t {
    Header,
    Mode,
    Torque,
    Speed,
    Position,
    Kp,
    Kd,
    Crc
};


template<TxPacketBuildStep>
struct TxPacketBuilder;

template<>
struct [[nodiscard]] TxPacketBuilder<TxPacketBuildStep::Crc> final {
    uint8_t* ptr;

    constexpr void finalize() && {
        const uint16_t crc16 = Crc16Builder::from_default()
            .push_bytes(std::span<const uint8_t>(ptr - 15, 15))
            .finalize();

        ptr = u8ptr_push_u16le(ptr, crc16);
    }
};


template<>
struct [[nodiscard]] TxPacketBuilder<TxPacketBuildStep::Kd> final {
    uint8_t* ptr;

    constexpr TxPacketBuilder<TxPacketBuildStep::Crc> push_kd_code(const KdCode code) && {
        ptr = u8ptr_push_u16le(ptr, code.bits);
        return {ptr};
    }
};


template<>
struct [[nodiscard]] TxPacketBuilder<TxPacketBuildStep::Kp> final {
    uint8_t* ptr;

    constexpr TxPacketBuilder<TxPacketBuildStep::Kd> push_kp_code(const KpCode code) && {
        ptr = u8ptr_push_u16le(ptr, code.bits);
        return {ptr};
    }
};


template<>
struct [[nodiscard]] TxPacketBuilder<TxPacketBuildStep::Position> final {
    uint8_t* ptr;

    constexpr TxPacketBuilder<TxPacketBuildStep::Kp> push_x1_code(const X1Code code) && {
        ptr = u8ptr_push_u32le(ptr, code.bits);
        return {ptr};
    }
};


template<>
struct [[nodiscard]] TxPacketBuilder<TxPacketBuildStep::Speed> final {
    uint8_t* ptr;

    constexpr TxPacketBuilder<TxPacketBuildStep::Position> push_x2_code(const X2Code code) && {
        ptr = u8ptr_push_u16le(ptr, code.bits);
        return {ptr};
    }
};


template<>
struct [[nodiscard]] TxPacketBuilder<TxPacketBuildStep::Torque> final {
    uint8_t* ptr;

    constexpr TxPacketBuilder<TxPacketBuildStep::Speed> push_tau_code(const TorqueCode code) && {
        ptr = u8ptr_push_u16le(ptr, code.bits);
        return {ptr};
    }
};


template<>
struct [[nodiscard]] TxPacketBuilder<TxPacketBuildStep::Mode> final {
    uint8_t* ptr;

    constexpr TxPacketBuilder<TxPacketBuildStep::Torque> push_mode(const ModeInfo mode_info) && {
        ptr = u8ptr_push_u8le(ptr, std::bit_cast<uint8_t>(mode_info));
        return {ptr};
    }
};


template<>
struct [[nodiscard]] TxPacketBuilder<TxPacketBuildStep::Header> final {
    uint8_t* ptr;

    constexpr TxPacketBuilder<TxPacketBuildStep::Mode> push_header() && {
        ptr = u8ptr_push_u16le(ptr, 0xfeee);
        return {ptr};
    }
};


static constexpr auto build_tx_frame(std::span<uint8_t, 17> buffer) {
    return TxPacketBuilder<TxPacketBuildStep::Header>{buffer.data()};
}

}