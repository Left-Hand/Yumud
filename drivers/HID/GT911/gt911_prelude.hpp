#pragma once

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"
#include "core/string/char_array.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"

#include "algebra/vectors/vec2.hpp"
#include "core/utils/nth.hpp"

namespace ymd::drivers{


struct [[nodiscard]] GT9XX_Prelude{
protected:
    using RegAddr = uint16_t;
    static constexpr uint8_t GT9XX_I2C_ADDR_BA  = 0x5D;
    static constexpr RegAddr GT9XX_PRODUCT_ID_REG  = 0x8140;
    static constexpr RegAddr GT9XX_TOUCHPOINT_STATUS_REG  = 0x814E;
    static constexpr RegAddr GT9XX_TOUCHPOINT_1_REG  = 0x814F;
    static constexpr RegAddr GT9XX_COMMAND_REG  = 0x8040;

    static constexpr size_t MAX_NUM_TOUCHPOINTS  = 5;

public:

    enum class Command : uint8_t{
        ReadStatus,
        DiffToOriginal,
        SoftReset,
        UpdateBias,
        CalibrateBias,
        ScreenPowerDown
    };

    static constexpr size_t TOUCHPOINT_ENTRY_LEN  = 8;

    struct [[nodiscard]] TouchPoint{
        using Self = TouchPoint;
        uint8_t track_id;
        uint16_t x;
        uint16_t y;
        uint16_t area;

        [[nodiscard]] static constexpr Self from_bytes(
            const std::span<const uint8_t, TOUCHPOINT_ENTRY_LEN> bytes
        ) {

            return Self {
                .track_id = bytes[0],
                .x = static_cast<uint16_t>(bytes[1] | (bytes[2] << 8)),  // Little endian
                .y = static_cast<uint16_t>(bytes[3] | (bytes[4] << 8)),  // Little endian
                .area = static_cast<uint16_t>(bytes[5] | (bytes[6] << 8))  // Little endian
            };
        }
    };


    static_assert(TOUCHPOINT_ENTRY_LEN == 8);

    static constexpr size_t GET_TOUCH_BUF_SIZE = TOUCHPOINT_ENTRY_LEN;
    static constexpr size_t GET_MULTITOUCH_BUF_SIZE = TOUCHPOINT_ENTRY_LEN * MAX_NUM_TOUCHPOINTS;

    enum class [[nodiscard]] ErrorKind:uint8_t{
        UnexpectedProductId,
        UnexpectedData,
        NotReady
    };

    DEF_ERROR_SUMWITH_HALERROR(Error, ErrorKind)

    struct [[nodiscard]] FamilySpecific { 
        CharArray<4> name;
        size_t max_points_count;
    };

protected:


    [[nodiscard]] static constexpr TouchPoint decode_specified_point(
        const std::span<const uint8_t, GET_MULTITOUCH_BUF_SIZE> bytes, 
        const Nth nth
    ){
        const auto start = nth.count() * TOUCHPOINT_ENTRY_LEN;
        return TouchPoint::from_bytes(std::span<const uint8_t, TOUCHPOINT_ENTRY_LEN>(
            bytes.data() + start, TOUCHPOINT_ENTRY_LEN));
    }

    [[nodiscard]] static constexpr RegAddr map_nth_to_addr(
        const Nth nth
    ){
        return GT9XX_TOUCHPOINT_1_REG + nth.count() * TOUCHPOINT_ENTRY_LEN;
    }

    enum class [[nodiscard]] WorkMode:uint8_t{
        Normal,
        LowPower,
        Sleep
    };

    enum class [[nodiscard]] InterruptTriggerMethod:uint8_t{
        RisingEdge,
        FallingEdge,
        LowLevel,
        HighLevel
    };
};


struct [[nodiscard]] GT9XX_Regs:public GT9XX_Prelude{
    struct [[nodiscard]] R32_ProductId{
        static constexpr RegAddr NUM_ADDRESS = 0x8140;
        std::array<uint8_t, 4> id;

        constexpr bool is_valid() const {
            return id[0] == '9' && id[1] == '1' && id[2] == '1' && id[3] == '\0';
        }
    };

    struct [[nodiscard]] R16_Firmware{
        static constexpr RegAddr NUM_ADDRESS = 0x8144;

        uint8_t low;
        uint8_t high;
    };

    struct [[nodiscard]] R32_Resolution{
        static constexpr RegAddr NUM_ADDRESS = 0x8146;

        uint16_t x;
        uint16_t y;
    };

    struct [[nodiscard]] R8_VendorId{
        static constexpr RegAddr NUM_ADDRESS = 0x8148;

        uint8_t id;
    };

    struct [[nodiscard]] R8_Status{
        static constexpr RegAddr ADDRES = 0x814e;

        uint8_t points_count:4;
        uint8_t :2;
        uint8_t large_detect:1;
        uint8_t buffer_status:1;
    };
};
}