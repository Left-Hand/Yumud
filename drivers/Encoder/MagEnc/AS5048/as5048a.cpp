#include "as5048.hpp"

using namespace ymd;
using namespace ymd::drivers::as5048;

namespace {
struct [[nodiscard]] SpiRequestPacket final{
    uint16_t reg_addr:14;
    uint16_t is_read:1;
    uint16_t is_even:1;

    [[nodiscard]] constexpr uint16_t to_bits() const noexcept {
        return std::bit_cast<uint16_t>(*this);
    }

    [[nodiscard]] constexpr bool is_verification_passed() const noexcept {
        return std::popcount(to_bits()) % 2 == is_even;
    }
};

static_assert(sizeof(SpiRequestPacket) == sizeof(uint16_t));

struct [[nodiscard]] SpiResponsePacket final{
    uint16_t data:14;
    // Error flag indicating a transmission error in a previous host transmission
    uint16_t error_flag:1;
    uint16_t is_even:1;

    [[nodiscard]] constexpr uint16_t to_bits() const noexcept {
        return std::bit_cast<uint16_t>(*this);
    }

    [[nodiscard]] constexpr bool is_verification_passed() const noexcept {
        return std::popcount(to_bits()) % 2 == is_even;
    }
};

static_assert(sizeof(SpiResponsePacket) == sizeof(uint16_t));
}