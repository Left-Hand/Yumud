#pragma once

#include "im6014_utils.hpp"
#include "im6014_prelude.hpp"

namespace ymd::robots::unitree::im6014{


enum class [[nodiscard]] CmdPacketBuildStep:uint8_t{
    Header,
    Status,
    Torque,
    Speed,
    Position,
    Kp,
    Kd,
    Crc
};


template<CmdPacketBuildStep>
struct CmdPacketBuilder;

template<>
struct [[nodiscard]] CmdPacketBuilder<CmdPacketBuildStep::Crc>final {
    uint8_t * ptr;

    constexpr void finalize() && {
        auto crc_builder = utils::Crc32Builder::from_default();

        if(std::is_constant_evaluated()){
            const uint8_t * org_ptr = ptr - 16;
            for(size_t i = 0; i < 16; i += 4){
                uint32_t word = 0;
                word |= org_ptr[i + 0] << 0;
                word |= org_ptr[i + 1] << 8;
                word |= org_ptr[i + 2] << 16;
                word |= org_ptr[i + 3] << 24;

                crc_builder = crc_builder.push_element(word);
            }
        }else{
            const uint32_t * org_ptr = reinterpret_cast<const uint32_t *>(std::assume_aligned<4>(ptr - 16));
            if(reinterpret_cast<uintptr_t>(org_ptr) & 0b11) __builtin_trap();
            for(size_t i = 0; i < 4; i ++){
                uint32_t word = org_ptr[i];
                crc_builder = crc_builder.push_element(word);
            }
        }

        const uint32_t crc32 = crc_builder.finalize();

        ptr = utils::ptr_push_u32(ptr, crc32);
    }
};



template<>
struct [[nodiscard]] CmdPacketBuilder<CmdPacketBuildStep::Kd>final {
    uint8_t * ptr;
constexpr 
    CmdPacketBuilder<CmdPacketBuildStep::Crc> push_kd_code(const KdCode code) && {
        ptr = utils::ptr_push_u16(ptr, code.bits);
        return {ptr};
    }
};


template<>
struct [[nodiscard]] CmdPacketBuilder<CmdPacketBuildStep::Kp>final {
    uint8_t * ptr;

    constexpr CmdPacketBuilder<CmdPacketBuildStep::Kd> push_kp_code(const KpCode code) && {
        ptr = utils::ptr_push_u16(ptr, code.bits);
        return {ptr};
    }
};



template<>
struct [[nodiscard]] CmdPacketBuilder<CmdPacketBuildStep::Position> final {
    uint8_t* ptr;

    constexpr CmdPacketBuilder<CmdPacketBuildStep::Kp> push_x1_code(const X1Code code) && {
        ptr = utils::ptr_push_u32(ptr, static_cast<uint32_t>(code.bits));
        return {ptr};
    }
};

template<>
struct [[nodiscard]] CmdPacketBuilder<CmdPacketBuildStep::Speed> final {
    uint8_t* ptr;

    constexpr CmdPacketBuilder<CmdPacketBuildStep::Position> push_x2_code(const X2Code code) && {
        ptr = utils::ptr_push_u16(ptr, code.bits);
        return {ptr};
    }
};



template<>
struct [[nodiscard]] CmdPacketBuilder<CmdPacketBuildStep::Torque> final {
    uint8_t* ptr;

    constexpr CmdPacketBuilder<CmdPacketBuildStep::Speed> push_tau_code(const TorqueCode code) && {
        ptr = utils::ptr_push_u16(ptr, code.bits);
        return {ptr};
    }
};

template<>
struct [[nodiscard]] CmdPacketBuilder<CmdPacketBuildStep::Status> final {
    uint8_t* ptr;

    constexpr CmdPacketBuilder<CmdPacketBuildStep::Torque> push_status(const StatusCode status_code) && {
        uint16_t bits = 0;
        bits |= (std::bit_cast<uint8_t>(status_code) << 0);
        bits |= (0x00 << 8);//reserved byte
        
        ptr = utils::ptr_push_u16(ptr, bits);
        return {ptr};
    }
};


template<>
struct [[nodiscard]] CmdPacketBuilder<CmdPacketBuildStep::Header> final {
    uint8_t* ptr;

    constexpr CmdPacketBuilder<CmdPacketBuildStep::Status> push_header() && {
        ptr = utils::ptr_push_u16(ptr, 0xfeee);
        return {ptr};
    }
};

static constexpr auto build_cmd_packet(std::span<uint8_t, 20> buffer){
    if(not std::is_constant_evaluated()){
        if(reinterpret_cast<uintptr_t>(buffer.data()) & 0b11) __builtin_trap();
    }
    return CmdPacketBuilder<CmdPacketBuildStep::Header>{buffer.data()};
}
}