#pragma once

#include "../can_id.hpp"
#include "../can_enum.hpp"

namespace ymd::hal{


namespace details{
struct [[nodiscard]] SXX32_CanStdIdMask final{
    using id_type = CanStdId;

    #pragma pack(push, 1)
    const uint16_t __resv1__:3 = 0;
    const uint16_t __hw_wired_ide__:1 = 0;
    uint16_t is_remote:1;
    uint16_t id_u11:11;
    #pragma pack(pop)

    static constexpr SXX32_CanStdIdMask zero(){
        return {CanStdId(0), CanRtr::Data}; // data == 0
    }

    static constexpr  SXX32_CanStdIdMask from_full(){
        return {CanStdId(CanStdId::MAX_VALUE), CanRtr::Remote}; // remote == 1
    }

    static constexpr SXX32_CanStdIdMask from_lower_mask(const size_t len, const CanRtr rtr){
        return {CanStdId(uint16_t((1 << len) - 1)), rtr};
    }

    static constexpr SXX32_CanStdIdMask from_higher_mask(const size_t len, const CanRtr rtr){
        return {CanStdId(uint16_t(~(uint16_t(1 << len) - 1))), rtr};
    }

    [[nodiscard]] constexpr uint16_t to_bits() const{
        return std::bit_cast<uint16_t>(*this);
    }

    constexpr SXX32_CanStdIdMask(const hal::CanStdId _id, const CanRtr rtr):
        is_remote(rtr == CanRtr::Remote), id_u11(_id.to_u11()){;}

    constexpr SXX32_CanStdIdMask(const SXX32_CanStdIdMask & other) = default;
    constexpr SXX32_CanStdIdMask(SXX32_CanStdIdMask && other) = default;
private:

};

static_assert(sizeof(SXX32_CanStdIdMask) == 2);

struct [[nodiscard]] SXX32_CanExtIdMask final{
    using id_type = CanExtId;

    #pragma pack(push, 1)
    const uint32_t __resv1__:1 = 0;
    uint32_t is_remote:1;
    const uint32_t __hw_wired_ide__:1 = 1;
    uint32_t id_u29:29;
    #pragma pack(pop)

    static constexpr SXX32_CanExtIdMask zero(){
        return {CanExtId(0), CanRtr::Data}; // data == 0
    }

    static constexpr SXX32_CanExtIdMask from_full(){
        return {CanExtId(CanExtId::MAX_VALUE), CanRtr::Remote}; // remote == 1
    }

    static constexpr SXX32_CanExtIdMask from_lower_mask(const size_t len, const CanRtr rtr){
        return {CanExtId(uint32_t((1 << len) - 1)), rtr};
    }

    static constexpr SXX32_CanExtIdMask from_higher_mask(const size_t len, const CanRtr rtr){
        return {CanExtId(~uint32_t(uint32_t(1 << len) - 1)), rtr};
    }

    [[nodiscard]] constexpr uint32_t to_bits() const{
        return std::bit_cast<uint32_t>(*this);
    }
    constexpr SXX32_CanExtIdMask(const CanExtId _id, const CanRtr rtr):
        is_remote(uint8_t(rtr)), id_u29(_id.to_u29()){;}
    constexpr SXX32_CanExtIdMask(const SXX32_CanExtIdMask & other) = default;
    constexpr SXX32_CanExtIdMask(SXX32_CanExtIdMask && other) = default;
private:

};

static_assert(sizeof(SXX32_CanExtIdMask) == 4);
}



}