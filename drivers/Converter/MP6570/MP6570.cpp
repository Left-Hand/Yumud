#include "mp6570.hpp"
#include "core/debug/debug.hpp"


#define MP6570_DEBUG

#ifdef MP6570_DEBUG
#undef MP6570_DEBUG
#define MP6570_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define MP6570_PANIC(...) PANIC(__VA_ARGS__)
#define MP6570_ASSERT(cond, ...) ASSERT(cond, __VA_ARGS__)
#else
#define MP6570_DEBUG(...)
#define MP6570_PANIC(...)  PANIC_NSRC()
#define MP6570_ASSERT(cond, ...) ASSERT_NSRC(cond)
#endif



#define RAISE_ERR(x, ...) ({\
    const auto && __err_check_err = (x);\
    ASSERT{false, #x, ##__VA_ARGS__};\
    __err_check_err;\
})\

#define RETURN_ON_ERR(x) ({\
    if(const auto __res_return_on_err = (x); __res_return_on_err.is_err()){\
        return CHECK_RES(__res_return_on_err);\
    }\
});\


using namespace ymd;
using namespace ymd::drivers::mp6570;



[[nodiscard]] static constexpr size_t count_ones(const uint16_t val){
    return __builtin_popcount(val);
}

[[nodiscard]] static constexpr bool is_odd(const uint16_t val){
    return count_ones(val) & 0b1;
}


static constexpr Option<uint16_t> rx_bytes_to_data(std::span<const uint8_t, 3> bytes){
    const uint32_t buf = uint32_t(bytes[2]) | uint32_t(bytes[1] << 8) | uint32_t(bytes[0] << 16);   
    const uint16_t reg_val = uint16_t(buf >> 4);

    if((bytes[0] & 0xf0) != 0xf0) return None;
    if((bytes[2] & 0x0e) != 0x0e) return None;

    bool assume_is_odd = is_odd(reg_val);
    bool is_odd_bit = (bytes[2] & 0x01);

    if(assume_is_odd != is_odd_bit) return None;

    return Some(reg_val);

}


hal::HalResult MP6570_SpiTransport::write_reg(const uint8_t reg_addr, const uint16_t reg_val){
    const TxPacket packet = TxPacket::from_write({
        .slave_addr = spi_slave_addr_,
        .reg_addr = reg_addr,
        .pen = true,
        .reg_val = reg_val,
    });

    return spi_drv_.write_bulk<uint8_t>(
        std::span(packet.payload_bytes),
        DISC);
}

hal::HalResult MP6570_SpiTransport::read_reg(const uint8_t reg_addr, uint16_t & reg_val){
    const TxPacket packet = TxPacket::from_read({
        .slave_addr = spi_slave_addr_,
        .reg_addr = reg_addr,
        .pen = true,
    });

    if(const auto res = spi_drv_.write_bulk<uint8_t>(
        std::span(packet.payload_bytes),
        CONT); res.is_err()) return res;

    std::array<uint8_t, 3> rx_bytes;

    if(const auto res = spi_drv_.read_bulk<uint8_t>(
        std::span(rx_bytes),
        DISC); res.is_err()) return res;

    if(const auto may_data = rx_bytes_to_data(std::span(rx_bytes)); 
        may_data.is_none()
    ){
        PANIC{};
    } else{
        reg_val = may_data.unwrap();
    }
    return hal::HalResult::Ok();
}