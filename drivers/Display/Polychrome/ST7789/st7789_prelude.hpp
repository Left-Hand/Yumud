#pragma once

#include "hal/bus/spi/spi.hpp"
#include "hal/bus/spi/spidrv.hpp"

#include "primitive/image/image.hpp"

#include "drivers/Display/prelude/prelude.hpp"

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
        ST7789_ReflashAlgo(const math::Vec2<uint16_t> size):
            size_(size){;}

        __fast_inline constexpr
        uint32_t get_point_index(const math::Vec2<uint16_t> p){
            return (p.x + p.y * size_t(size_.x));
        }

        __fast_inline constexpr
        Range2<uint32_t> get_point_index(const math::Rect2<uint16_t> r){
            return {
                get_point_index(r.top_left), 
                get_point_index({uint16_t(r.top_left.x + r.size.x - 1), uint16_t(r.top_left.y + r.size.y - 1)})};
        }

        bool update(const math::Rect2<uint16_t> rect);
        bool update(const math::Vec2<uint16_t> p){
            return update(math::Rect2<uint16_t>{p, size_});
        }

        auto size() const {return size_;}
    private:
        const math::Vec2<uint16_t> size_;
        math::Rect2<uint16_t> now_area_ = math::Rect2<uint16_t>::ZERO;
        uint32_t last_point_ = 0;
    };

};

}