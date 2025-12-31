#pragma once

#include "core/math/real.hpp"


#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"

#include "hal/bus/spi/spidrv.hpp"

namespace ymd::drivers{

struct GP22_Prelude{
    enum class Error_Kind{
        MeasurementNotDone
    };

    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

    template<typename T = void>
    using IResult = Result<T, Error>;

    using RegAddr = uint8_t;

    enum class TSTO1:uint32_t{
        // 0 = GP2 functionality, FIRE_IN input for 
        // sing-around
        // 1 = START_TDC output
        // 2 = STOP1 TDC output
        // 3 = STOP2 TDC output
        // 4 = Start Temperature measurement 
        // output
        // 5 = EN_STOP by DELVAL output
        // 6 = Comparator out
        // 7 = 32 kHz clock
        Gp2Func = 0,
        StartTdc,
        Stop1Tdc,
        Stop2Tdc,
        StartTemp,
        EnStopByDelval,
        ComparatorOut,
        Clock32k
    };

    enum class TSTO2:uint32_t{
        // 0 = GP2 functionality, High level enables 
        // the START pin.
        // 1 = START_TDC output
        // 2 = STOP1 TDC output
        // 3 = STOP2 TDC output
        // 4 = Stop Temperature measurement 
        // output
        // 5 = “0” indicates TOF_DOWN being active, 
        // “1” indicates TOF_UP being active
        // 6 = n.c.
        // 7 = 4 kHz (32 kHz/8) clock
        TstOut1 = 0,
        StartTdc,
        Stop1Tdc,
        Stop2Tdc,
        StopTemp,
        ToFDown,
        Nc,
        Clock4k
    };
};

struct GP22_Regs final:public GP22_Prelude{


    enum class MeasurementMode:uint32_t {
        MeasurementMode1 = 0,
        MeasurementMode2 = 1
    };
    using u32 = uint32_t;

    struct R32_Cfg0:public Reg32<>{
        static constexpr RegAddr ADDRESS = 0x1;

        u32 id0:8;
        u32 neg_stop1:1;
        u32 neg_stop2:1;
        u32 messb2:1;
        u32 no_cal_auto:1;
        u32 calibrate:1;
        u32 sel_eclk_temp:1;
        u32 anz_fake:1;
        u32 tcycle:1;
        u32 anz_port:1;
        u32 start_clkhs:2;
        u32 div_clkhs:2;
        u32 anz_per_calres:2;
        u32 div_fire:4;
        u32 anz_fire:5;
    }DEF_R32(cfg0_reg)

    struct R32_Cfg1:public Reg32<>{
        static constexpr RegAddr ADDRESS = 0x1;

        uint32_t id1:8;

        TSTO1 sel_tsto1:3;
        TSTO2 sel_tsto2:3;
        uint32_t sel_start_fire:1;
        uint32_t curr_32k:1;

        uint32_t hitin1:3;
        uint32_t hitin2:3;
        uint32_t k_d:1;
        uint32_t en_fast_int:1;
        
        uint32_t hit1:4;
        uint32_t hit2:4;
    }DEF_R32(cfg1_reg)
};

class GP22_Transport final:
    private GP22_Prelude{
public:
        // TDC硬件初始化
    IResult<> reset() {
        if(may_nrst_pin_.is_none()) return Ok();
        auto & rstn_gpio = may_nrst_pin_.unwrap();
        rstn_gpio.set_high();
        clock::delay(1us);
        rstn_gpio.set_low();
        clock::delay(1us);
        rstn_gpio.set_high();
        clock::delay(1ms);
        return Ok();
    }

    [[nodiscard]] IResult<> write_u8(const uint8_t data);
    [[nodiscard]] IResult<uint8_t> transrecive_u8(const uint8_t data);
    [[nodiscard]] IResult<> write_u32(const uint32_t data);
    [[nodiscard]] IResult<uint32_t> trans_u8_receive_u32(const uint8_t data);
private:
    Option<hal::GpioIntf &> may_nrst_pin_;
};
}