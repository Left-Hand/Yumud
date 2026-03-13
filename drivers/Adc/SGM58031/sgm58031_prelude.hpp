#pragma once

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"
#include "core/math/real.hpp"

#include "hal/conn/i2c/i2cdrv.hpp"




namespace ymd::drivers{

struct SGM58031_Prelude{
    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0b0100000 >> 1);
    enum class Error_Kind{
        ChipIdMismatch
    };

    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

    template<typename T = void>
    using IResult = Result<T, Error>;

    enum class [[nodiscard]] DataRate:uint8_t{
        _6_25Hz = 0,
        _12_5Hz, 
        _25Hz, 
        _50Hz, 
        _100Hz, 
        _200Hz, 
        _400Hz, 
        _800Hz,

        _7_5Hz = 0b1000, 
        _15Hz, 
        _30Hz, 
        _60Hz, 
        _120Hz, 
        _240Hz, 
        _480Hz, 
        _960Hz
    };

    enum class [[nodiscard]] MUX:uint16_t{
        P0N1 = 0, P0N3, P1N3, P2N3, P0NG, P1NG, P2NG, P3NG
    };

    enum class [[nodiscard]] PGA:uint16_t{
        _2_3 = 0, _1, _2, _4, _8, _16
    };

    struct [[nodiscard]] FS final{
    public:
        enum Kind:uint8_t{
            _6_144 = 0, _4_096, _2_048, _1_024, _0_512, _0_256
        };

        constexpr FS(Kind kind):kind_(kind){;}
        constexpr iq16 to_real() const{
            switch(kind_){
                case FS::_0_256:
                    return iq16(0.256);
                case FS::_0_512:
                    return iq16(0.512f);
                case FS::_1_024:
                    return iq16(1.024f);
                case FS::_2_048:
                    return iq16(2.048f);
                case FS::_4_096:
                    return iq16(4.096f);
                case FS::_6_144:
                    return iq16(6.144f);
            }
            __builtin_unreachable();
        }

        [[nodiscard]] constexpr uint8_t to_u8() const {
            return uint8_t(kind_);
        }

        [[nodiscard]] constexpr auto to_pga() const {
            return static_cast<PGA>(kind_);
        }
    private:
        Kind kind_;
    };


    enum class [[nodiscard]] RegAddr:uint8_t{
        Conv = 0,
        Config,
        LowThr, 
        HighThr, 
        Config1,
        DeviceID,
        Trim
    };

    struct [[nodiscard]] ConvCode final{

        uint16_t bits;
    };
};

struct SGM58031_Regset:public SGM58031_Prelude{
    struct R16_Config:public Reg16<>{
        static constexpr auto ADDRESS = RegAddr::Config;
        static constexpr uint16_t RESET_VALUE = 0x8583;

        // Comparator Queue and Disable Function
        // 00 = Assert after one conversion
        // 01 = Assert after two conversions
        // 10 = Assert after four conversions
        // 11 = Disable comparator (default)
        uint16_t comp_que : 2;

        // Latching Comparator
        // 0 = Non-latching comparator (default)
        // 1 = Latching comparator
        uint16_t comp_lat : 1;

        // Comparator Polarity
        // 0 = Active low (default)
        // 1 = Active high
        uint16_t comp_pol : 1;

        // Comparator Mode
        // 0 = A traditional comparator with hysteresis (default)
        // 1 = A window comparator
        uint16_t comp_mode :1;
        uint16_t datarate :3;

        // Device Operating Mode
        // 0 = Continuous conversion mode
        // 1 = Power-down single-shot mode (default)
        uint16_t mode:   1;
        PGA pga:    3;
        MUX mux:    3;


        // Working Status/Single-Shot Conversion Start
        // For a write status:
        // 0 = No effect
        // 1 = Start a single conversion (when in single-shot
        // mode)
        // For a read status:
        // 0 = The chip is doing a conversion
        // 1 = The chip isn’t doing a conversion
        uint16_t os:     1;
    }DEF_R16(config_reg)

    struct R16_Config1:public Reg16<>{
        static constexpr auto ADDRESS = RegAddr::Config1;        
        uint16_t __resv1__    :3;

        // 0 = None (default)
        // 1 = Use AIN3 as external reference for ADC
        uint16_t ext_ref      :1;

        // 0 = Disable leakage blocking circuit for the scenario that I 2
        // C bus voltage is lower
        // than V DD of the part. The I 2
        // C interface is still functional but V DD sees leakage when
        // V BUS < V DD - 0.3V (default)
        // 1 = Bus voltage can be lower than V DD without causing leakage. The V DD range is
        // 3V to 5.5V and the I 2
        // C bus voltage should be limited to 3V to 5.5V
        uint16_t bus_flex     :1;
        uint16_t __resv2__   :1;

        // 0 = No current sourced (default)
        // 1 = Source a pair of 2μA current to selected pair of AINs
        uint16_t burn_out     :1;

        // 0 = DR[2:0] = 000 ~ 111 for conversion rate of 6.25Hz, 12.5Hz, 25Hz, 50Hz, 100Hz,
        // 200Hz, 400Hz and 800Hz (default)
        // 1 = DR[2:0] = 000 ~ 111 for conversion rate of 7.5Hz, 15Hz, 30Hz, 60Hz, 120Hz,
        // 240Hz, 480Hz and 960Hz
        uint16_t dr_sel       :1;

        // Writing '1' to PD powers down this part, and this PD bit is automatically cleared
        // internally. Another continuous/single conversion can be carried out again without
        // the need to clear this bit.
        uint16_t pd          :1;
        uint16_t __resv3__   :7;
    }DEF_R16(config1_reg)

    struct R16_DeviceId:public Reg16<>{
        static constexpr auto ADDRESS = RegAddr::DeviceID;
        static constexpr uint16_t KEY = 0x0080; 
        uint16_t __resv1__   :5;
        uint16_t ver         :3;
        uint16_t id          :5;
        uint16_t __resv2__   :3;
        
    };
    VALIDATE_R16(R16_DeviceId)

    struct R16_Trim:public Reg16<>{
        static constexpr auto ADDRESS = RegAddr::Trim;
        uint16_t gn         :11;
        uint16_t __resv__    :5;
        
    }DEF_R16(trim_reg)

    struct R16_Conv:public Reg16<>{
        static constexpr auto ADDRESS = RegAddr::Conv;
        ConvCode code;
    }DEF_R16(conv_reg)

    struct R16_LowThr:public Reg16<>{
        static constexpr auto ADDRESS = RegAddr::LowThr;
        ConvCode code;
    }DEF_R16(low_thr_reg)

    struct R16_HighThr:public Reg16<>{
        static constexpr auto ADDRESS = RegAddr::HighThr;
        ConvCode code;
    }DEF_R16(high_thr_reg)
};



}