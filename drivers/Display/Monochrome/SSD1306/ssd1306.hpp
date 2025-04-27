#pragma once

#include "drivers/Display/DisplayerPhy.hpp"
#include "types/image/PackedImage.hpp"

namespace ymd::drivers{

struct SSD13XX_72X40_Config{};
struct SSD13XX_128X64_Config{};
struct SSD13XX_128X32_Config{};
struct SSD13XX_88X48_Config{};
struct SSD13XX_64X48_Config{};
struct SSD13XX_128X80_Config{};

template<typename T>
struct _oled_preset;


class SSD13XX:public Displayer<Binary>{
public:
    using Vector2 = ImageBasics::Vector2;
    using Vector2i = ImageBasics::Vector2i;

    using Error = DisplayerError;

    template<typename T = void>
    using IResult = Result<T, Error>;
protected:
    DisplayerPhyIntf & interface_;
    void setarea_unsafe(const Rect2i & area) {
        setpos_unsafe(area.position);
    }

    void putpixel_unsafe(const Vector2i & pos, const Binary color){
        auto & frame = fetch_frame();
        frame.putpixel_unsafe(pos, color);
    }

    void setpos_unsafe(const Vector2i & pos) ;
    void set_flush_pos(const Vector2i & pos){
        auto [x, y] = pos + offset_;
        interface_.write_command(0xb0 | size_t(y / 8));
        interface_.write_command(((x & 0xf0 )>>4) |0x10);
        interface_.write_command((x & 0x0f));
    }


    [[nodiscard]] IResult<> set_offset(){
        return interface_.write_command(0xD3) |  
        interface_.write_command(offset_.y);
    }

    const Vector2i offset_;
    const std::span<const uint8_t> cmds_;
    VerticalBinaryImage frame_;


    [[nodiscard]] IResult<> preinit_by_cmds();
public:

    template<typename Raw, typename T = std::decay_t<Raw>>
    SSD13XX(DisplayerPhyIntf & interface, Raw && tag):
        ImageBasics(_oled_preset<T>::size),
        Displayer(_oled_preset<T>::size), 
        interface_(interface),
        offset_(_oled_preset<T>::offset), 
        cmds_(_oled_preset<T>::get_cmds()),
        frame_(_oled_preset<T>::size){
        }

    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u8(0x78);

    [[nodiscard]] IResult<> init();

    [[nodiscard]] IResult<> update();

    [[nodiscard]] IResult<> enable(const bool en = true){
        if(en){
            return interface_.write_command(0x8D) | 
            interface_.write_command(0x14) | 
            interface_.write_command(0xAF);
        }else{
            return interface_.write_command(0x8D) | 
            interface_.write_command(0x10) | 
            interface_.write_command(0xAE);
        }
    }

    [[nodiscard]] IResult<> turn_display(const bool i);

    [[nodiscard]] IResult<> enable_flip_y(const bool flip = true){return interface_.write_command(0xA0 | flip);}
    [[nodiscard]] IResult<> enable_flip_x(const bool flip = true){return interface_.write_command(0xC0 | (flip << 3));}
    [[nodiscard]] IResult<> enable_inversion(const bool inv = true){return interface_.write_command(0xA7 - inv);}  

    VerticalBinaryImage & fetch_frame() {return frame_;};
};



namespace details{
template<typename T>
static constexpr Vector2i oled_display_size_v = _oled_preset<T>::size;

template<typename T>
static constexpr Vector2i oled_display_offset_v = _oled_preset<T>::size;

template<typename T>
static constexpr Vector2i oled_initcmd_v = _oled_preset<T>::initcmd;
}

template<>
struct _oled_preset<SSD13XX_72X40_Config>{
    static constexpr Vector2i size = Vector2i(72, 40);
    static constexpr Vector2i offset = Vector2i(28, 0);
    static constexpr auto buf = std::to_array<uint8_t>({ 
        0xAE,0xD5,0xF0,0xA8,0X27,0XD3,0X00,0X40,
        0X8D,0X14,0X20,0X02,0XA1,0XC8,0XDA,0X12,
        0XAD,0X30,0X81,0XFF,0XD9,0X22,0XDB,0X20,
        0XA4,0XA6,0X0C,0X11
    });
    static constexpr std::span<const uint8_t> get_cmds(){

        return std::span<const uint8_t>(buf.data(), buf.size());
    }
};

template<>
struct _oled_preset<SSD13XX_128X64_Config>{
    static constexpr Vector2i size = Vector2i(128, 64);
    static constexpr Vector2i offset = Vector2i(0, 0);
    static constexpr auto buf = std::to_array<uint8_t>({ 
        0xAE,0xD5,0xF0,0xA8,0X27,0XD3,0X00,0X40,
        0X8D,0X14,0X20,0X02,0XA1,0XC8,0XDA,0X12,
        0XAD,0X30,0X81,0XFF,0XD9,0X22,0XDB,0X20,
        0XA4,0XA6,0X0C,0X11
    });

    static constexpr std::span<const uint8_t> get_cmds(){

        return std::span<const uint8_t>(buf.data(), buf.size());
    }
};



template<>
struct _oled_preset<SSD13XX_128X32_Config>{
    static constexpr Vector2i size = Vector2i(128, 32);
    static constexpr Vector2i offset = Vector2i(2, 0);
    static constexpr auto buf = std::to_array<uint8_t>({ 
        0xAE,0x00, 0x10,0x00, 0xB0, 0X81, 0XFF, 0XA1, 0XA6,
        0XA8,0X1F,0XC8,0XD3,0X00, 0XD5, 0X80,0XD9,
        0X1F,0XDA,0X00,0XDB, 0X40, 0X8D, 0X14, 0XAF
    });
    static constexpr std::span<const uint8_t> get_cmds(){

        return std::span<const uint8_t>(buf.data(), buf.size());
    }
};

template<>
struct _oled_preset<SSD13XX_88X48_Config>{
    static constexpr Vector2i size = Vector2i(88, 48);
    static constexpr Vector2i offset = Vector2i(2, 0);
    static constexpr auto buf = std::to_array<uint8_t>({ 
        0xAE,0x00, 0x10,0x00, 0xB0, 0X81, 0XFF, 0XA1, 
        0XA6, 0XA8,0X1F,0XC8,0XD3,0X00, 0XD5, 0X80,
        0XD9, 0X1F,0XDA,0X00,0XDB, 0X40, 0X8D, 0X14, 0XAF
    });
    static constexpr std::span<const uint8_t> get_cmds(){

        return std::span<const uint8_t>(buf.data(), buf.size());
    }
};

template<>
struct _oled_preset<SSD13XX_64X48_Config>{
    static constexpr Vector2i size = Vector2i(64, 48);
    static constexpr Vector2i offset = Vector2i(2, 0);
    static constexpr auto buf = std::to_array<uint8_t>({ 
        0xAE,0x00, 0x10,0x00, 0xB0, 0X81, 0XFF, 0XA1, 
        0XA6, 0XA8,0X1F,0XC8,0XD3,0X00, 0XD5, 0X80,
        0XD9, 0X1F,0XDA,0X00,0XDB, 0X40, 0X8D, 0X14, 0XAF
    });
    static constexpr std::span<const uint8_t> get_cmds(){

        return std::span<const uint8_t>(buf.data(), buf.size());
    }
};


template<>
struct _oled_preset<SSD13XX_128X80_Config>{
    static constexpr Vector2i size = Vector2i(128, 80);
    static constexpr Vector2i offset = Vector2i(2, 0);
    static constexpr auto buf = std::to_array<uint8_t>({ 
        0xAE,0x00, 0x10,0x00, 0xB0, 0X81, 0XFF, 0XA1, 
        0XA6, 0XA8,0X1F,0XC8,0XD3,0X00, 0XD5, 0X80,
        0XD9, 0X1F,0XDA,0X00,0XDB, 0X40, 0X8D, 0X14, 0XAF
    });
    static constexpr std::span<const uint8_t> get_cmds(){

        return std::span<const uint8_t>(buf.data(), buf.size());
    }
};



}
