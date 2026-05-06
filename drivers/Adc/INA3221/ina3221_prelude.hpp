#pragma once


//这个驱动已经完成
//这个驱动已经测试
//这个驱动还未支持所有特性

// eg:
// INA3221 ina = {&i2c};

// ina.init().unwrap();

// while(true){
//     ina.update(INA3221::ChannelSelection::CH1).unwrap();
//     DEBUG_PRINTLN(
//         ina.get_bus_volt(ch).unwrap(), 
//         ina.get_shunt_volt(ch).unwrap() * iq16(INV_SHUNT_RES)
//     );
//     clock::delay(2ms);
// }



// 注意：
// 1. INA3221不支持地址自增模式，每次对寄存器的操作都是单独的

// INA3221 是一款三通道、高侧电流和总线电压监视
// 器，具有一个兼容I2C和SMBUS的接口。INA3221
// 不仅能够监视分流压降和总线电源电压，还针对这些信
// 号提供有可编程的转换时间和平均值计算模式。
// INA3221 提供关键报警和警告报警，用于检测每条通
// 道上可编程的多种超范围情况。


#include "core/io/regs.hpp"
#include "core/math/realmath.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"



#include "hal/conn/i2c/i2cdrv.hpp"


namespace ymd::drivers{

struct INA3221_Prelude{
public:
    enum class Error_Kind:uint8_t{
        ChipIdMismatch,
        ManuIdMisMatch,
    };

    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)
    DEF_FRIEND_DERIVE_DEBUG(Error_Kind)

    template<typename T = void>
    using IResult = Result<T, Error>;

    using RegAddr = uint8_t;


    // Address Pins and Slave Addresses
    // A0   ADDRESS 0
    // GND  1000000 0
    // VS   1000001 0
    // SDA  1000010 0
    // SCL  1000011 0

    static constexpr auto DEFAULT_I2C_ADDR = 
        hal::I2cSlaveAddr<7>::from_u7(0b1000000);

    enum class [[nodiscard]] ChannelSelection:uint8_t{
        CH1 = 0,
        CH2,
        CH3
    };

    enum class [[nodiscard]] Source:uint16_t{
        CH1_BusBar,
        CH1_Shunt,
        CH2_BusBar,
        CH2_Shunt,
        CH3_BusBar,
        CH3_Shunt
    };

    struct [[nodiscard]] AverageTimes final{
    public:
        enum class [[nodiscard]] Kind:uint16_t{
            _1 = 0,
            _4 = 1,
            _16 = 2,
            _64 = 3,
            _128 = 4,
            _256 = 5,
            _512 = 6,
            _1024 = 7
        };

        using enum Kind;

        constexpr AverageTimes(Kind kind):kind_(kind){;}

        static constexpr AverageTimes from_times(uint8_t times){
            uint8_t temp = CTZ(times);
            uint8_t temp2;
        
            if(times <= 64){
                temp2 = temp >> 1;
            }else{
                temp2 = 4 + (temp - 7); 
            }
            return {static_cast<Kind>(temp2)};
        }

        constexpr uint8_t to_bits() const noexcept {
            return static_cast<uint8_t>(kind_);}
        constexpr Kind kind() const noexcept {return kind_;}
    
    private:
        Kind kind_;
    };

    enum class [[nodiscard]] ConversionTime:uint16_t{
        _140us = 0, 
        _204us, 
        _332us, 
        _588us, 
        _1_1ms, 
        _2_116_ms,
        _4_156ms, 
        _8_244ms
    };

    struct [[nodiscard]] Config final{
        ConversionTime shunt_conv_time;
        ConversionTime bus_conv_time;
        AverageTimes average_times;

        static constexpr Config from_default() {
            return Config{
                .shunt_conv_time = ConversionTime::_140us, 
                .bus_conv_time = ConversionTime::_140us, 
                .average_times = AverageTimes::_1
            };
        }
    };




    struct [[nodiscard]] ShuntVoltCode final{
        using Self = ShuntVoltCode;

        uint16_t bits;

        static constexpr Self from_mv(const int32_t mv){
            return Self{.bits = mv_to_sv_code(mv)};
        }

        [[nodiscard]] constexpr int32_t to_mv() const noexcept { 
            return sv_code_to_mv(bits); 
        }

        [[nodiscard]] constexpr int32_t to_uv() const noexcept { 
            return sv_code_to_uv(bits); 
        }

        [[nodiscard]] constexpr iq16 to_volts() const noexcept { 
            return sv_code_to_volts(bits); 
        }

    private:
        [[nodiscard]] static constexpr uint16_t mv_to_sv_code(const int32_t mv){
            uint16_t bits = uint16_t(mv * (1000 / 40)) << 3;
            return bits;
        }

        [[nodiscard]] static constexpr int32_t sv_code_to_mv(const uint16_t bits){
            constexpr uint32_t RATIO = static_cast<uint32_t>((1ull << 32) * 40 / (1000));
            const int16_t signed_bits = static_cast<int16_t>(bits);
            return static_cast<int32_t>(static_cast<int64_t>(signed_bits >> 3) * RATIO >> 32);
        }

        [[nodiscard]] static constexpr int32_t sv_code_to_uv(const uint16_t bits){
            const int16_t signed_bits = static_cast<int16_t>(bits);
            return static_cast<int32_t>(signed_bits >> 3) * 40;
        }

        [[nodiscard]] static constexpr iq16 sv_code_to_volts(const uint16_t bits){
            constexpr uint64_t RATIO = static_cast<uint64_t>((1ull << 48) * 40 / (1000000));
            const int16_t signed_bits = static_cast<int16_t>(bits);
            return iq16::from_bits(static_cast<int32_t>(static_cast<int64_t>(signed_bits >> 3) * RATIO >> 32));
        }
    };

    struct [[nodiscard]] ShuntVoltSumCode final{
        using Self = ShuntVoltSumCode;

        uint16_t bits;

        static constexpr Self from_mv(const int32_t mv){
            return Self{.bits = mv_to_svsum_code(mv)};
        }

        [[nodiscard]] constexpr int32_t to_mv() const noexcept { 
            return svsum_code_to_mv(bits); 
        }

        [[nodiscard]] constexpr int32_t to_uv() const noexcept { 
            return svsum_code_to_uv(bits); 
        }

        [[nodiscard]] constexpr iq16 to_volts() const noexcept { 
            return svsum_code_to_volts(bits); 
        }
    private:
        [[nodiscard]] static constexpr uint16_t mv_to_svsum_code(const int32_t mv){
            uint16_t bits = uint16_t(mv * (1000 / 40)) << 1;
            return bits;
        }

        [[nodiscard]] static constexpr int32_t svsum_code_to_mv(const uint16_t bits){
            constexpr uint32_t RATIO = static_cast<uint32_t>((1ull << 32) * 40 / (1000));
            const int16_t signed_bits = static_cast<int16_t>(bits);
            return static_cast<int32_t>((static_cast<int64_t>(signed_bits >> 1) * RATIO) >> 32);
        }

        [[nodiscard]] static constexpr iq16 svsum_code_to_volts(const uint16_t bits){
            constexpr uint32_t RATIO = static_cast<uint32_t>((1ull << 48) * 40 / (1000000));
            const int16_t signed_bits = static_cast<int16_t>(bits);
            return iq16::from_bits(static_cast<int32_t>((static_cast<int64_t>(signed_bits >> 1) * RATIO) >> 32));
        }

        [[nodiscard]] static constexpr int32_t svsum_code_to_uv(const uint16_t bits){
            const int16_t signed_bits = static_cast<int16_t>(bits);
            return static_cast<int32_t>(signed_bits >> 1) * 40;
        }
    };


    struct [[nodiscard]] BusVoltCode final{
        //lsb 8mv
        using Self = BusVoltCode;
        uint16_t bits;

        static constexpr Self from_mv(const int32_t mv){
            return Self{.bits = mv_to_bv_code(mv)};
        }

        [[nodiscard]] constexpr int32_t to_mv() const noexcept { 
            return bv_code_to_mv(bits); 
        }

        [[nodiscard]] constexpr iq16 to_volts() const noexcept { 
            return bv_code_to_volts(bits); 
        }

    private:
        [[nodiscard]] static constexpr uint16_t mv_to_bv_code(const int32_t mv){ 
            uint16_t bits = uint16_t(mv & (~0x07));
            return bits;
        }

        [[nodiscard]] static constexpr int32_t bv_code_to_mv(const uint16_t bits){ 
            const int16_t signed_bits = static_cast<int16_t>(bits);
            return static_cast<int32_t>(signed_bits & (~0x07));
        }

        [[nodiscard]] static constexpr iq16 bv_code_to_volts(const uint16_t bits){ 
            constexpr uint64_t RATIO = static_cast<uint64_t>((1ull << 48) / (1000));
            const int16_t signed_bits = static_cast<int16_t>(bits);
            // return iq16(static_cast<int32_t>(signed_bits & (~0x07))) / 1000;
            return iq16::from_bits((static_cast<int64_t>(signed_bits & (~0x07)) * RATIO) >> 32);
        }

    };


    struct [[nodiscard]] MaskCode final{
        uint16_t conv_ready:1;
        uint16_t timing_alert:1;
        uint16_t power_valid_alert:1;
        uint16_t constant_alert1:1;
        uint16_t constant_alert2:1;
        uint16_t constant_alert3:1;
        uint16_t sum_alert:1;
        uint16_t instant_alert1:1;
        uint16_t instant_alert2:1;
        uint16_t instant_alert3:1;
        uint16_t instant_en:1;
        uint16_t constant_en:1;
        uint16_t sum_en:3;
        uint16_t __resv__:1;
    };

};

struct INA3221_Regs:public INA3221_Prelude {
    struct [[nodiscard]] R16_Config:public Reg16<>{
        static constexpr RegAddr REG_ADDR = RegAddr{0x00};
        static constexpr uint16_t RESET_VALUE = 0x7127;

        uint16_t shunt_measure_en :1;
        uint16_t bus_measure_en :1;
        uint16_t continuous_en :1;
        ConversionTime shunt_conv_time:3;
        ConversionTime bus_conv_time:3;
        AverageTimes::Kind average_times:3;
        uint16_t ch3_en:1;
        uint16_t ch2_en:1;
        uint16_t ch1_en:1;

        // Reset bit. Set this bit = 1 to generate a system reset that is the
        // same as a power-on reset (POR). This bit resets all registers to
        // default values and self-clears.
        uint16_t rst:1;
    };

    struct [[nodiscard]] R16_ShuntVolt1: public Reg16<>{
        static constexpr RegAddr REG_ADDR = RegAddr{0x01};
        static constexpr uint16_t RESET_VALUE = 0x0000;

        ShuntVoltCode code;
    };

    struct [[nodiscard]] R16_BusVolt1:public Reg16<>{
        static constexpr RegAddr REG_ADDR = RegAddr{0x02};
        static constexpr uint16_t RESET_VALUE = 0x0000;

        BusVoltCode code;
    };

    struct [[nodiscard]] R16_ShuntVolt2: public Reg16<>{
        static constexpr RegAddr REG_ADDR = RegAddr{0x03};
        static constexpr uint16_t RESET_VALUE = 0x0000;

        ShuntVoltCode code;
    };


    struct [[nodiscard]] R16_BusVolt2:public Reg16<>{
        static constexpr RegAddr REG_ADDR = RegAddr{0x04};
        static constexpr uint16_t RESET_VALUE = 0x0000;

        BusVoltCode code;
    };

    struct [[nodiscard]] R16_ShuntVolt3: public Reg16<>{
        static constexpr RegAddr REG_ADDR = RegAddr{0x05};
        static constexpr uint16_t RESET_VALUE = 0x0000;

        ShuntVoltCode code;
    };

    struct [[nodiscard]] R16_BusVolt3:public Reg16<>{
        static constexpr RegAddr REG_ADDR = RegAddr{0x06};
        static constexpr uint16_t RESET_VALUE = 0x0000;

        BusVoltCode code;
    };

    //0x07
    struct [[nodiscard]] R16_InstantOVC1:public Reg16<>{
        static constexpr RegAddr REG_ADDR = RegAddr{0x07};
        static constexpr uint16_t RESET_VALUE = 0x7ff8;

        ShuntVoltCode code;
    };

    //0x08
    struct [[nodiscard]] R16_ConstantOVC1:public Reg16<>{
        static constexpr RegAddr REG_ADDR = RegAddr{0x08};
        static constexpr uint16_t RESET_VALUE = 0x7ff8;

        ShuntVoltCode code;
    };


    //0x09
    struct [[nodiscard]] R16_InstantOVC2:public Reg16<>{
        static constexpr RegAddr REG_ADDR = RegAddr{0x09};
        static constexpr uint16_t RESET_VALUE = 0x7ff8;

        ShuntVoltCode code;
    };

    //0x0a
    struct [[nodiscard]] R16_ConstantOVC2:public Reg16<>{
        static constexpr RegAddr REG_ADDR = RegAddr{0x0A};
        static constexpr uint16_t RESET_VALUE = 0x7ff8;

        ShuntVoltCode code;
    };

    //0x0b
    struct [[nodiscard]] R16_InstantOVC3:public Reg16<>{
        static constexpr RegAddr REG_ADDR = RegAddr{0x0b};
        static constexpr uint16_t RESET_VALUE = 0x7ff8;

        ShuntVoltCode code;
    };


    //0x0c
    struct [[nodiscard]] R16_ConstantOVC3:public Reg16<>{
        static constexpr RegAddr REG_ADDR = RegAddr{0x0C};
        static constexpr uint16_t RESET_VALUE = 0x7ff8;

        ShuntVoltCode code;
    };


    //0x0d
    struct [[nodiscard]] R16_ShuntVoltSum: public Reg16<>{
        static constexpr RegAddr REG_ADDR = RegAddr{0x0D};
        static constexpr uint16_t RESET_VALUE = 0x0000;

        ShuntVoltSumCode code;
    };

    //0x0e
    struct [[nodiscard]] R16_ShuntVoltSumLimit: public Reg16<>{
        static constexpr RegAddr REG_ADDR = RegAddr{0x0E};
        static constexpr uint16_t RESET_VALUE = 0x7ffe;

        ShuntVoltSumCode code;
    };


    //0x0f
    struct [[nodiscard]] R16_Mask:public Reg16<>{
        static constexpr RegAddr REG_ADDR = RegAddr{0x0f};
        static constexpr uint16_t RESET_VALUE = 0x0002;

        MaskCode code;

    };



    // This register contains the value used to determine if the power-valid conditions are met. The power-valid
    // condition is reached when all bus-voltage channels exceed the value set in this limit register. When the power-
    // valid condition is met, the PV alert pin asserts high to indicate that the INA3221 has confirmed all bus voltage
    // channels are above the power-valid upper-limit value. In order for the power-valid conditions to be monitored, the
    // bus measurements must be enabled through one of the corresponding MODE bits set in the Configuration
    // register. The power-valid upper-limit LSB value is 8 mV. Power-on reset value is 2710h = 10.000 V.
    struct [[nodiscard]] R16_PowerValidUp:public Reg16<>{
        static constexpr RegAddr REG_ADDR = RegAddr{0x10};
        static constexpr uint16_t RESET_VALUE = 0x2710;
        BusVoltCode code;
    };


    // This register contains the value used to determine if any of the bus-voltage channels drops below the power-valid
    // lower-limit when the power-valid conditions are met. This limit contains the value used to compare all bus-
    // channel readings to make sure that all channels remain above the power-valid lower-limit, thus maintaining the
    // power-valid condition. If any bus-voltage channel drops below the power-valid lower-limit, the PV alert pin pulls
    // low to indicate that the INA3221 detects a bus voltage reading below the power-valid lower-limit. In order for the
    // power-valid condition to be monitored, the bus measurements must be enabled through the mode (MODE3-1)
    // bits set in the Configuration register. The power-valid lower-limit LSB value is 8 mV. Power-on reset value is
    // 2328h = 9.000 V
    struct [[nodiscard]] R16_PowerValidLo:public Reg16<>{
        static constexpr RegAddr REG_ADDR = RegAddr{0x11};
        static constexpr uint16_t RESET_VALUE = 0x2328;
        BusVoltCode code;
    };

    struct [[nodiscard]] R16_ManuId:public Reg16<>{
        static constexpr RegAddr REG_ADDR = RegAddr{0xfe};
        static constexpr uint16_t KEY = 0x5449;
        uint16_t bits;
    };

    
    struct [[nodiscard]] R16_ChipId:public Reg16<>{
        static constexpr RegAddr REG_ADDR = RegAddr{0xff};
        static constexpr uint16_t KEY = 0x3220;
        uint16_t bits;
    };

    VALIDATE_R16(R16_Config)
    VALIDATE_R16(R16_ShuntVolt1)
    VALIDATE_R16(R16_BusVolt1)
    VALIDATE_R16(R16_ShuntVolt2)
    VALIDATE_R16(R16_BusVolt2)
    VALIDATE_R16(R16_ShuntVolt3)
    VALIDATE_R16(R16_BusVolt3)
    VALIDATE_R16(R16_InstantOVC1)
    VALIDATE_R16(R16_ConstantOVC1)
    VALIDATE_R16(R16_InstantOVC2)
    VALIDATE_R16(R16_ConstantOVC2)
    VALIDATE_R16(R16_InstantOVC3)
    VALIDATE_R16(R16_ConstantOVC3)
    VALIDATE_R16(R16_ShuntVoltSum)
    VALIDATE_R16(R16_ShuntVoltSumLimit)
    VALIDATE_R16(R16_Mask)
    VALIDATE_R16(R16_PowerValidUp)
    VALIDATE_R16(R16_PowerValidLo)
    VALIDATE_R16(R16_ManuId)
    VALIDATE_R16(R16_ChipId)

    R16_Config       config_reg = {};
    R16_ShuntVolt1    shuntvolt1_reg = {};
    R16_BusVolt1      busvolt1_reg = {};
    R16_ShuntVolt2    shuntvolt2_reg = {};
    R16_BusVolt2      busvolt2_reg = {};
    R16_ShuntVolt3    shuntvolt3_reg = {};
    R16_BusVolt3      busvolt3_reg = {};

    R16_ShuntVoltSum    shuntvolt_sum_reg = {};
    R16_ShuntVoltSumLimit    shuntvolt_sum_limit_reg = {};
    R16_Mask         mask_reg = {};
    R16_PowerValidUp      power_ho_reg = {};
    R16_PowerValidLo      power_lo_reg = {};

};

class INA3221_Transport final : public INA3221_Prelude{
public:
    static constexpr auto ENDIAN = std::endian::big;

    explicit INA3221_Transport(const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}
    
    IResult<> read_reg(
        const RegAddr addr, uint16_t & data
    ){
        if(const auto res = i2c_drv_.read_reg((addr), data, ENDIAN);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    IResult<> write_reg(
        const RegAddr addr, const uint16_t data
    ){
        if(const auto res = (i2c_drv_.write_reg((addr), data, ENDIAN));
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    IResult<> read_reg(
        const RegAddr addr, int16_t & data
    ){
        if(const auto res = (i2c_drv_.read_reg((addr), data, ENDIAN));
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    IResult<> write_reg(
        const RegAddr addr, const int16_t data
    ){
        if(const auto res = (i2c_drv_.write_reg((addr), data, ENDIAN));
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    IResult<> read_bulk(
        const RegAddr addr, std::span<uint16_t> pbuf){
        if(const auto res = i2c_drv_.read_bulk(uint8_t(addr), pbuf, ENDIAN);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }
private:
    hal::I2cDrv i2c_drv_;
};


}