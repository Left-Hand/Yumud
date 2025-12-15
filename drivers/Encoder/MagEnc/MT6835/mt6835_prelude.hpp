#pragma once

#include "hal/bus/spi/spidrv.hpp"

#include "drivers/Encoder/MagEncoder.hpp"
#include "core/io/regs.hpp"

namespace ymd::drivers{

struct MT6835_Prelude{
    using Error = EncoderError;

    template<typename T = void>
    using IResult = Result<T, Error>;

    using RegAddr = uint8_t;

    enum class UvwPolePairs:uint8_t{
        _1 = 0,
        _2, _3, _4, _5, _6, _7, _8, _9, 
        _10, _11, _12, _13, _14, _15, _16
    };

    enum class Command:uint16_t{
        Write = 0b0110,
        Read = 0b0011,
        Burn = 0b1100,
        AutoSetZero = 0b0101,
        ContinuousRead = 0b1010
    };

    struct [[nodiscard]] Packet{
        uint8_t angle_20_13;
        uint8_t angle_12_5;
        uint8_t over_speed:1;
        uint8_t mag_weak:1;
        uint8_t under_voltage:1;
        uint8_t angle_4_0:5;
        uint8_t crc;

        [[nodiscard]] std::span<uint8_t, 4> as_bytes_mut(){
            return std::span<uint8_t, 4>(reinterpret_cast<uint8_t*>(this), 4);
        }

        [[nodiscard]] IResult<Angular<uq32>> parse() const {
            if(calc_crc() != crc) [[unlikely]]
                return Err(Error::InvalidCrc);

            if(over_speed) [[unlikely]]
                return Err(Error::OverSpeed);

            if(under_voltage) [[unlikely]]
                return Err(Error::UnderVoltage);

            if(mag_weak) [[unlikely]]
                return Err(Error::MagnetLow);

            const auto turns = uq20::from_bits(angle_20());
            return Ok(Angular<uq32>::from_turns(turns));
        }
    private:
        [[nodiscard]] constexpr uint32_t angle_20() const {
            return (angle_20_13 << 13) | (angle_12_5 << 5) | angle_4_0;
        }

        [[nodiscard]] constexpr uint8_t calc_crc() const{
            // CRC校验公式：x^8+x^2+x+1 angle[20]作为最高位先移位进入
            uint8_t calculated_crc = 0;
            uint32_t data_bits = std::bit_cast<uint32_t>(*this) & 0xFFFFFF; // Extract the 24-bit data portion (excluding CRC)
            
            // Calculate CRC using the polynomial x^8+x^2+x+1 (0x07)
            for(int i = 23; i >= 0; i--) {
                uint8_t bit = (data_bits >> i) & 1;
                uint8_t msb = calculated_crc >> 7;
                
                calculated_crc <<= 1;
                calculated_crc |= bit;
                
                if(msb) {
                    calculated_crc ^= 0x07; // Apply polynomial x^8+x^2+x+1
                }
            }
            
            // Finalize CRC by shifting out the last 8 bits
            for(size_t i = 0; i < 8; i++) {
                uint8_t msb = calculated_crc >> 7;
                calculated_crc <<= 1;
                
                if(msb) {
                    calculated_crc ^= 0x07;
                }
            }
            
            return calculated_crc;

        }
    };

    static_assert(sizeof(Packet) == 4);

};

class MT6835_Regs:public MT6835_Prelude{
    struct R8_UserId:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x001;
        uint8_t id:8;
    };

    struct R8_AngleH:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x003;
        uint8_t angle:8;
    };

    struct R8_AngleM:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x004;
        uint8_t angle:8;
    };

    struct R8_AngleL:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x005;
        uint8_t over_speed:1;
        uint8_t mag_weak:1;
        uint8_t under_voltage:1;
        uint8_t angle:5;
    };

    struct R8_Crc:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x006;
        uint8_t crc:8;
    };

    struct R8_AbzResH:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x007;
        uint8_t resolution:8;
    };

    struct R8_AbzResL:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x008;
        uint8_t ab_swap:1;
        uint8_t abz_off:1;
        uint8_t resolution:6;
    };

    struct R8_ZeroPosH:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x009;
        uint8_t zero_pos:8;
    };

    struct R8_ZeroPosL:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x00A;
        uint8_t z_pul_wid:3;
        uint8_t z_falling_on_0edge:1;
        uint8_t zero_pos:4;
    };


    struct R8_Uvw:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x00B;
        uint8_t uvw_res:4;
        uint8_t uvw_off:1;
        uint8_t uvw_mux:1;
        uint8_t z_phase:2;
    };


    struct R8_Pwm:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x00C;
        uint8_t pwm_sel:3;
        uint8_t pwm_pol:1;
        uint8_t pwm_fq:1;
        uint8_t nlc_en:1;
    };

    struct R8_Roatation:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x00D;
        uint8_t hyst:3;
        uint8_t rot_dir:1;
    };

    struct R8_Cali:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x00E;

        uint8_t :4;
        uint8_t autocal_freq:3;
        uint8_t gpio_ds:1;
    };

    struct R8_BandWidth:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x011;
        uint8_t bw:3;
        uint8_t :5;
    };

    struct R8_Nlc:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x013;
        static constexpr RegAddr ADDRESS_end = 0x0D2;

        uint8_t :8;
    };
};


};