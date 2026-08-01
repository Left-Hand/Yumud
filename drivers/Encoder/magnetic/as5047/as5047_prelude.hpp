#pragma once

#include "hal/conn/spi/spidrv.hpp"

#include "drivers/encoder/encoder.hpp"
#include "core/utils/reg_base.hpp"

namespace ymd::drivers{

struct AS5047_Prelude{

    using RegAddr = uint16_t;
    using Error = EncoderError;


    struct [[nodiscard]] Packet final{
        uint16_t bits;
    };


    static constexpr bool is_even_low15(const uint16_t bits){
        return std::popcount<uint16_t>(bits & 0x7fff) % 2 == 0;
    }

    static constexpr Packet make_write_addr_packet(const uint16_t addr){
        uint16_t bits = addr & 0x3fff;
        const bool is_even = is_even_low15(bits);
        bits |= (is_even ? 0x8000 : 0);
        return {bits};
    }

    static constexpr Packet make_write_data_packet(const uint16_t data){
        //the same
        return make_write_addr_packet(data);
    }

    static constexpr Packet writepkt_to_readpkt(Packet pkt){
        pkt.bits |= 0x4000;

        return pkt;
    }

    static constexpr Packet make_read_addr_packet(const uint16_t addr){
        return writepkt_to_readpkt(make_write_addr_packet(addr));
    }
};

struct AS5047_Regs:public AS5047_Prelude{

    struct ErrflReg:public Reg8{
        static constexpr RegAddr REG_ADDR = RegAddr{0x001};


        uint8_t frame_error:1;
        uint8_t invalid_cmd_error:1;
        uint8_t parity_error:1;
        uint8_t :5;
    };

    struct ProgReg:public Reg8{
        static constexpr RegAddr REG_ADDR = RegAddr{0x002};

        uint8_t prog_otp_en:1;
        uint8_t otp_reflash:1;
        uint8_t start_otp_prog:1;
        uint8_t prog_verify:1;
        uint8_t :4;
    };

};

}