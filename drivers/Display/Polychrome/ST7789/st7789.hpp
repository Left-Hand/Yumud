#pragma once

#include "drivers/Display/prelude/prelude.hpp"
#include "types/image/image.hpp"
#include "hal/bus/spi/spi.hpp"
#include "hal/bus/spi/spidrv.hpp"

namespace ymd::drivers{

struct ST7789_Prelude{

    // https://docs.rs/st7789/latest/st7789/instruction/enum.Instruction.html
    enum class Instruction:uint8_t{
        NOP = 0,
        SWRESET = 1,
        RDDID = 4,
        RDDST = 9,
        SLPIN = 16,
        SLPOUT = 17,
        PTLON = 18,
        NORON = 19,
        INVOFF = 32,
        INVON = 33,
        DISPOFF = 40,
        DISPON = 41,
        CASET = 42,
        RASET = 43,
        RAMWR = 44,
        RAMRD = 46,
        PTLAR = 48,
        VSCRDER = 51,
        TEOFF = 52,
        TEON = 53,
        MADCTL = 54,
        VSCAD = 55,
        COLMOD = 58,
        VCMOFSET = 197,
    };

    using Error = DisplayerError;

    template<typename T = void>
    using IResult = Result<void, Error>;


    class ST7789_ReflashAlgo{
    public:
        ST7789_ReflashAlgo(const Vector2<uint16_t> & size):
            size_(size){;}

        __fast_inline constexpr
        uint32_t get_point_index(const Vector2<uint16_t> p){
            return (p.x + p.y * size_t(size_.x));
        }

        __fast_inline constexpr
        Range2<uint32_t> get_point_index(const Rect2<uint16_t> r){
            return {
                get_point_index(r.position), 
                get_point_index({uint16_t(r.position.x + r.size.x - 1), uint16_t(r.position.y + r.size.y - 1)})};
        }

        bool update(const Rect2<uint16_t> rect);
        bool update(const Vector2<uint16_t> p){
            return update(Rect2<uint16_t>{p, size_});
        }

        auto size() const {return size_;}
    private:
        const Vector2<uint16_t> size_;
        Rect2<uint16_t> curr_area_ = {0,0,1,1};
        uint32_t last_point_ = 0;
    };

};

enum class ST7789_Presets:uint8_t{
    _120X80,
    _240X135,
    _320X170
};


class ST7789_Phy final:
    ST7789_Prelude{
public:
    template<typename T = void>
    using IResult = Result<void, drivers::DisplayerError>;
private:
    hal::SpiHw & spi_;
    hal::SpiSlaveIndex idx_;

    hal::Gpio & dc_gpio_;
    Option<hal::Gpio &>res_gpio_;
    Option<hal::Gpio &>blk_gpio_;

    static constexpr auto COMMAND_LEVEL = LOW;
    static constexpr auto DATA_LEVEL = HIGH;


    template <hal::valid_spi_data T>
    [[nodiscard]] hal::HalResult phy_write_burst(
        const std::span<const auto> pbuf, 
        Continuous cont = DISC) {
        if (const auto err = spi_
            .begin(idx_.to_req()); err.is_err()) return err; 
        if constexpr (sizeof(T) != 1){
            if(const auto res = spi_.set_data_width(magic::type_to_bits_v<T>); res.is_err())
                return res;
        }

        const auto len = pbuf.size();
        // DEBUG_PRINTLN(len, pbuf[0], static_cast<T>(pbuf[0]));
        for (size_t i = 0; i < len; i++){
            (void)spi_.fast_write(static_cast<RGB565>(pbuf[i]).to_u16());
            // (void)spi_.write(static_cast<uint32_t>(p[i]));
        } 

        if (cont == DISC) spi_.end();

        if constexpr (sizeof(T) != 1) {
            if(const auto res = spi_.set_data_width(8); res.is_err()) return res;
        }

        return hal::HalResult::Ok();
    }

    template <hal::valid_spi_data T>
    [[nodiscard]] hal::HalResult phy_write_repeat(
        const is_stdlayout auto data, 
        const size_t len, 
        Continuous cont = DISC) {
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
    [[nodiscard]] hal::HalResult phy_write_single(
        const is_stdlayout auto data, 
        Continuous cont = DISC) {
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
    explicit ST7789_Phy(
        hal::SpiHw & bus,
        const hal::SpiSlaveIndex index,
        Some<hal::Gpio *> dc_gpio, 
        Option<hal::Gpio &> res_gpio = None,
        Option<hal::Gpio &> blk_gpio = None
    ):  
        spi_(bus), 
        idx_(index), 
        dc_gpio_(dc_gpio.deref()), 
        res_gpio_(res_gpio), 
        blk_gpio_(blk_gpio) {};

    [[nodiscard]] IResult<> init(){
        dc_gpio_.outpp();
        if(res_gpio_.is_some())
            res_gpio_.unwrap().outpp(HIGH);

        if(blk_gpio_.is_some())
            blk_gpio_.unwrap().outpp(HIGH);

        return reset();
    }

    [[nodiscard]] IResult<> reset(){
        if(res_gpio_.is_none()) return Ok();
        auto & res_gpio = res_gpio_.unwrap();
        clock::delay(10ms);
        res_gpio.clr();
        clock::delay(10ms);
        res_gpio.set();
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
    [[nodiscard]] IResult<> write_burst(std::span<const auto> pbuf){
        dc_gpio_.set();
        return IResult<>(phy_write_burst<U>(pbuf));
    }

    template<typename U>
    [[nodiscard]] IResult<> write_repeat(const auto & data, size_t len){
        // dc_gpio_ = DATA_LEVEL;
        dc_gpio_.set();
        return IResult<>(phy_write_repeat<U>(data, len));
    }
};

class ST7789 final:
    public ST7789_Prelude{
public:
    explicit ST7789(
        ST7789_Phy && phy, 
        const Vector2<uint16_t> & size
    ):
        phy_(phy),
        algo_(size){;}


    auto size() const {return algo_.size();}

    IResult<> init(const ST7789_Presets preset);
    IResult<> fill(const RGB565 color){
        return putrect_unchecked(size().to_rect(), color);
    }
    IResult<> setpos_unchecked(const Vector2<uint16_t> pos);
    IResult<> setarea_unchecked(const Rect2<uint16_t> rect);

    IResult<> put_texture(const Rect2<uint16_t> & rect, const is_color auto * pcolor){
        if(const auto res = setarea_unchecked(rect);
            res.is_err()) return res;
        if(const auto res = put_next_texture(rect, pcolor);
            res.is_err()) return res;
        return Ok();
    }

    IResult<> put_next_texture(const Rect2<uint16_t> rect, const is_color auto * pcolor){
        return phy_.write_burst<uint16_t>(std::span(pcolor, rect.get_area()));
    }

    IResult<> set_display_offset(const Vector2<uint16_t> _offset){
        offset_ = _offset;
        return Ok();
    }
    
    IResult<> enable_flip_y(const Enable flip){
        return modify_ctrl_reg(flip == EN, 7);
    }

    IResult<> enable_flip_x(const Enable flip){
        return modify_ctrl_reg(flip == EN, 6);
    }

    IResult<> enable_swap_xy(const Enable flip){
        return modify_ctrl_reg(flip == EN, 5);
    }

    IResult<> set_flush_dir_v(const bool dir){
        return modify_ctrl_reg(dir, 4);
    }

    IResult<> set_format_rgb(const bool is_rgb){
        return modify_ctrl_reg(!is_rgb, 3);
    }

    IResult<> set_flush_dir_h(const bool dir){
        return modify_ctrl_reg(dir, 2);
    }


    IResult<> enable_inversion(const Enable inv_en){
        return write_command((inv_en == EN) ? 0x21 : 0x20);
    }

    [[nodiscard]] __fast_inline IResult<> putpixel_unchecked(
        const Vector2<uint16_t> pos, 
        const RGB565 color
    ){
        if(const auto res = setpos_unchecked(pos);
            res.is_err()) return res;
        if(const auto res = phy_.write_data16(uint16_t(color));
            res.is_err()) return res;
        return Ok();
    }

    [[nodiscard]] IResult<> putrect_unchecked(
        const Rect2<uint16_t> rect, 
        const RGB565 color
    );

    [[nodiscard]] IResult<> puttexture_unchecked(
        const Rect2<uint16_t> rect, 
        const RGB565 * pcolor
    );

    [[nodiscard]] Rect2u get_expose_rect(){
        return algo_.size().to_rect();
    }

private:
    using Algo = ST7789_ReflashAlgo;

    ST7789_Phy phy_;
    Algo algo_;

    Vector2<uint16_t> offset_;
    uint8_t scr_ctrl_ = 0;

    [[nodiscard]] __fast_inline IResult<> write_command(const uint8_t cmd){
        return phy_.write_command(cmd);
    }

    [[nodiscard]] __fast_inline IResult<> write_data8(const uint8_t data){
        return phy_.write_data8(data);
    }

    [[nodiscard]] __fast_inline IResult<> write_data16(const uint16_t data){
        return phy_.write_data16(data);
    }

    [[nodiscard]] IResult<> modify_ctrl_reg(const bool is_high, const uint8_t pos);

};



Result<void, DisplayerError> init_lcd(ST7789 & displayer, const ST7789_Presets preset);


};