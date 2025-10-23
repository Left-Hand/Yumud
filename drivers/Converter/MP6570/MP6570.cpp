#include "MP6570.hpp"
#include "core/debug/debug.hpp"

using namespace ymd;
using namespace ymd::drivers;


static constexpr size_t count_ones(const uint16_t val){
    return __builtin_popcount(val);
}

static constexpr bool is_odd(const uint16_t val){
    return count_ones(val) & 0b1;
}



struct TxFrame{
    std::array<uint8_t, 2> header;
    std::array<uint8_t, 3> payload;

    struct WriteConfig{
        const uint8_t slave_addr;
        const uint8_t reg_addr;
        const bool pen;
        const uint16_t data;
    };
    
    struct ReadConfig{
        const uint8_t slave_addr;
        const uint8_t reg_addr;
        const bool pen;
    };

    constexpr TxFrame(const WriteConfig & config):
        header{
            uint8_t(uint8_t(config.slave_addr << 1) | uint8_t(0b1)),

            uint8_t(config.reg_addr | uint8_t(config.pen)), 
        },
        payload{

            uint8_t((config.data >> 12) & 0b1111), 

            uint8_t(config.data >> 4),

            uint8_t(uint8_t((config.data & 0xff) << 4) | 
                uint8_t(config.pen ? 
                    (is_odd(config.data) ? uint8_t(0b10) : uint8_t(0b00)) 
                    : uint8_t(0b0))),
        } {}

    constexpr TxFrame(const ReadConfig & config):
        header{
            uint8_t(uint8_t(config.slave_addr << 1) | uint8_t(0b0)),

            uint8_t(config.reg_addr | uint8_t(config.pen)), 
        },
        payload{
            0,0,0
        } {}
};

struct RxFrame{
    std::array<uint8_t, 3> payload;

    std::optional<uint16_t> result(){

        //fuck you big endian!!!
        const uint32_t buf = uint32_t(payload[2]) | uint32_t(payload[1] << 8) | uint32_t(payload[0] << 16);   
        const uint16_t data = uint16_t(buf >> 4);

        if((payload[0] & 0xf0) != 0xf0) return std::nullopt;
        if((payload[2] & 0x0e) != 0x0e) return std::nullopt;

        bool assume_is_odd = is_odd(data);
        bool is_odd_bit = (payload[2] & 0x01);

        if(assume_is_odd != is_odd_bit) return std::nullopt;

        return data;
    }
};

hal::HalResult MP6570_Phy::write_reg(const uint8_t reg_addr, const uint16_t data){
    if(i2c_drv_){
        return i2c_drv_->write_reg(reg_addr, data, MSB);
    }else if(spi_drv_){

        const TxFrame frame = {TxFrame::WriteConfig{
            .slave_addr = spi_slave_addr_,
            .reg_addr = reg_addr,
            .pen = true,
            .data = data,
        }};

        return spi_drv_->write_burst<uint8_t>(
            std::span(frame.payload),
            DISC);
    }else{
        PANIC("No phy");
        __builtin_unreachable();
    }
}

hal::HalResult MP6570_Phy::read_reg(const uint8_t reg_addr, uint16_t & data){
    if(i2c_drv_){
        return i2c_drv_->read_reg(reg_addr, data, MSB);
    }else if(spi_drv_){
        const TxFrame frame = {TxFrame::ReadConfig{
            .slave_addr = spi_slave_addr_,
            .reg_addr = reg_addr,
            .pen = true,
        }};

        if(const auto err = spi_drv_->write_burst<uint8_t>(
            std::span(frame.payload),
            CONT); err.is_err()) return err;

        RxFrame rr;

        if(const auto err = spi_drv_->read_burst<uint8_t>(
            std::span(rr.payload),
            DISC); err.is_err()) return err;

        const auto result_opt = rr.result();
        if(result_opt){
            data = *result_opt;
            return hal::HalResult::BusOverload;
        }else{
            return hal::HalResult::Ok();
        }
        
    }else{
        PANIC("No phy");
        __builtin_unreachable();
    }
}
