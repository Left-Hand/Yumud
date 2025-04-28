#pragma once

#include "drivers/Display/DisplayerPhy.hpp"
#include "types/image/image.hpp"
#include "hal/bus/spi/spi.hpp"

namespace ymd::drivers{

class ST7789_Phy final{
public:
    template<typename T = void>
    using IResult = Result<void, DisplayerError>;
protected:
    hal::SpiDrv spi_drv_;
    hal::Gpio & dc_gpio_;
    hal::Gpio & res_gpio_;
    hal::Gpio & blk_gpio_;

    static constexpr auto COMMAND_LEVEL = LOW;
    static constexpr auto DATA_LEVEL = HIGH;
public:

    ST7789_Phy(
            hal::SpiDrv && spi_drv, 
            hal::Gpio & dc_gpio, 
            hal::Gpio & res_gpio = hal::NullGpio,
            hal::Gpio & blk_gpio = hal::NullGpio
        ) :spi_drv_(std::move(spi_drv)), dc_gpio_(dc_gpio), res_gpio_(res_gpio), blk_gpio_(blk_gpio){}

    ST7789_Phy(
        const hal::SpiDrv & spi_drv, 
        hal::Gpio & dc_gpio, 
        hal::Gpio & res_gpio = hal::NullGpio,
        hal::Gpio & blk_gpio = hal::NullGpio
    ) : spi_drv_(spi_drv), dc_gpio_(dc_gpio), res_gpio_(res_gpio), blk_gpio_(blk_gpio){}


    ST7789_Phy(
        hal::Spi & bus,
        const hal::SpiSlaveIndex index,
        hal::Gpio & dc_gpio, 
        hal::Gpio & res_gpio = hal::NullGpio,
        hal::Gpio & blk_gpio = hal::NullGpio
    ):ST7789_Phy(hal::SpiDrv(bus, index), dc_gpio, res_gpio, blk_gpio) {};

    [[nodiscard]] IResult<> init(){
        dc_gpio_.outpp();
        res_gpio_.outpp(HIGH);
        blk_gpio_.outpp(HIGH);

        return reset();
    }

    [[nodiscard]] IResult<> reset(){
        delay(10);
        res_gpio_.clr();
        delay(10);
        res_gpio_.set();
        return Ok();
    }

    [[nodiscard]] IResult<> set_back_light_brightness(const real_t brightness){
        return Ok();
    }

    [[nodiscard]] IResult<> write_command(const uint8_t cmd){
        dc_gpio_ = COMMAND_LEVEL;
        return IResult<>(spi_drv_.write_single<uint8_t>(cmd));
    }

    [[nodiscard]] IResult<> write_data8(const uint8_t data){
        dc_gpio_ = DATA_LEVEL;
        return IResult<>(spi_drv_.write_single<uint8_t>(data));
    }

    [[nodiscard]] IResult<> write_data16(const uint16_t data){
        dc_gpio_ = DATA_LEVEL;
        return IResult<>(spi_drv_.write_single<uint16_t>(data));
    }

    template<typename U>
    [[nodiscard]] IResult<> write_burst(const auto * data, size_t len){
        dc_gpio_ = DATA_LEVEL;
        return IResult<>(spi_drv_.write_burst<U>(data, len));
    }

    template<typename U>
    [[nodiscard]] IResult<> write_burst(const auto & data, size_t len){
        dc_gpio_ = DATA_LEVEL;
        return IResult<>(spi_drv_.write_burst<U>(data, len));
    }
};

class ST7789:public Displayer<RGB565>{
public:
    template<typename T = void>
    using IResult = Result<void, DisplayerError>;

    class ST7789_ReflashAlgo{
    public:

        ST7789_ReflashAlgo(const Vector2_t<uint16_t> & size):
            size_(size){;}

        __fast_inline constexpr
        uint32_t get_point_index(const Vector2_t<uint16_t> p){
            return (p.x + p.y * size_t(size_.x));
        }

        __fast_inline constexpr
        Range_t<uint32_t> get_point_index(const Rect2_t<uint16_t> r){
            return {
                get_point_index(r.position), 
                get_point_index({r.position.x + r.size.x - 1, r.position.y + r.size.y - 1})};
        }

        bool update(const Rect2_t<uint16_t> rect);
        bool update(const Vector2_t<uint16_t> p){
            return update(Rect2_t<uint16_t>{p, size_});
        }
    private:
        const Vector2_t<uint16_t> size_;
        Rect2_t<uint16_t> curr_area_ = Rect2i(0,0,1,1);
        uint32_t last_point_ = 0;
    };

private:
    using Algo = ST7789_ReflashAlgo;

    ST7789_Phy phy_;
    Algo algo_;

    Vector2_t<uint16_t> offset_;
    uint8_t scr_ctrl_ = 0;

    __fast_inline IResult<> write_command(const uint8_t cmd){
        return phy_.write_command(cmd);
    }

    __fast_inline IResult<> write_data8(const uint8_t data){
        return phy_.write_data8(data);
    }

    __fast_inline IResult<> write_data16(const uint16_t data){
        return phy_.write_data16(data);
    }

    IResult<> modify_ctrl(const bool yes, const uint8_t pos);

protected:

    void setpos_unsafe(const Vector2i & pos);
    void setarea_unsafe(const Rect2i & rect);

    __fast_inline void putpixel_unsafe(const Vector2i & pos, const RGB565 color){
        setpos_unsafe(pos);
        phy_.write_data16(uint16_t(color)).unwrap();
    }

    void putrect_unsafe(const Rect2i & rect, const RGB565 color);
    void puttexture_unsafe(const Rect2i & rect, const RGB565 * color_ptr);
    void putseg_v8_unsafe(const Vector2i & pos, const uint8_t mask, const RGB565 color);
    void putseg_h8_unsafe(const Vector2i & pos, const uint8_t mask, const RGB565 color);
public:
    ST7789(const ST7789_Phy & phy, const Vector2_t<uint16_t> & size):
            ImageBasics(size), 
            Displayer<RGB565>(size), 
            phy_(phy),
            algo_(size){;}

    IResult<> init();

    void put_texture(const Rect2i & rect, const is_color auto * color_ptr){
        setarea_unsafe(rect);
        phy_.write_burst<uint16_t>(color_ptr, rect.get_area()).unwrap();
    }

    IResult<> set_display_offset(const Vector2i & _offset){
        offset_ = _offset;
        return Ok();
    }
    
    IResult<> set_flip_y(const bool flip){
        return modify_ctrl(flip, 7);
    }

    IResult<> set_flip_x(const bool flip){
        return modify_ctrl(flip, 6);
    }

    IResult<> set_swap_xy(const bool flip){
        return modify_ctrl(flip, 5);
    }

    IResult<> set_flush_dir_v(const bool dir){
        return modify_ctrl(dir, 4);
    }

    IResult<> set_format_rgb(const bool is_rgb){
        return modify_ctrl(!is_rgb, 3);
    }

    IResult<> set_flush_dir_h(const bool dir){
        return modify_ctrl(dir, 2);
    }


    IResult<> set_inversion(const bool inv){
        return write_command(0x20 + inv);
    }
};

enum class ST7789_Presets{
    _120X80,
    _240X135,
    _320X170
};

Result<void, DisplayerError> init_lcd(ST7789 & displayer, const ST7789_Presets preset);

};