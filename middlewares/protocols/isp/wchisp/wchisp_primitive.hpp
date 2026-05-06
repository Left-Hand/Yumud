#include <cstdint>
#include <cstddef>
#include <span>
#include <array>

namespace ymd::wchisp{

static constexpr size_t MAX_PACKET_SIZE = 64;
static constexpr size_t SECTOR_SIZE = 1024;


enum class [[nodiscard]] CommandKind:uint8_t{
    Identify = 0xa1,
    IspEnd = 0xa2,
    IspKey = 0xa3,
    Erase = 0xa4,
    Program = 0xa5,
    Verify = 0xa6,
    ReadConfig = 0xa7,
    WriteConfig = 0xa8,
    DataErase = 0xa9,
    DataProgram = 0xaa,
    DataRead = 0xab,
    WriteOtp = 0xc3,
    ReadOtp = 0xc4,
    SetBaud = 0xc5,
};

enum class Error:uint8_t{
    InvalidResponse
};

struct [[nodiscard]] Response final{
    const uint8_t * p_bytes_;
    size_t length_;

    constexpr std::span<const uint8_t> payload() const noexcept {
        return std::span<const uint8_t>(p_bytes_,length_);
    }
    // constexpr bool is_ok() const noexcept {
    //     return is_ok_;
    // }

    // constexpr bool is_err() const noexcept {
    //     return not is_ok();
    // }

    // static constexpr from_raw_bytes(std::span<const uint8_t> bytes){

    // }
};


static constexpr std::array<uint8_t, 8> xor_key(
    uint8_t chip_id,
    std::span<const uint8_t> chip_uid
) {
    uint8_t checksum = 0;
    for (uint8_t byte : chip_uid) {
        checksum += byte;
    }

    std::array<uint8_t, 8> key;
    key.fill(checksum);

    key.back() += chip_id;

    return key;
}

}