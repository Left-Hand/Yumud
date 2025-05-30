#pragma once

#include "CH9431_collections.hpp"

namespace ymd::drivers{

class CH9431_Phy final:public CH9431_Collections {
public:
    static constexpr uint8_t WRITE_CMD = 0x03;
    static constexpr uint8_t READ_CMD = 0x02;

    enum class ReadNmBits:uint8_t{
        RXB0SIDL_5 = 0b00,
        RXB0DO_8 = 0b01,
        RXB1SIDL_5 = 0b00,
        RXB1DO_8 = 0b11,
    };

    static constexpr uint8_t from_read_nm_bits(const ReadNmBits bits){
        return (std::bit_cast<uint8_t>(bits) << 1) | 0b10010000;
    }
    

    enum class LoadAbcBits:uint8_t{
        TXB0SIDL_5 = 0b00,
        TXB0DO_8 = 0b01,
        TXB1SIDL_5 = 0b00,
        TXB1DO_8 = 0b11,
        TXB2SIDL_5 = 0b00,
        TXB2DO_8 = 0b11,
    };

    enum class FilterMatch:uint8_t{
        RXF0 = 0b000,
        RXF1 = 0b001,
        RXF2 = 0b010,
        RXF3 = 0b011,
        RXF4 = 0b100,
        RXF5 = 0b101,
        RXB1 = 0b110
        // RXB1 = 0b111,
    };

    struct RxStatusRetBits{
        FilterMatch filter_match:3;
        uint8_t is_rtr:1;
        uint8_t is_ext:1;
        uint8_t :1;
        uint8_t rx0i:1;
        uint8_t rx1i:1;
    };

    struct ReadStatusBits{
        uint8_t rx0i:1;
        uint8_t rx1i:1;
        uint8_t txreq:1;
        uint8_t tx0i:1;
        uint8_t tx1req:1;
        uint8_t tx1i:1;
        uint8_t tx2req:1;
        uint8_t tx2i:1;
    };

    
    static constexpr uint8_t from_load_abc_bits(const LoadAbcBits bits){
        return (std::bit_cast<uint8_t>(bits)) | 0b01000000;
    }
    
    struct RequestSendKmnBits{
        uint8_t  CTRL0:1;
        uint8_t  CTRL1:1;
        uint8_t  CTRL2:1;
        const uint8_t __RESV__:5 = 0b10000;
    };

    static_assert(sizeof(RequestSendKmnBits) == 1);

    

    static constexpr uint8_t READ_STATUS_CMD = 0b10100000;
    static constexpr uint8_t READ_RX_STATUS_CMD = 0b10110000;

    static constexpr uint8_t BIT_MODIFY_CMD = 0b00001001;


private:
    hal::SpiDrv spi_drv_;

    void wait_tsc(){
        clock::delay(1us);
    }

    IResult<> modify_bit(const uint8_t addr, const uint8_t mask, const uint8_t data){
        const std::array<uint8_t, 4> buf = {
            BIT_MODIFY_CMD,
            addr,
            mask,
            data,
        };

        if(const auto res = spi_drv_.write_burst<uint8_t>(buf.data(), buf.size());
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    IResult<uint8_t> read_byte(const uint8_t cmd){
        spi_drv_.write_single<uint8_t>(cmd, CONT);

        uint8_t ret;
        spi_drv_.read_single<uint8_t>(ret);

        return Ok(ret);
    }

    IResult<> write_byte(const uint8_t cmd, const uint8_t data){
        const auto buf = std::array{cmd, data};
        
        if(const auto res = spi_drv_.write_burst<uint8_t>(buf.data(), buf.size());
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    IResult<> write_cmd(const uint8_t cmd){
        if(const auto res = spi_drv_.write_single<uint8_t>(cmd);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    IResult<RxStatusRetBits> read_rx_status(){
        return read_byte(READ_RX_STATUS_CMD)
            .map([](const uint8_t data){
                return std::bit_cast<RxStatusRetBits>(data);
            });
    }

    IResult<ReadStatusBits> read_status(){
        return read_byte(READ_STATUS_CMD)
            .map([](const uint8_t data){
                return std::bit_cast<ReadStatusBits>(data);
            });
    }
};
}