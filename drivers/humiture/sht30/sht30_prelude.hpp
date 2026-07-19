#pragma once

//参考资料：
// https://blog.csdn.net/qq_44016222/article/details/142312501


// SHT3x-DIS是新一代的温湿度传感器，基于新的CMOSens传感器芯片，
// 是新的温湿度平台的核心。与前代产品相比，SHT3x-DIS具有更高的智能性、可靠性和更精确的规格。
// 其功能包括增强的信号处理、两个独特且用户可选择的I2C地址和高达1 MHz的通信速度。
// DFN封装尺寸为2.5×2.5 mm²，高度为0.9 mm，可集成到各种应用中。
// 此外，2.15 V至5.5 V的宽电源电压范围保证了与不同组装情况的兼容性。



#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"

#include "core/math/real.hpp"
#include "core/math/realmath.hpp"

#include "hal/conn/i2c/i2cdrv.hpp"


namespace ymd::drivers{

struct SHT30_Prelude{
    // ADDR (pin 2) connected to logic low 
    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0x44);

    // ADDR (pin 2) connected to logic high
    static constexpr auto EXTRA_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0x45);

    enum class [[nodiscard]] Error_Kind:uint8_t{
        CrcMismatch
    };
    
    DEF_FRIEND_DERIVE_DEBUG(Error_Kind)
    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

    template<typename T = void>
    using IResult = Result<T, Error>;
    
    enum class RegAddr:uint8_t{

    };


    struct [[nodiscard]] ChecksumBuilder final{
        static constexpr uint8_t POLYNOMIAL = 0x31;

        static constexpr ChecksumBuilder from_default(){
            ChecksumBuilder self;
            self.checksum = 0xff;
            return self;
        }

    
        constexpr ChecksumBuilder push_byte(const uint8_t byte) const noexcept {
            ChecksumBuilder self = *this;
			self.checksum ^= (byte);
			for(int bit = 8; bit > 0; --bit) {
					if(self.checksum & 0x80) {
							self.checksum = (self.checksum << 1) ^ POLYNOMIAL;
					}  else {
							self.checksum = (self.checksum << 1);
					}
			}
            return self;
        }

        constexpr ChecksumBuilder push_bytes(std::span<const uint8_t> bytes) const noexcept {
            ChecksumBuilder self = *this;
            for(size_t i = 0; i < bytes.size(); i++){
                self = self.push_byte(bytes[i]);
            }
            return self;
        }

        [[nodiscard]] uint8_t finalize() const noexcept {
            return checksum;
        }
    private:
        uint8_t checksum;
    };


    struct [[nodiscard]] Command final{
        uint16_t bits;


    };

    static constexpr Command FETCH_DATA_COMMAND = Command{0xe000};
    static constexpr Command ART_COMMAND = Command{0x2b32};
    static constexpr Command BREAK_COMMAND = Command{0x3093};
    static constexpr Command SOFT_RESET_COMMAND = Command{0x30a2};

    struct [[nodiscard]] TemperatureCode final {
        uint16_t bits;

        constexpr iq16 to_celsius(){
            const auto celsius = iq16::from_bits(static_cast<int32_t>(bits)) * 175 - 45;
            return celsius;
        }
    };

    struct [[nodiscard]] HumiCode final{
        uint16_t bits;

        constexpr iq16 to_perunit(){
            return iq16::from_bits(static_cast<int32_t>(bits));
        }
    };

    struct Packet{
        using Self = Packet;

        TemperatureCode temp_code;
        HumiCode humi_code;

        static constexpr IResult<Packet> try_from_bytes(std::span<const uint8_t, 6> bytes){
            
            auto validate = [](const uint8_t * ptr) -> Result<void, void>{
                const uint8_t expected_checksum = ChecksumBuilder::from_default()
                    .push_byte(ptr[0])
                    .push_byte(ptr[1])
                    .finalize();

                const uint8_t actual_checksum = ptr[2];
                if(expected_checksum != actual_checksum) 
                    return Err();
                return Ok();
            };

            if(const auto res = validate(&bytes[0]); 
                res.is_err()) return Err(Error::CrcMismatch);

            if(const auto res = validate(&bytes[3]); 
                res.is_err()) return Err(Error::CrcMismatch);

            Self self = {
                .temp_code = {.bits = static_cast<uint16_t>(bytes[0] << 8 | bytes[1])},
                .humi_code = {.bits = static_cast<uint16_t>(bytes[3] << 8 | bytes[4])}
            };

            return Ok(self);
        } 
    };
};

struct SHT30_Regset:public SHT30_Prelude{
};


class SHT30{

};

}


