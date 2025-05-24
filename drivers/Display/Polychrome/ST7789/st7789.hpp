#pragma once

#include "drivers/Display/DisplayerPhy.hpp"
#include "types/image/image.hpp"
#include "hal/bus/spi/spi.hpp"
#include "hal/bus/spi/spidrv.hpp"

namespace ymd::drivers{

class ST7789_Phy final{
public:
    template<typename T = void>
    using IResult = Result<void, drivers::DisplayerError>;
private:
    hal::SpiHw & spi_;
    hal::SpiSlaveIndex idx_;

    hal::Gpio & dc_gpio_;
    hal::Gpio & res_gpio_;
    hal::Gpio & blk_gpio_;

    static constexpr auto COMMAND_LEVEL = LOW;
    static constexpr auto DATA_LEVEL = HIGH;


    template <hal::valid_spi_data T>
    hal::HalResult phy_write_burst(const std::span<const auto> pdata, Continuous cont = DISC) {
        if (const auto err = spi_.begin(idx_.to_req()); err.is_err()) return err; 
        if constexpr (sizeof(T) != 1){
            if(const auto res = spi_.set_data_width(magic::type_to_bits_v<T>); res.is_err())
                return res;
        }

        const auto len = pdata.size();
        // DEBUG_PRINTLN(len, pdata[0], static_cast<T>(pdata[0]));
        for (size_t i = 0; i < len; i++){
            (void)spi_.fast_write(static_cast<RGB565>(pdata[i]));
            // (void)spi_.write(static_cast<uint32_t>(p[i]));
        } 

        if (cont == DISC) spi_.end();

        if constexpr (sizeof(T) != 1) {
            if(const auto res = spi_.set_data_width(8); res.is_err()) return res;
        }

        return hal::HalResult::Ok();
    }

    template <hal::valid_spi_data T>
    hal::HalResult phy_write_repeat(const is_stdlayout auto data, const size_t len, Continuous cont = DISC) {
        static_assert(sizeof(T) == sizeof(std::decay_t<decltype(data)>));
        if (const auto err = spi_.begin(idx_.to_req()); err.is_err()) return err; 
        if constexpr (sizeof(T) != 1){
            if(const auto res = spi_.set_data_width(sizeof(T) * 8); res.is_err())
                return res;
        }
        for (size_t i = 0; i < len; i++){
            if(const auto res = spi_.write(uint32_t(static_cast<T>(data)));
                res.is_err()) return res;
        } 
        if (cont == DISC) spi_.end();
        if constexpr (sizeof(T) != 1) {
            if(const auto res = spi_.set_data_width(8); res.is_err()) return res;
        }
        return hal::HalResult::Ok();
    }

    template<hal::valid_spi_data T>
    hal::HalResult phy_write_single(const is_stdlayout auto data, Continuous cont = DISC) {
        static_assert(sizeof(T) == sizeof(std::decay_t<decltype(data)>));

        if(const auto res = spi_.begin(idx_.to_req()); res.is_err()) return res;
        if constexpr (sizeof(T) != 1){
            if(const auto res = spi_.set_data_width(sizeof(T) * 8); res.is_err())
                return res;
        }

        if constexpr (sizeof(T) == 1) {
            if(const auto res = spi_.write(uint8_t(data)); res.is_err()) return res;
        } else if constexpr (sizeof(T) == 2) {
            if(const auto res = spi_.write(uint16_t(data)); res.is_err()) return res;
        }

        if (cont == DISC) spi_.end();
        if constexpr (sizeof(T) != 1) {
            if(const auto res = spi_.set_data_width(8); res.is_err()) return res;
        }

        return hal::HalResult::Ok();
    }

public:


    // ST7789_Phy(
    //     const hal::SpiDrv & spi_drv, 
    //     hal::Gpio & dc_gpio, 
    //     hal::Gpio & res_gpio = hal::NullGpio,
    //     hal::Gpio & blk_gpio = hal::NullGpio
    // ) : spi_drv_(spi_drv), dc_gpio_(dc_gpio), res_gpio_(res_gpio), blk_gpio_(blk_gpio){}


    ST7789_Phy(
        hal::SpiHw & bus,
        const hal::SpiSlaveIndex index,
        hal::Gpio & dc_gpio, 
        hal::Gpio & res_gpio = hal::NullGpio,
        hal::Gpio & blk_gpio = hal::NullGpio
    ):spi_(bus), idx_(index), dc_gpio_(dc_gpio), res_gpio_(res_gpio), blk_gpio_(blk_gpio) {};

    [[nodiscard]] IResult<> init(){
        dc_gpio_.outpp();
        res_gpio_.outpp(HIGH);
        blk_gpio_.outpp(HIGH);

        return reset();
    }

    [[nodiscard]] IResult<> reset(){
        clock::delay(10ms);
        res_gpio_.clr();
        clock::delay(10ms);
        res_gpio_.set();
        return Ok();
    }

    [[nodiscard]] IResult<> set_back_light_brightness(const real_t brightness){
        return Ok();
    }

    [[nodiscard]] IResult<> write_command(const uint8_t cmd){
        // dc_gpio_ = COMMAND_LEVEL;
        dc_gpio_.clr();
        return IResult<>(phy_write_single<uint8_t>(cmd));
    }

    [[nodiscard]] IResult<> write_data8(const uint8_t data){
        // dc_gpio_ = DATA_LEVEL;
        dc_gpio_.set();
        return IResult<>(phy_write_single<uint8_t>(data));
    }

    [[nodiscard]] IResult<> write_data16(const uint16_t data){
        // dc_gpio_ = DATA_LEVEL;
        dc_gpio_.set();
        return IResult<>(phy_write_single<uint16_t>(data));
    }

    template<typename U>
    [[nodiscard]] IResult<> write_burst(std::span<const auto> pdata){
        dc_gpio_.set();
        return IResult<>(phy_write_burst<U>(pdata));
    }

    template<typename U>
    [[nodiscard]] IResult<> write_repeat(const auto & data, size_t len){
        // dc_gpio_ = DATA_LEVEL;
        dc_gpio_.set();
        return IResult<>(phy_write_repeat<U>(data, len));
    }
};

class ST7789:public Displayer<RGB565>{
public:
    template<typename T = void>
    using IResult = Result<void, DisplayerError>;

    class ST7789_ReflashAlgo{
    public:

        ST7789_ReflashAlgo(const Vector2<uint16_t> & size):
            size_(size){;}

        __fast_inline constexpr
        uint32_t get_point_index(const Vector2<uint16_t> p){
            return (p.x + p.y * size_t(size_.x));
        }

        __fast_inline constexpr
        Range2_t<uint32_t> get_point_index(const Rect2<uint16_t> r){
            return {
                get_point_index(r.position), 
                get_point_index({uint16_t(r.position.x + r.size.x - 1), uint16_t(r.position.y + r.size.y - 1)})};
        }

        bool update(const Rect2<uint16_t> rect);
        bool update(const Vector2<uint16_t> p){
            return update(Rect2<uint16_t>{p, size_});
        }
    private:
        const Vector2<uint16_t> size_;
        Rect2<uint16_t> curr_area_ = {0,0,1,1};
        uint32_t last_point_ = 0;
    };

private:
    using Algo = ST7789_ReflashAlgo;

    ST7789_Phy phy_;
    Algo algo_;

    Vector2<uint16_t> offset_;
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

    __fast_inline void putpixel_unsafe(const Vector2u & pos, const RGB565 color){
        setpos_unsafe(pos);
        phy_.write_data16(uint16_t(color)).unwrap();
    }

    void putrect_unsafe(const Rect2u & rect, const RGB565 color);
    void puttexture_unsafe(const Rect2u & rect, const RGB565 * color_ptr);
    void putseg_v8_unsafe(const Vector2u & pos, const uint8_t mask, const RGB565 color);
    void putseg_h8_unsafe(const Vector2u & pos, const uint8_t mask, const RGB565 color);
public:
    ST7789(const ST7789_Phy & phy, const Vector2<uint16_t> & size):
            ImageBasics(size), 
            Displayer<RGB565>(size), 
            phy_(phy),
            algo_(size){;}

    IResult<> init();

    
    void setpos_unsafe(const Vector2u & pos);
    void setarea_unsafe(const Rect2u & rect);

    void put_texture(const Rect2u & rect, const is_color auto * pcolor){
        setarea_unsafe(rect);
        put_next_texture(rect, pcolor);
    }

    void put_next_texture(const Rect2u & rect, const is_color auto * pcolor){
        phy_.write_burst<uint16_t>(std::span(pcolor, rect.get_area())).unwrap();
    }

    IResult<> set_display_offset(const Vector2u & _offset){
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