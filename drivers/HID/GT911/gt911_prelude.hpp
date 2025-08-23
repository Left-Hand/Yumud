#pragma once

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"

#include "types/vectors/vector2.hpp"
#include "core/utils/nth.hpp"

namespace ymd::drivers{


struct Gt911_Prelude{
protected:
    using RegAddr = uint16_t;
    static constexpr uint8_t GT911_I2C_ADDR_BA  = 0x5D;
    static constexpr RegAddr GT911_PRODUCT_ID_REG  = 0x8140;
    static constexpr RegAddr GT911_TOUCHPOINT_STATUS_REG  = 0x814E;
    static constexpr RegAddr GT911_TOUCHPOINT_1_REG  = 0x814F;
    static constexpr RegAddr GT911_COMMAND_REG  = 0x8040;

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

    struct TouchPoint{
        uint8_t track_id;
        uint16_t x;
        uint16_t y;
        uint16_t area;
        // uint8_t __resv__;
    };

    static constexpr size_t TOUCHPOINT_ENTRY_LEN  = sizeof(TouchPoint);
    static_assert(TOUCHPOINT_ENTRY_LEN == 8);

    static constexpr size_t GET_TOUCH_BUF_SIZE = TOUCHPOINT_ENTRY_LEN;
    static constexpr size_t GET_MULTITOUCH_BUF_SIZE = TOUCHPOINT_ENTRY_LEN * MAX_NUM_TOUCHPOINTS;

    enum class ErrorKind:uint8_t{
        UnexpectedProductId,
        UnexpectedData,
        NotReady
    };

    DEF_ERROR_SUMWITH_HALERROR(Error, ErrorKind)

protected:
    static constexpr TouchPoint decode_point(
        const std::span<const uint8_t, TOUCHPOINT_ENTRY_LEN> pbuf
    ) {

        Gt911_Prelude::TouchPoint point;
        point.track_id = pbuf[0];
        point.x = static_cast<uint16_t>(pbuf[1] | (pbuf[2] << 8));  // Little endian
        point.y = static_cast<uint16_t>(pbuf[3] | (pbuf[4] << 8));  // Little endian
        point.area = static_cast<uint16_t>(pbuf[5] | (pbuf[6] << 8));  // Little endian
        return point;
    }

    static constexpr TouchPoint map_buf_to_point(
        const std::span<const uint8_t, GET_MULTITOUCH_BUF_SIZE> pbuf, 
        const Nth nth
    ){
        const auto start = nth.count() * TOUCHPOINT_ENTRY_LEN;
        return decode_point(std::span<const uint8_t, TOUCHPOINT_ENTRY_LEN>(
            pbuf.data() + start, TOUCHPOINT_ENTRY_LEN));
    }

    static constexpr RegAddr map_nth_to_addr(
        const Nth nth
    ){
        return GT911_TOUCHPOINT_1_REG + nth.count() * TOUCHPOINT_ENTRY_LEN;
    }

    enum class WorkMode:uint8_t{
        Normal,
        LowPower,
        Sleep
    };

    enum class InterruptTriggerMethod:uint8-t{
        RisingEdge,
        FallingEdge,
        LowLevel,
        HighLevel
    };
};


struct Gt911_Regs:public Gt911_Prelude{
    struct R32_ProductId{
        static constexpr RegAddr ADDRESS = 0x8140;
        uint8_t id[4];

        constexpr bool is_valid() const {
            return id[0] == '9' && id[1] == '1' && id[2] == '1' && id[3] == '\0';
        }
    };

    struct R16_Firmware{
        static constexpr RegAddr ADDRESS = 0x8144;

        uint8_t low;
        uint8_t high;
    };

    struct R32_Resolution{
        static constexpr RegAddr ADDRESS = 0x8146;

        uint16_t x;
        uint16_t y;
    };

    struct VendorId{
        static constexpr RegAddr ADDRESS = 0x8148;

        uint8_t id;
    };

    struct Status{
        static constexpr RegAddr ADDRES = 0x814e;

        uint8_t points_count:4;
        uint8_t :2;
        uint8_t large_detect:1;
        uint8_t buffer_status:1;
    };
};
}