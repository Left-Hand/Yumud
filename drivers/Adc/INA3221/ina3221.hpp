#pragma once


//这个驱动已经完成
//这个驱动已经测试
//这个驱动还未支持所有特性

// eg:
// auto i2c = hal::I2cSw(SCL_GPIO, SDA_GPIO);
// i2c.init(100_KHz);

// INA3221 ina = {i2c};

// ina.init().unwrap();

// while(true){
//     const auto ch = INA3221::ChannelIndex::CH1;
//     ina.update(ch).unwrap();
//     DEBUG_PRINTLN(
//         ina.get_bus_volt(ch).unwrap(), 
//         ina.get_shunt_volt(ch).unwrap() * real_t(INV_SHUNT_RES)
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
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"

#include "concept/analog_channel.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"


namespace ymd::drivers{

struct INA3221_Prelude{
public:
    enum class Error_Kind{
        WrongChipId,
        WrongManuId,
    };

    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

    template<typename T = void>
    using IResult = Result<T, Error>;

    using RegAddress = uint8_t;


    // Address Pins and Slave Addresses
    // A0   ADDRESS 0
    // GND  1000000 0
    // VS   1000001 0
    // SDA  1000010 0
    // SCL  1000011 0

    static constexpr auto DEFAULT_I2C_ADDR = 
        hal::I2cSlaveAddr<7>::from_u7(0b1000000);

    enum class ChannelIndex:uint8_t{
        CH1 = 0,
        CH2,
        CH3
    };

    enum class Source:uint16_t{
        CH1_BusBar,
        CH1_Shunt,
        CH2_BusBar,
        CH2_Shunt,
        CH3_BusBar,
        CH3_Shunt
    };

    class AverageTimes{
    public:
        enum class Kind:uint16_t{
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

        constexpr uint8_t as_raw() const {
            return static_cast<uint8_t>(kind_);}
        constexpr Kind kind() const {return kind_;}
    
    private:
        Kind kind_;
    };

    enum class ConversionTime:uint16_t{
        _140us = 0, 
        _204us, 
        _332us, 
        _588us, 
        _1_1ms, 
        _2_116_ms,
        _4_156ms, 
        _8_244ms
    };

    struct Config{
        ConversionTime shunt_conv_time;
        ConversionTime bus_conv_time;
        AverageTimes average_times;
    };



};

struct INA3221_Regs:public INA3221_Prelude {
    struct R16_Config:public Reg16<>{
        static constexpr RegAddress address = 0x00;

        uint16_t shunt_measure_en :1;
        uint16_t bus_measure_en :1;
        uint16_t continuos :1;
        ConversionTime shunt_conv_time:3;
        ConversionTime bus_conv_time:3;
        uint16_t average_times:3;
        uint16_t ch3_en:1;
        uint16_t ch2_en:1;
        uint16_t ch1_en:1;
        uint16_t rst:1;
    };

    static_assert(sizeof(R16_Config) == 2);

    static constexpr int16_t volt_to_i16(const real_t volt){
        return int16_t(volt * 100000) & 0xfff8;
    }

    struct R16_ShuntVolt:public Reg16i<>{

        int16_t : 16;

        constexpr real_t to_volt() const {
            return iq_t<24>(iq_t<16>(this->as_val() >> 3) / 25) / 1000;
            // return real_t(this->as_val());
        }

        constexpr int to_uv() const {
            return ((this->as_val() >> 3) * 40);
            // return (this->as_val());
        }

        static constexpr int16_t to_i16(const real_t volt){
            return volt_to_i16(volt);
        }
    };

    struct R16_ShuntVolt1:public R16_ShuntVolt{
        static constexpr RegAddress address = 0x01;};
    struct R16_ShuntVolt2:public R16_ShuntVolt{
        static constexpr RegAddress address = 0x03;};
    struct R16_ShuntVolt3:public R16_ShuntVolt{
        static constexpr RegAddress address = 0x05;};
    struct R16_ShuntVoltSum:public R16_ShuntVolt{
        static constexpr RegAddress address = 0x0D;};
    struct R16_ShuntVoltSumLimit:public R16_ShuntVolt{
        static constexpr RegAddress address = 0x0E;};

    struct R16_BusVolt:public Reg16i<>{

        int16_t : 16;

        constexpr real_t to_volt() const {
            return real_t((int16_t(this->as_val()) >> 3) * 8) / 1000;
        }

        constexpr int to_mv() const {
            return int16_t((int16_t(this->as_val()) >> 3) * 8);
        }

        static constexpr int16_t to_i16(const real_t volt){
            return int16_t(volt * 1000) & 0xfff8;
        }
    };

    struct R16_BusVolt1:public R16_BusVolt{
        static constexpr RegAddress address = 0x02;};
    struct R16_BusVolt2:public R16_BusVolt{
        static constexpr RegAddress address = 0x04;};
    struct R16_BusVolt3:public R16_BusVolt{
        static constexpr RegAddress address = 0x06;};



    struct R16_InstantOVC:public Reg16i<>{
        static constexpr int16_t to_i16(const real_t volt){
            return volt_to_i16(volt);
        }
        int16_t :16;
    };

    struct R16_InstantOVC1:public R16_InstantOVC{
        static constexpr RegAddress address = 0x07;};
    struct R16_InstantOVC2:public R16_InstantOVC{
        static constexpr RegAddress address = 0x09;};
    struct R16_InstantOVC3:public R16_InstantOVC{
        static constexpr RegAddress address = 0x0b;};

    struct R16_ConstantOVC:public Reg16i<>{
        static constexpr int16_t to_i16(const real_t volt){
            return volt_to_i16(volt);
        }
        int16_t :16;
    };

    struct R16_ConstantOVC1:public R16_ConstantOVC{
        static constexpr RegAddress address = 0x08;};
    struct R16_ConstantOVC2:public R16_ConstantOVC{
        static constexpr RegAddress address = 0x0A;};
    struct R16_ConstantOVC3:public R16_ConstantOVC{
        static constexpr RegAddress address = 0x0C;};

    struct R16_Mask:public Reg16<>{
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
        uint16_t :1;
    };

    struct R16_PowerHo:public Reg16i<>{
        static constexpr RegAddress address = 0x10;
        int16_t :16;
    };

    struct R16_PowerLo:public Reg16i<>{
        static constexpr RegAddress address = 0x11;
        int16_t :16;
    };

    struct R16_ManuId:public Reg16<>{
        static constexpr uint16_t key = 0x5449;
        static constexpr RegAddress address = 0xfe;
        uint16_t:16;
    };

    
    struct R16_ChipId:public Reg16<>{
        static constexpr uint16_t key = 0x3220;
        static constexpr RegAddress address = 0xff;
        uint16_t:16;
    };

    R16_Config       config_reg = {};
    R16_ShuntVolt1    shuntvolt1_reg = {};
    R16_BusVolt1      busvolt1_reg = {};
    R16_ShuntVolt2    shuntvolt2_reg = {};
    R16_BusVolt2      busvolt2_reg = {};
    R16_ShuntVolt3    shuntvolt3_reg = {};
    R16_BusVolt3      busvolt3_reg = {};
    R16_InstantOVC1   instant_ovc1_reg = {};
    R16_ConstantOVC1  constant_ovc1_reg = {};
    R16_InstantOVC2   instant_ovc2_reg = {};
    R16_ConstantOVC2  constant_ovc2_reg = {};
    R16_InstantOVC3   instant_ovc3_reg = {};
    R16_ConstantOVC3  constant_ovc3_reg = {};

    R16_ShuntVoltSum    shuntvolt_sum_reg = {};
    R16_ShuntVoltSumLimit    shuntvolt_sum_limit_reg = {};
    R16_Mask         mask_reg = {};
    R16_PowerHo      power_ho_reg = {};
    R16_PowerLo      power_lo_reg = {};

    R16_ManuId       manu_id_reg = {};
    R16_ChipId       chip_id_reg = {};
};

class INA3221_Phy final : public INA3221_Prelude{
public:
    static constexpr auto ENDIAN = MSB;

    INA3221_Phy(const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}
    
    [[nodiscard]] IResult<> read_reg(
        const RegAddress addr, uint16_t & data
    ){
        if(const auto res = i2c_drv_.read_reg((addr), data, ENDIAN);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    [[nodiscard]] IResult<> write_reg(
        const RegAddress addr, const uint16_t data
    ){
        if(const auto res = (i2c_drv_.write_reg((addr), data, ENDIAN));
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    [[nodiscard]] IResult<> read_reg(
        const RegAddress addr, int16_t & data
    ){
        if(const auto res = (i2c_drv_.read_reg((addr), data, ENDIAN));
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    [[nodiscard]] IResult<> write_reg(
        const RegAddress addr, const int16_t data
    ){
        if(const auto res = (i2c_drv_.write_reg((addr), data, ENDIAN));
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    [[nodiscard]] IResult<> read_burst(
        const RegAddress addr, std::span<uint16_t> pbuf){
        if(const auto res = i2c_drv_.read_burst(uint8_t(addr), pbuf, ENDIAN);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }
private:
    hal::I2cDrv i2c_drv_;
};

class INA3221 final:
    public INA3221_Regs{
public:
    struct INA3221Channel:public hal::AnalogInIntf{
    public:
        enum class ChannelType:uint8_t{
            Shunt,
            BusBar,
        };

    protected:
        INA3221 & parent_;
        ChannelType ch_;
        ChannelIndex idx_;
    public:
        INA3221Channel(INA3221 & parent, const ChannelType ch, const ChannelIndex idx):
                parent_(parent), ch_(ch), idx_(idx){}

        INA3221Channel(const INA3221Channel & other) = delete;
        INA3221Channel(INA3221Channel && other) = delete;
        operator real_t() override{
            return get_volt().unwrap();
        }

        IResult<real_t> get_volt(){
            switch(ch_){
                case ChannelType::Shunt:
                    return parent_.get_shunt_volt(idx_);
                case ChannelType::BusBar:
                    return parent_.get_bus_volt(idx_);
                default: __builtin_unreachable();
            }
        }
    };
public:
    static constexpr auto DEFAULT_CONFIG = Config{
        .shunt_conv_time = ConversionTime::_140us, 
        .bus_conv_time = ConversionTime::_140us, 
        .average_times = AverageTimes::_1
    };

    INA3221(const hal::I2cDrv & i2c_drv):phy_(i2c_drv){;}
    INA3221(hal::I2cDrv && i2c_drv):phy_(std::move(i2c_drv)){;}
    INA3221(hal::I2c & i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        phy_(hal::I2cDrv(i2c, addr)){;}
    ~INA3221(){;}
    
    [[nodiscard]] IResult<bool> is_ready();


    [[nodiscard]] IResult<> init(const Config & cfg = DEFAULT_CONFIG);
    [[nodiscard]] IResult<> reconf(const Config & cfg);
    [[nodiscard]] IResult<> update();
    [[nodiscard]] IResult<> update(const ChannelIndex index);
    [[nodiscard]] IResult<> validate();
    [[nodiscard]] IResult<> reset();
    [[nodiscard]] IResult<> set_average_times(const AverageTimes times);
    [[nodiscard]] IResult<> enable_measure_bus(const Enable en = EN);
    [[nodiscard]] IResult<> enable_measure_shunt(const Enable en = EN);
    [[nodiscard]] IResult<> enable_continuous(const Enable en = EN);

    [[nodiscard]] IResult<> enable_channel(const ChannelIndex index, const Enable en = EN);

    [[nodiscard]] IResult<> set_bus_conversion_time(const ConversionTime time);
    [[nodiscard]] IResult<> set_shunt_conversion_time(const ConversionTime time);

    [[nodiscard]] IResult<int> get_shunt_volt_uv(const ChannelIndex index);
    [[nodiscard]] IResult<int> get_bus_volt_mv(const ChannelIndex index);

    [[nodiscard]] IResult<real_t> get_shunt_volt(const ChannelIndex index);
    [[nodiscard]] IResult<real_t> get_bus_volt(const ChannelIndex index);

    [[nodiscard]] IResult<> set_instant_ovc(const ChannelIndex index, const real_t volt);
    [[nodiscard]] IResult<> set_constant_ovc(const ChannelIndex index, const real_t volt);
private:
    INA3221_Phy phy_;


    template<typename T>
    [[nodiscard]] IResult<> write_reg(const RegCopy<T> & reg){
        if(const auto res = phy_.write_reg(reg.address, reg.as_val());
            res.is_err()) return res;
        reg.apply();
        return Ok();
    }

    template<typename T>
    [[nodiscard]] IResult<> read_reg(T & reg){
        return phy_.read_reg(reg.address, reg.as_ref());
    }

    [[nodiscard]] IResult<> read_reg(const RegAddress addr, auto & data){
        return phy_.read_reg(addr, data);
    }

    [[nodiscard]] IResult<> write_reg(const RegAddress addr, const auto data){
        return phy_.write_reg(addr, data);
    }


    std::array<INA3221Channel, 6> channels = {
        INA3221Channel{*this, INA3221Channel::ChannelType::BusBar,    ChannelIndex::CH1},
        INA3221Channel{*this, INA3221Channel::ChannelType::Shunt,  ChannelIndex::CH1},
        INA3221Channel{*this, INA3221Channel::ChannelType::BusBar,    ChannelIndex::CH2},
        INA3221Channel{*this, INA3221Channel::ChannelType::Shunt,  ChannelIndex::CH2},
        INA3221Channel{*this, INA3221Channel::ChannelType::BusBar,    ChannelIndex::CH3},
        INA3221Channel{*this, INA3221Channel::ChannelType::Shunt,  ChannelIndex::CH3},
    };
};

}