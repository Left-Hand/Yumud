#pragma once



#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "primitive/arithmetic/angular.hpp"

#include "hal/bus/spi/spidrv.hpp"


#include "drivers/Encoder/MagEncoder.hpp"


namespace ymd::drivers{
namespace vce2755{
// 对应的 CRC 生成多项式为 X4+X+1，初始值=0000b，数据输入输出不取反。
static constexpr uint8_t calc_crc4(uint32_t bits20) {
    // 确保只使用低20位
    bits20 &= 0x000FFFFF;
    
    // CRC 寄存器，初始值为 0000 (4位)
    uint8_t crc = 0x00;
    
    // 处理20个数据位，从最高位开始
    for (int i = 19; i >= 0; i--) {
        // 获取当前数据位
        bool data_bit = (bits20 >> i) & 0x01;
        
        // 计算反馈位：CRC最高位(bit3)与数据位异或
        bool feedback = ((crc >> 3) & 0x01) ^ data_bit;
        
        // CRC左移1位
        crc = (crc << 1) & 0x0F;
        
        // 如果反馈位为1，则与多项式异或
        if (feedback) {
            // 多项式 X^4 + X + 1 对应的二进制: 10011
            // 去掉最高位后为: 0011 (0x03)
            crc ^= 0x03;
        }
    }
    
    return crc & 0x0F;  // 确保只返回低4位
}
static_assert(calc_crc4(0x12345) == 0x08);
static_assert(calc_crc4(0x00) == 0x00);
static_assert(calc_crc4(0x00001) == 0x03);



}
struct VCE2755_Prelude{
    using Error = EncoderError;

    template<typename T = void>
    using IResult = Result<T, Error>;

    enum class [[nodiscard]] Package:uint8_t{
        SOIC8 = 0x5A,
        TQFN3 = 0x5B
    };

    struct [[nodiscard]] PackageCode final{
        static constexpr uint8_t KEY1 = static_cast<uint8_t>(Package::SOIC8);
        static constexpr uint8_t KEY2 = static_cast<uint8_t>(Package::TQFN3);

        uint8_t bits;

        [[nodiscard]] constexpr bool is_valid() const {
            return bits == KEY1 || bits == KEY2;
        }

        [[nodiscard]] constexpr Option<Package> try_into_package() const {
            if(not is_valid()) return None;
            return Some(into_package());
        }
        
        [[nodiscard]] Package into_package() const {
            return static_cast<Package>(bits);
        }

        friend OutputStream & operator<<(OutputStream & os, const PackageCode & self){ 
            if(self.is_valid()) return os << self.into_package();
            return os << "Unknown" << os.brackets<'('>() << self.bits << os.brackets<')'>();
        }
    };

    enum class [[nodiscard]] IoDriveStrength:uint8_t{
        _2mA = 0b00,
        _4mA = 0b01,
    };

    enum class [[nodiscard]] PwmFreq:uint8_t{
        _971_1Hz,_485_6Hz
    };


    enum class [[nodiscard]] Mux:uint8_t{
        _1,
        _2,
        _3,
        //QFN only
        _4,

        TQFN_ABZ_PWM_SSI_SPI = _1,
        TQFN_UVW_PWM_SSI_SPI = _2,
        TQFN_ABZ_UVW_SPI = _3,
        TQFN_ABZ_SPI = _4,

        SOIC_ABZ_SPI = _1,
        SOIC_UVW_SPI = _2,
        SOIC_PWM_SPI = _3,
    };

    enum class [[nodiscard]] Hysteresis:uint8_t{
        _0deg = 0b000,
        _0_011deg = 0b001,
        _0_022deg = 0b010,
        _0_044deg = 0b011,
        _0_066deg = 0b100,
        _0_088deg = 0b101,
        _0_132deg = 0b110,
        _0_176deg = 0b111,
    };

    enum class [[nodiscard]] ZeroPulseWidth:uint8_t{
        _1lsb   = 0b000,
        _2lsb   = 0b001,
        _4lsb   = 0b010,
        _8lsb   = 0b011,
        _12lsb  = 0b100,
        _16lsb  = 0b101,
        _180deg = 0b110,
    };

    enum class [[nodiscard]] BandWidth:uint8_t{ 
        _8BW0 = 0b011001,
        _4BW0 = 0b100011,
        _2BW0 = 0b101101,
        _BW0  = 0b110111,
    };

    enum class [[nodiscard]] WeakMagAlarmLevel:uint8_t{
        OneEighth,
        Quarter,
        Half,
        Disabled
    };


    enum class [[nodiscard]] AbzPowerOnWaveform:uint8_t{
        // 00 脉冲序列 1（上电期间的标准脉冲输出）


        // 01 脉冲序列 2（上电期间 Z1 脉冲从低拉高持续 5ms 后，AB 输
        // 出上电初始位置的绝对角度脉冲信号）


        // 11 脉冲序列 3（上电期间 Z1 脉冲从低拉高持续 10ms 后，AB
        // 输出上电初始位置的绝对角度脉冲信号
    };



    using RegAddr = uint8_t;
};

struct VCE2755_Regset:public VCE2755_Prelude{
    //0x00
    struct [[nodiscard]] R8_ChipId:public Reg8<>{
        static constexpr RegAddr ADDRESS = RegAddr{0x00};
        PackageCode code;
    };

    struct [[nodiscard]] AnglePacket final{
        static constexpr RegAddr ADDRESS = RegAddr{0x03};
        union{
            struct {
                uint8_t angle_17_10;
                uint8_t angle_9_2;

                // CRC0～CRC3 为 4bitCRC，系 ANGLE+SMF+BTE 共 20bit 数据的 CRC 校验值，
                // 对应的 CRC 生成多项式为 X4+X+1，初始值=0000b，数据输入输出不取反。
                uint8_t crc_3_0:4;

                //内部定义标志
                uint8_t bte:1;
                uint8_t mag_weak:1;
                uint8_t angle_1_0:2;
            };

            std::array<uint8_t, 3> bytes;
        };
        uint8_t __padding__; // to 32bit
            
        [[nodiscard]] static AnglePacket from_bytes(
            const uint8_t b1, const uint8_t b2, const uint8_t b3
        ){
            AnglePacket ret;
            ret.bytes[0] = b1;
            ret.bytes[1] = b2;
            ret.bytes[2] = b3;
            return ret;
        }


        [[nodiscard]] constexpr IResult<Angular<uq32>> parse() const {


            const auto bits20 = b20();

            #if 0
            // TODO 验证crc校验功能
            // if(vce2755::calc_crc4(bits20) != crc_3_0) [[unlikely]]
            //     return Err(Error::InvalidCrc);
            #endif
            const auto turns = static_cast<uq32>(uq18::from_bits(static_cast<uint32_t>(bits20 >> 2)));
            return Ok(Angular<uq32>::from_turns(turns));
        }
    private:

        [[nodiscard]] constexpr uint32_t b20() const{
            uint32_t bits = 0;
            bits |= static_cast<uint32_t>(bytes[0]) << 12;
            bits |= static_cast<uint32_t>(bytes[1]) << 4;
            bits |= static_cast<uint32_t>(bytes[2]) >> 4;
            return bits;
        }


    };
    static_assert(sizeof(AnglePacket) == 4);

    //0x40
    struct [[nodiscard]] R8_IO:public Reg8<> {
        static constexpr RegAddr ADDRESS = RegAddr{0x40};
        uint8_t spi_3wire_en:1;

        //0: 2mA 
        //1: 4mA
        IoDriveStrength io_strength:1;
        Mux mux:2;
        AbzPowerOnWaveform abz_power_on_waveform:2;
        uint8_t :2;
    };

    //0x41
    struct [[nodiscard]] R8_AbzInvert:public Reg8<> {
        static constexpr RegAddr ADDRESS = RegAddr{0x41};
        uint8_t abz_inv:1;
        uint8_t :5;
        PwmFreq pwm_freq:1;
        uint8_t :1;
    };

    //0x42
    struct [[nodiscard]] R8_Direction:public Reg8<> {
        uint8_t :5;
        uint8_t is_ccw:1;
        uint8_t :2;
    };


    //0x43,0x44,
    struct [[nodiscard]] R16_AbzResolution:public Reg8<> {
        static constexpr RegAddr ADDRESS = RegAddr{0x43};
        uint8_t abz_res_9_8:2;
        uint8_t :6;
        uint8_t abz_res_7_0:8;

        void set_resolution(const uint16_t resolution) {
            abz_res_9_8 = resolution >> 8;
            abz_res_7_0 = resolution & 0xFF;
        }

        uint16_t get_resolution() const {
            return (abz_res_9_8 << 8) | abz_res_7_0;
        }
    };
    
    //0x46,0x47
    struct [[nodiscard]] R16_ZeroPosition:public Reg8<> {
        static constexpr RegAddr ADDRESS = RegAddr{0x46};
        uint8_t zero_pos_11_8:4;
        uint8_t :4;
        uint8_t zero_pos_7_0:8;

        void set_angle(const Angular<uq32> angle) {
            const uint16_t zero_pos_u12 = static_cast<uint16_t>(angle.to_turns().to_bits() >> (32 - 12));
            zero_pos_11_8 = zero_pos_u12 >> 8;
            zero_pos_7_0 = zero_pos_u12 & 0xFF;
        }
    };


    //0x48
    struct [[nodiscard]] R8_Hysteresis:public Reg8<> {
        static constexpr RegAddr ADDRESS = RegAddr{0x48};
        uint8_t :5;
        Hysteresis hysteresis:3;
    };

    //0x4a
    struct [[nodiscard]] R8_ZWidth:public Reg8<> {
        static constexpr RegAddr ADDRESS = RegAddr{0x4a};
        uint8_t :5;
        uint8_t z_width:3;
    };

    //0x4c
    struct [[nodiscard]] R8_UvwResolution:public Reg8<> {
        static constexpr RegAddr ADDRESS = RegAddr{0x4c};
        uint8_t abz_resolution:3;
        uint8_t :5;
    };

    //0x4d
    struct [[nodiscard]] R8_Bandwidth:public Reg8<> {
        static constexpr RegAddr ADDRESS = RegAddr{0x4d};
        BandWidth bandwidth:6;
        WeakMagAlarmLevel weak_mag_alarm_lvl:2;
    };

    AnglePacket packet_;
};


};