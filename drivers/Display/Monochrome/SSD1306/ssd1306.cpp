#include "ssd1306.hpp"
#include "core/debug/debug.hpp"

using namespace ymd;
using namespace ymd::drivers;

using Error = SSD13XX::Error;

template<typename T = void>
using IResult = Result<T, Error>;

IResult<> SSD1306_Transport::write_command(const uint8_t cmd){
    if(p_i2c_drv_.has_value()){
        if(const auto res = p_i2c_drv_->write_reg<uint8_t>(CMD_TOKEN, uint8_t(cmd));
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }else if(p_spi_drv_.has_value()){
        // return IResult<>(p_spi_drv_->write_reg<uint8_t>(CMD_TOKEN, uint8_t(cmd)));
        TODO();
    }
    return Err(Error(Error::NoAvailablePhy));
}

IResult<> SSD1306_Transport::write_data(const uint8_t data){
    if(p_i2c_drv_.has_value()) {
        if(const auto res = p_i2c_drv_->write_reg<uint8_t>(DATA_TOKEN, uint8_t(data));
            res.is_err()) return Err(res.unwrap_err());
    }else if(p_spi_drv_.has_value()){
        // return IResult<>(p_spi_drv_->write_reg<uint8_t>(DATA_TOKEN, uint8_t(data)));
        TODO();
    }
    return Err(Error(Error::NoAvailablePhy));
}

template<is_stdlayout T>
IResult<> SSD1306_Transport::write_burst(const std::span<const T> pbuf){
    if(p_i2c_drv_.has_value()){
        if constexpr(sizeof(T) != 1){
            if(const auto res = p_i2c_drv_->write_burst<T>(DATA_TOKEN, pbuf, std::endian::little);
                res.is_err()) return Err(res.unwrap_err());
            return Ok();
        }else {
            if(const auto res = p_i2c_drv_->write_burst<T>(DATA_TOKEN, pbuf);
                res.is_err()) return Err(res.unwrap_err());
            return Ok();
        }
    }else if(p_spi_drv_.has_value()){
        // if constexpr(sizeof(T) != 1){
        //     return IResult<>(p_spi_drv_->write_burst<T>(pbuf));
        // }else {
        //     return IResult<>(p_spi_drv_->write_burst<T>(pbuf));
        // }
        TODO();
    }
    return Err(Error(Error::Kind::NoAvailablePhy));
}

template<is_stdlayout T>
IResult<> SSD1306_Transport::write_repeat(const T data, size_t len){
    if(p_i2c_drv_.has_value()){
        if constexpr(sizeof(data) != 1){
            return IResult<>(p_i2c_drv_->write_repeat<T>(DATA_TOKEN, data, len, std::endian::little));
        }else {
            return IResult<>(p_i2c_drv_->write_repeat<T>(DATA_TOKEN, data, len));
        }
    }else if(p_spi_drv_.has_value()){
        TODO();
        // if constexpr(sizeof(data) != 1){
        //     return IResult<>(p_spi_drv_->write_repeat<T>(data, len, std::endian::little));
        // }else {
        //     return IResult<>(p_spi_drv_->write_repeat<T>(data, len));
        // }
    }
    return Err(Error(Error::NoAvailablePhy));
}


IResult<> SSD13XX::set_offset(const math::Vec2u16 offset){
    if(const auto res = write_command(0xD3);
        res.is_err()) return res; 
    if(const auto res = write_command(offset.y);
        res.is_err()) return res;
    return Ok();
}


IResult<> SSD13XX::set_flush_pos(const math::Vec2u16 pos){
    const auto x = std::get<0>(pos + offset_);
    const auto y = std::get<1>(pos + offset_);
    // const auto [x, y] = pos + offset_;
    if(const auto res = write_command(static_cast<uint8_t>(0xb0 | static_cast<uint8_t>(y >> 3)));
        res.is_err()) return res;
    if(const auto res = write_command(static_cast<uint8_t>((x & 0xf0 ) >> 4) | 0x10);
        res.is_err()) return res;
    if(const auto res = write_command(static_cast<uint8_t>(x & 0x0f));
        res.is_err()) return res;
    return Ok();
}

IResult<> SSD13XX::enable_display(const Enable en){
    
    if(en == EN){
        if(const auto res = write_command(0x8D);
            res.is_err()) return res;
        if(const auto res = write_command(0x14);
            res.is_err()) return res;
        if(const auto res = write_command(0xAF);
            res.is_err()) return res;
    }else{
        if(const auto res = write_command(0x8D);
            res.is_err()) return res;
        if(const auto res = write_command(0x10);
            res.is_err()) return res;
        if(const auto res = write_command(0xAE);
            res.is_err()) return res;
    }

    return Ok();
}

IResult<> SSD13XX::update(){
    auto & frame = fetch_frame();
    for(size_t y = 0; y < size().y; y += 8){
        if(const auto res = set_flush_pos(math::Vec2u16(0, y)); 
            res.is_err()) return res;

        const auto line = std::span<const uint8_t>(
            &frame[(y / 8) * size().x], size().x);

        if(const auto res = transport_.write_burst(line);
            res.is_err()) return res;
    }
    return Ok();
}

IResult<> SSD13XX::preinit_by_cmds(const std::span<const uint8_t> init_cmds_list){
    // DEBUG_PRINTLN(init_cmds_list_);
    for(const auto cmd:init_cmds_list){
        if(const auto res = write_command(cmd);
            res.is_err()) return res;
    }

    return Ok();
}

// IResult<> SSD13XX::enable_inversion(const bool i){
//     if(const auto res = write_command(0xC8 - 8*uint8_t(i));
//         res.is_err()) return res;  //正常显示
//     return write_command(0xA1 - uint8_t(i));
// }
