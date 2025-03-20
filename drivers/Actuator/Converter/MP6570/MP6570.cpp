#include "MP6570.hpp"
#include "core/debug/debug.hpp"

using namespace ymd::drivers;

using ymd::BusError;  

scexpr size_t count_ones(const uint16_t val){
    return __builtin_popcount(val);
}

scexpr bool is_odd(const uint16_t val){
    // return count_ones(val) % 2;
    return count_ones(val) & 0b1;
}



struct SpiFrame{
    // #pragma pack(push, 1)
    // uint8_t slave_addr:7;
    // uint8_t write:1;
    // uint8_t address:7;
    // uint8_t pen:1;
    
    // const uint8_t __resv1__ :4 = 0b0000;
    // uint8_t data_h:8;
    // uint8_t data_l:8;
    // const uint8_t __resv2__ :2 = 0b11;
    // uint8_t por:1;
    // const uint8_t __resv2__ :1 = 0b1;
    // #pragma pack(pop)
    // strc
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

    constexpr SpiFrame(const WriteConfig & config):
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

    constexpr SpiFrame(const ReadConfig & config):
        header{
            uint8_t(uint8_t(config.slave_addr << 1) | uint8_t(0b0)),

            uint8_t(config.reg_addr | uint8_t(config.pen)), 
        },
        payload{
            0,0,0
        } {}
};

struct SpiReadResult{
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

BusError MP6570::Phy::write_reg(const uint8_t reg_addr, const uint16_t data){
    if(i2c_drv_){
        return i2c_drv_->write_reg(reg_addr, data, MSB);
    }else if(spi_drv_){

        const SpiFrame frame = {SpiFrame::WriteConfig{
            .slave_addr = spi_slave_addr_,
            .reg_addr = reg_addr,
            .pen = true,
            .data = data,
        }};

        return spi_drv_->write_burst<uint8_t>(
            (frame.payload.begin())
            , (frame.payload.size()),
            DISC);
    }else{
        PANIC("No phy");
        __builtin_unreachable();
    }
}

BusError MP6570::Phy::read_reg(const uint8_t reg_addr, uint16_t & data){
    if(i2c_drv_){
        return i2c_drv_->read_reg(reg_addr, data, MSB);
    }else if(spi_drv_){
        const SpiFrame frame = {SpiFrame::ReadConfig{
            .slave_addr = spi_slave_addr_,
            .reg_addr = reg_addr,
            .pen = true,
        }};

        spi_drv_->write_burst<uint8_t>(
            (frame.payload.begin())
            , (frame.payload.size()),
            CONT).unwrap();

        SpiReadResult rr;

        spi_drv_->read_burst<uint8_t>(
            (rr.payload.begin())
            , (rr.payload.size()),
            DISC
        ).unwrap();

        const auto result_opt = rr.result();
        if(result_opt){
            data = *result_opt;
            return BusError::OVERLOAD;
        }else{
            return BusError::OK;
        }
        
    }else{
        PANIC("No phy");
        __builtin_unreachable();
    }
}
