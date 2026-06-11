#pragma once

#include "m8010_prelude.hpp"
#include "m8010_utils.hpp"

namespace ymd::robots::unitree::m8010 {

enum class [[nodiscard]] RxFrameBuildStep : uint8_t {
    Header,
    Mode,
    Torque,
    Speed,
    Position,
    Temperature,
    Misc,
    Crc
};

template<RxFrameBuildStep>
struct RxFrameBuilder;

template<>
struct [[nodiscard]] RxFrameBuilder<RxFrameBuildStep::Crc> final {
    uint8_t* ptr;

    constexpr void finalize() && {
        const uint16_t crc16 = Crc16Builder::from_default()
            .push_bytes(std::span<const uint8_t>(ptr - 14, 14))
            .finalize();

        ptr = u8ptr_push_u16le(ptr, crc16);
    }
};

template<>
struct [[nodiscard]] RxFrameBuilder<RxFrameBuildStep::Misc> final {
    uint8_t* ptr;

    constexpr RxFrameBuilder<RxFrameBuildStep::Crc> push_misc(const RxMisc misc) && {
        ptr = u8ptr_push_u16le(ptr, misc.to_u16());
        return {ptr};
    }
};

template<>
struct [[nodiscard]] RxFrameBuilder<RxFrameBuildStep::Temperature> final {
    uint8_t* ptr;

    constexpr RxFrameBuilder<RxFrameBuildStep::Misc> push_temp_code(const TempCode code) && {
        ptr = u8ptr_push_u8le(ptr, code.bits);
        return {ptr};
    }
};

template<>
struct [[nodiscard]] RxFrameBuilder<RxFrameBuildStep::Position> final {
    uint8_t* ptr;

    constexpr RxFrameBuilder<RxFrameBuildStep::Temperature> push_x1_code(const X1Code code) && {
        ptr = u8ptr_push_u32le(ptr, code.bits);
        return {ptr};
    }
};

template<>
struct [[nodiscard]] RxFrameBuilder<RxFrameBuildStep::Speed> final {
    uint8_t* ptr;

    constexpr RxFrameBuilder<RxFrameBuildStep::Position> push_x2_code(const X2Code code) && {
        ptr = u8ptr_push_u16le(ptr, code.bits);
        return {ptr};
    }
};

template<>
struct [[nodiscard]] RxFrameBuilder<RxFrameBuildStep::Torque> final {
    uint8_t* ptr;

    constexpr RxFrameBuilder<RxFrameBuildStep::Speed> push_tau_code(const TorqueCode code) && {
        ptr = u8ptr_push_u16le(ptr, code.bits);
        return {ptr};
    }
};

template<>
struct [[nodiscard]] RxFrameBuilder<RxFrameBuildStep::Mode> final {
    uint8_t* ptr;

    constexpr RxFrameBuilder<RxFrameBuildStep::Torque> push_mode(const ModeInfo mode_info) && {
        ptr = u8ptr_push_u8le(ptr, std::bit_cast<uint8_t>(mode_info));
        return {ptr};
    }
};

template<>
struct [[nodiscard]] RxFrameBuilder<RxFrameBuildStep::Header> final {
    uint8_t* ptr;

    constexpr RxFrameBuilder<RxFrameBuildStep::Mode> push_header() && {
        ptr = u8ptr_push_u16le(ptr, 0xfdee); // RxHeader 使用 0xfd, 0xee
        return {ptr};
    }
};

static constexpr auto build_rx_frame(std::span<uint8_t, RX_FRAME_SIZE> buffer) {
    return RxFrameBuilder<RxFrameBuildStep::Header>{buffer.data()};
}

}