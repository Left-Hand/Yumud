#pragma once

#include "core/platform.hpp"
#include "core/int/uint24_t.hpp"
#include "core/math/real.hpp"
#include <tuple>

namespace ymd{
enum class [[nodiscard]] ColorEnum:uint32_t{
    // 基础颜色
    WHITE       = 0xFFFFFF,    // 白色
    BLACK       = 0x000000,    // 黑色
    RED         = 0xFF0000,    // 红色
    GREEN       = 0x00FF00,    // 绿色 (注意：原值0x00FF00对应的是绿色)
    BLUE        = 0x0000FF,    // 蓝色
    YELLOW      = 0xFFFF00,    // 黄色
    BRRED       = 0xFF4500,    // 棕红色
    PINK        = 0xFFC0CB,    // 粉色
    BROWN       = 0xA52A2A,    // 棕色
    GRAY        = 0x808080,    // 灰色
    GBLUE       = 0xADD8E6,    // 淡蓝色

    // 清新色调
    LIGHT_BLUE  = 0x87CEEB,    // 天蓝色
    SKY_BLUE    = 0x87CEFA,    // 倾斜天蓝色
    AQUA        = 0x00FFFF,    // 青色
    TURQUOISE   = 0x40E0D0,    // 绿松石色
    MINT_GREEN  = 0x98FF98,    // 薄荷绿色
    PASTEL_GREEN= 0x77DD77,    // 淡绿色
    LAVENDER    = 0xE6E6FA,    // 薰衣草色
    CORAL       = 0xFF7F50,    // 珊瑚色
    SALMON      = 0xFA8072,    // 鲑鱼色
    PEACH       = 0xFFDAB9,    // 桃色
    LIGHT_PINK  = 0xFFB6C1,    // 浅粉色
    PASTEL_PINK = 0xFFD1DC,    // 淡粉色
    LEMON_CHIFFON = 0xFFFACD,  // 柠檬纱色
    LIGHT_YELLOW = 0xFFFFE0,   // 浅黄色
    HONEYDEW    = 0xF0FFF0,    // 蜜瓜色
    MISTY_ROSE  = 0xFFE4E1,    // 紫色玫瑰色
    ALICE_BLUE  = 0xF0F8FF,    // 爱丽丝蓝
    SEASHELL    = 0xFFF5EE,    // 海贝色
    LINEN       = 0xFAF0E6,    // 亚麻色
    

    // 其他常用色
    CYAN        = 0x00FFFF,    // 青色
    MAGENTA     = 0xFF00FF,    // 洋红色
    ORANGE      = 0xFFA500,    // 橙色
    PURPLE      = 0x800080,    // 紫色
    LIME        = 0x00FF00,    // 酸橙色
    TEAL        = 0x008080,    // 蓝绿色
    NAVY        = 0x000080,    // 海军蓝
    MAROON      = 0x800000,    // 栗色
    OLIVE       = 0x808000     // 橄榄色
};

enum class [[nodiscard]] RgbType:uint8_t{
    _RGB332,
    _RGB565,
    _RGB888,
    _Binary,
    _HSV888,
    _LAB888,
    _XYZ888,
    _Gray,
    _IGray,
};



struct RGB332;
struct RGB565;
struct RGB888;
struct Binary;
struct HSV888;
struct LAB888;
struct XYZ888;
struct Gray;
struct IGray;

template<typename To, typename From>
struct [[nodiscard]] ColorCaster;


template<typename To, typename From>
static constexpr To color_cast(From && from){
    if constexpr (std::is_same_v<To, std::decay_t<From>>) {
        return from;
    }else{
        return ColorCaster<To, std::decay_t<From>>::cast(from);
    }
}

struct [[nodiscard]] RGB888 final{

    uint8_t b;
    uint8_t g;
    uint8_t r;

    static constexpr RGB888 from_r8g8b8(uint8_t r, uint8_t g, uint8_t b){
        return RGB888{
            .b = b,
            .g = g,
            .r = r
        };
    }

    static constexpr RGB888 from_u32(const uint32_t _data){
        return RGB888{
            .b = (static_cast<uint8_t>(_data & 0xff)),
            .g = (static_cast<uint8_t>(_data >> 8)),
            .r = (static_cast<uint8_t>(_data >> 16))
        };
    }

    [[nodiscard]] constexpr math::uint24_t as_u24() const {return math::uint24_t(r | (g << 8) | (b << 16));}
};

static_assert(sizeof(RGB888) == 3);
struct [[nodiscard]] LAB888 final{

    uint8_t l;
    int8_t a;
    int8_t b;

public:

    [[nodiscard]] constexpr math::uint24_t as_u24() const {return math::uint24_t(l | (a << 8) | (b << 16));}

    static constexpr LAB888 from_l8a8b8(uint8_t l, uint8_t a, uint8_t b){
        return LAB888(l, a, b);
    } 
private:
    constexpr explicit LAB888(const uint8_t _l, const int8_t _a, const int8_t _b):
        l(_l), a(_a), b(_b){;}
};


static_assert(sizeof(RGB888) == 3);

struct [[nodiscard]] RGB332 final{
    using Self = RGB332;
    union{
        struct{
            uint8_t b : 2;
            uint8_t g : 3;
            uint8_t r : 3;
        };
        uint8_t bits;
    };


    constexpr explicit RGB332(){;}

    static constexpr Self from_bits(const uint8_t bits){
        return std::bit_cast<Self>(bits);
    }

    static constexpr Self from_r3g3b2(uint8_t r, uint8_t g, uint8_t b){
        return Self(r,g,b);
    } 

    [[nodiscard]] constexpr uint8_t to_u8() const {return bits;}

private:
    constexpr explicit RGB332(const uint8_t _r, const uint8_t _g, const uint8_t _b): 
        b(_b), g(_g), r(_r){;}
};

static_assert(sizeof(RGB332) == 1);

struct alignas(2) [[nodiscard]] RGB565 final{
    uint16_t b : 5;
    uint16_t g : 6;
    uint16_t r : 5;

    static constexpr 
    RGB565 from_r5g6b5(const uint8_t r, const uint8_t g, const uint8_t b){
        return RGB565{
            .b = static_cast<uint16_t>(b & 0b11111), 
            .g = static_cast<uint16_t>(g & 0b111111), 
            .r = static_cast<uint16_t>(r & 0b11111)
        };
    }

    static constexpr 
    RGB565 from_u16(const uint16_t raw){
        return std::bit_cast<RGB565>(raw);
    }
    [[nodiscard]] constexpr uint16_t to_u16() const {
        return std::bit_cast<uint16_t>(*this);
    }
private:
    static constexpr std::tuple<uint8_t, uint8_t, uint8_t>
    seprate(const uint16_t bits){
        return {(bits >> 11) & 0x1f, (bits >> 5) & 0x3f, bits & 0x1f};}
    static constexpr uint16_t uni(const uint8_t _r, const uint8_t _g, const uint8_t _b){
        return ((_r & 0x1f) << 11) | ((_g & 0x3f) << 5) | (_b & 0x1f);}

};

static_assert(sizeof(RGB565) == 2);


struct [[nodiscard]] HSV888 final{

    uint8_t h;
    uint8_t s;
    uint8_t v;


    static constexpr HSV888 from_h8s8v8(
        const uint8_t _h, const uint8_t _s, const uint8_t _v
    ){
        return HSV888{
            .h = _h,
            .s = _s,
            .v = _v
        };
    }

    [[nodiscard]] constexpr math::uint24_t as_u24() const {
        return math::uint24_t(uint32_t(h) << 16 | uint32_t(s) << 8 | uint32_t(v));
    }

};

static_assert(sizeof(HSV888) == 3);

struct alignas(4) [[nodiscard]] ARGB32 final{

    uint8_t b;
    uint8_t g;
    uint8_t r;
    uint8_t a;


    [[nodiscard]] constexpr ARGB32 from_a8r8g8b8(
        const uint8_t _a, const uint8_t _r, const uint8_t _g, const uint8_t _b){
            return ARGB32{
                .b = _b,
                .g = _g, 
                .r = _r, 
                .a = _a
            };
        }

    [[nodiscard]] constexpr uint32_t to_u32() const {
        return std::bit_cast<uint32_t>(*this);
    }

};

static_assert(sizeof(ARGB32) == 4);

struct [[nodiscard]] Binary final{

    enum Kind:uint8_t{
        WHITE   = 0xFF,  // White color
        BLACK   = 0x00   // Black color
    };


    constexpr explicit Binary(){;}
    
    [[nodiscard]] static constexpr Binary from_bool(const bool b){
        return Binary(b ? 0xff : 0x00);
    }

    [[nodiscard]] static constexpr Binary black(){
        return Binary(static_cast<uint8_t>(BLACK));
    }

    [[nodiscard]] static constexpr Binary white(){
        return Binary(static_cast<uint8_t>(WHITE));
    }

    [[nodiscard]] constexpr bool operator ==(const Binary& rhs){
        return bits == rhs.bits;
    }
    [[nodiscard]] constexpr bool operator !=(const Binary& rhs){
        return bits != rhs.bits;
    }

    [[nodiscard]] constexpr Binary operator ~() const {return flip();}

    [[nodiscard]] constexpr Binary bitwise_or(const Binary & other) const { 
        return Binary(bits | other.bits);
    }

    [[nodiscard]] constexpr Binary bitwise_and(const Binary & other) const { 
        return Binary(bits & other.bits);
    }
    [[nodiscard]] constexpr bool is_white() const {return bits == WHITE;}

    [[nodiscard]] constexpr bool is_black() const {return bits == BLACK;}

    [[nodiscard]] constexpr Binary flip() const {
        const uint8_t ret = ~bits;
        return Binary(uint8_t(ret));
    }

    [[nodiscard]] constexpr uint8_t to_u8() const {return bits;}


private:
    uint8_t bits;

    constexpr explicit Binary(const uint8_t cu8) : bits(cu8){;}
};

static_assert(sizeof(Binary) == 1);

struct [[nodiscard]] Gray final{

    enum{
        WHITE   = 0xFF,  // White color
        BLACK   = 0x00   // Black color
    };

    constexpr explicit Gray(){;}

    [[nodiscard]] static constexpr Gray from_u8(const uint8_t _data){
        return Gray{_data};
    }

    [[nodiscard]] static constexpr Gray black(){
        return Gray{BLACK};
    }

    [[nodiscard]] static constexpr Gray white(){
        return Gray{WHITE};
    }

    [[nodiscard]] constexpr RGB888 to_rgb888(){
        return RGB888::from_r8g8b8(bits, bits, bits);
    }

    [[nodiscard]] constexpr auto operator <=> (const Gray & other) const {
        return bits <=> other.bits;}
    [[nodiscard]] constexpr bool operator == (const Gray & other) const {
        return bits == other.bits;}


    [[nodiscard]] constexpr bool is_white() const {return bits == uint8_t(0xff);}

    [[nodiscard]] constexpr bool is_black() const {return bits == uint8_t(0x00);}

    [[nodiscard]] constexpr uint8_t to_u8() const {return bits;}

    [[nodiscard]] constexpr Gray flip() const {
        const uint8_t ret = ~bits;
        return Gray(uint8_t(ret));
    }

    [[nodiscard]] constexpr Binary to_binary(const Gray threshold) const 
        {return Binary::from_bool(bits > threshold.to_u8());}
private:
    uint8_t bits;
    [[nodiscard]] constexpr explicit Gray(const uint8_t cu8) : bits(cu8){;}
};

static_assert(sizeof(Gray) == 1);

struct [[nodiscard]] IGray final{
    enum{
        WHITE   = 127,  // White color
        BLACK   = 0x00   // Black color
    };

    [[nodiscard]] static constexpr IGray from_i8(const int8_t value) {
        return IGray(value);
    }

    [[nodiscard]] static constexpr IGray white() { return IGray(WHITE); }
    [[nodiscard]] static constexpr IGray black() { return IGray(BLACK); }

    [[nodiscard]] constexpr int8_t as_i8() const {return bits;}
    [[nodiscard]] constexpr auto operator <=> (const IGray & other) const {
        return bits <=> other.bits;}

    [[nodiscard]] constexpr Binary to_binary(const Gray threshold){
        return Binary::from_bool(ABS(bits) > threshold.to_u8());}

    [[nodiscard]] constexpr Binary to_binary_signed(const IGray threshold){
        return Binary::from_bool(bits > threshold.as_i8());}

private:
    constexpr IGray(int8_t _data) : bits(_data){}
    int8_t bits;
};

static_assert(sizeof(IGray) == 1);


using RGB24 = RGB888;


template<>
struct [[nodiscard]] ColorCaster<Gray, RGB565>{
    static constexpr Gray cast(const RGB565 & color){
        uint32_t sum = 0;
        sum += static_cast<uint32_t>(color.r) * uint32_t(77 * 8) ;
        sum += static_cast<uint32_t>(color.g) * uint32_t(150 * 4) ;
        sum += static_cast<uint32_t>(color.b) * uint32_t(29 * 8);

        return Gray::from_u8(static_cast<uint8_t>(sum >> 8));
    }
};

template<>
struct [[nodiscard]] ColorCaster<Gray, RGB888>{
    static constexpr Gray cast(const RGB888 & color){
        return Gray::from_u8(static_cast<uint8_t>((color.r*77 + color.g*150 + color.b*29) >> 8));
    }
};

template<>
struct [[nodiscard]] ColorCaster<RGB565, RGB888> {
    static constexpr RGB565 cast(const RGB888 & color){
        return RGB565::from_r5g6b5(color.r >> 3, color.g >> 2, color.b >> 3);
    }
};



template<>
struct [[nodiscard]] ColorCaster<RGB565, Binary> {
    static constexpr RGB565 cast(const Binary & color){
        if(not color.is_white()) return RGB565::from_u16(0); 
        return RGB565::from_u16(0xffff);
    }
};


template<>
struct [[nodiscard]] ColorCaster<RGB565, Gray> {
    static constexpr RGB565 cast(const Gray & color){
        const auto cu8 = color.to_u8();
        return RGB565::from_r5g6b5(
            static_cast<uint8_t>(cu8 >> 3), 
            static_cast<uint8_t>(cu8 >> 2), 
            static_cast<uint8_t>(cu8 >> 3)
        );
    }
};

template<>
struct [[nodiscard]] ColorCaster<RGB565, ColorEnum> {
    static constexpr RGB565 cast(const ColorEnum & color){
        return color_cast<RGB565>(color_cast<RGB888>(color));
    }
};

template<>
struct [[nodiscard]] ColorCaster<RGB888, RGB565> {
    static constexpr RGB888 cast(const RGB565 & color){
        return RGB888::from_r8g8b8(
            static_cast<uint8_t>(color.r) << 3, 
            static_cast<uint8_t>(color.g) << 2, 
            static_cast<uint8_t>(color.b) << 3
        );
    }
};

template<>
struct [[nodiscard]] ColorCaster<RGB888, Binary> {
    static constexpr RGB888 cast(const Binary & color){
        if(not color.is_white()) return RGB888::from_u32(0); 
        const uint8_t cu8 = color.is_white() ? 0xff : 0;
        return RGB888::from_r8g8b8(cu8, cu8, cu8);
    }
};


template<>
struct [[nodiscard]] ColorCaster<RGB888, Gray> {
    static constexpr RGB888 cast(const Gray & color){
        const auto cu8 = color.to_u8();
        return RGB888::from_r8g8b8(cu8, cu8, cu8);
    }
};



template<>
struct [[nodiscard]] ColorCaster<HSV888, RGB888> {

    static constexpr uint8_t HUE_RED = 0;
    static constexpr uint8_t HUE_ORANGE = 32;
    static constexpr uint8_t HUE_YELLOW = 64;
    static constexpr uint8_t HUE_GREEN = 96;
    static constexpr uint8_t HUE_AQUA = 128;
    static constexpr uint8_t HUE_BLUE = 160;
    static constexpr uint8_t HUE_PURPLE = 192;
    static constexpr uint8_t HUE_PINK = 224;

    __attribute__((always_inline))
    [[nodiscard]] static constexpr uint8_t scale8(const uint8_t i, const uint8_t scale){
        return (((uint16_t)i) * (1+(uint16_t)(scale))) >> 8;
    };

    __attribute__((always_inline))
    [[nodiscard]] static constexpr uint8_t qsub8(const uint8_t i, const uint8_t j){
        return MAX(int(i) - int(j), 0);
    };

    __attribute__((always_inline))
    [[nodiscard]] static constexpr uint8_t qadd8(const uint8_t i, const uint8_t j){
        return MIN(int(i) + int(j), 255);
    };

    static constexpr HSV888 cast(const RGB888 & rgb){
        #define FIXFRAC8(N,D) (((N)*256)/(D))

        auto sqrt16 = [](uint16_t x) -> uint8_t
        {
            if( x <= 1) {
                return x;
            }

            uint8_t low = 1; // lower bound
            uint8_t hi, mid;

            if( x > 7904) {
                hi = 255;
            } else {
                hi = (x >> 5) + 8; // initial estimate for upper bound
            }

            do {
                mid = (low + hi) >> 1;
                if ((uint16_t)(mid * mid) > x) {
                    hi = mid - 1;
                } else {
                    if( mid == 255) {
                        return 255;
                    }
                    low = mid + 1;
                }
            } while (hi >= low);

            return low - 1;
        };

        
        uint8_t r = rgb.r;
        uint8_t g = rgb.g;
        uint8_t b = rgb.b;
        
        // find desaturation
        uint8_t desat = 255;
        if( r < desat) desat = r;
        if( g < desat) desat = g;
        if( b < desat) desat = b;
        
        // remove saturation from all channels
        r -= desat;
        g -= desat;
        b -= desat;
        
        uint8_t h, s, v;

        s = 255 - desat;
        
        if( s != 255 ) {
            // undo 'dimming' of saturation
            s = 255 - sqrt16(uint16_t(255-s) << 8);
        }

        if( (r + g + b) == 0) {
            // we pick hue zero for no special reason
            h = 0;
            s = 0, 
            v = 255 - s;

            return HSV888::from_h8s8v8(h,s,v);
        }
        
        // scale all channels up to compensate for desaturation
        if( s < 255) {
            if( s == 0) s = 1;
            uint32_t scaleup = 65535 / (s);
            r = ((uint32_t)(r) * scaleup) >> 8;
            g = ((uint32_t)(g) * scaleup) >> 8;
            b = ((uint32_t)(b) * scaleup) >> 8;
        }
        
        uint16_t total = r + g + b;
        

        if( total < 255) {
            if( total == 0) total = 1;
            uint32_t scaleup = 65535 / (total);
            r = ((uint32_t)(r) * scaleup) >> 8;
            g = ((uint32_t)(g) * scaleup) >> 8;
            b = ((uint32_t)(b) * scaleup) >> 8;
        }
        
        if( total > 255 ) {
            v = 255;
        } else {
            v = qadd8(desat,total);
            // undo 'dimming' of brightness
            if( v != 255) v = sqrt16(uint16_t(v) << 8);
            // without lib8tion: iq16 ... ew ... sqrt... double ew, or rather, ew ^ 0.5
            // if( v != 255) v = (256.0 * sqrt( (iq16)(v) / 256.0));
            
        }
        
        uint8_t highest = r;
        if( g > highest) highest = g;
        if( b > highest) highest = b;
        
        if( highest == r ) {
            // Red is highest.
            // Hue could be Purple/Pink-Red,Red-Orange,Orange-Yellow
            if( g == 0 ) {
                // if green is zero, we're in Purple/Pink-Red
                h = (HUE_PURPLE + HUE_PINK) >> 1;
                h += scale8( qsub8(r, 128), FIXFRAC8(48,128));
            } else if ( (r - g) > g) {
                // if R-G > G then we're in Red-Orange
                h = HUE_RED;
                h += scale8( g, FIXFRAC8(32,85));
            } else {
                // R-G < G, we're in Orange-Yellow
                h = HUE_ORANGE;
                h += scale8( qsub8((g - 85) + (171 - r), 4), FIXFRAC8(32,85)); //221
            }
            
        } else if ( highest == g) {
            // Green is highest
            // Hue could be Yellow-Green, Green-Aqua
            if( b == 0) {
                // if Blue is zero, we're in Yellow-Green
                //   G = 171..255
                //   R = 171..  0
                h = HUE_YELLOW;
                uint8_t radj = scale8( qsub8(171,r),   47); //171..0 -> 0..171 -> 0..31
                uint8_t gadj = scale8( qsub8(g,171),   96); //171..255 -> 0..84 -> 0..31;
                uint8_t rgadj = radj + gadj;
                uint8_t hueadv = rgadj >> 1;
                h += hueadv;
                //h += scale8( qadd8( 4, qadd8((g - 128), (128 - r))),
                //             FIXFRAC8(32,255)); //
            } else {
                // if Blue is nonzero we're in Green-Aqua
                if( (g-b) > b) {
                    h = HUE_GREEN;
                    h += scale8( b, FIXFRAC8(32,85));
                } else {
                    h = HUE_AQUA;
                    h += scale8( qsub8(b, 85), FIXFRAC8(8,42));
                }
            }
            
        } else /* highest == b */ {
            // Blue is highest
            // Hue could be Aqua/Blue-Blue, Blue-Purple, Purple-Pink
            if( r == 0) {
                // if red is zero, we're in Aqua/Blue-Blue
                h = HUE_AQUA + ((HUE_BLUE - HUE_AQUA) >> 2);
                h += scale8( qsub8(b, 128), FIXFRAC8(24,128));
            } else if ( (b-r) > r) {
                // B-R > R, we're in Blue-Purple
                h = HUE_BLUE;
                h += scale8( r, FIXFRAC8(32,85));
            } else {
                // B-R < R, we're in Purple-Pink
                h = HUE_PURPLE;
                h += scale8( qsub8(r, 85), FIXFRAC8(32,85));
            }
        }
        
        h += 1;

        #undef FIXFRAC8

        return HSV888::from_h8s8v8(h,s,v);
    }
};

template<>
struct [[nodiscard]] ColorCaster<RGB888, ColorEnum>{
    static constexpr RGB888 cast(const ColorEnum from){
        return RGB888::from_u32(std::bit_cast<uint32_t>(from));
    }
};

template<>
struct [[nodiscard]] ColorCaster<Gray, ColorEnum>{
    static constexpr Gray cast(const ColorEnum from){
        const auto rgb = RGB888::from_u32(std::bit_cast<uint32_t>(from));
        return Gray::from_u8(static_cast<uint8_t>((rgb.r*77 + rgb.g*150 + rgb.b*29) >> 8));
    }
};


template<>
struct [[nodiscard]] ColorCaster<RGB888, HSV888> {

    static constexpr uint8_t HSV_SECTION_6 = (0x20);
    static constexpr uint8_t HSV_SECTION_3 = (0x40);


    static constexpr RGB888 cast(const HSV888 & hsv){
        // Convert hue, saturation and brightness ( HSV/HSB ) to RGB
        // "Dimming" is used on saturation and brightness to make
        // the output more visually linear.

        // Apply dimming curves
        uint8_t value = ( hsv.v);
        uint8_t saturation = hsv.s;

        // The brightness floor is minimum number that all of
        // R, G, and B will be set to.
        uint8_t invsat = ( 255 - saturation);
        uint8_t brightness_floor = (value * invsat) >> 8;

        // The color amplitude is the maximum amount of R, G, and B
        // that will be added on top of the brightness_floor to
        // create the specific hue desired.
        uint8_t color_amplitude = value - brightness_floor;

        // Figure out which section of the hue wheel we're in,
        // and how far offset we are withing that section
        uint8_t section = hsv.h / HSV_SECTION_3; // 0..2
        uint8_t offset = hsv.h % HSV_SECTION_3;  // 0..63

        uint8_t rampup = offset; // 0..63
        uint8_t rampdown = (HSV_SECTION_3 - 1) - offset; // 63..0

        uint8_t rampup_amp_adj   = (rampup   * color_amplitude) >> 6;
        uint8_t rampdown_amp_adj = (rampdown * color_amplitude) >> 6;

        // add brightness_floor offset to everything
        uint8_t rampup_adj_with_floor   = rampup_amp_adj   + brightness_floor;
        uint8_t rampdown_adj_with_floor = rampdown_amp_adj + brightness_floor;
        
        uint8_t r, g, b;
        if(section) {
            if( section == 1) {
                // section 1: 0x40..0x7F
                r = brightness_floor;
                g = rampdown_adj_with_floor;
                b = rampup_adj_with_floor;
            } else {
                // section 2; 0x80..0xBF
                r = rampup_adj_with_floor;
                g = brightness_floor;
                b = rampdown_adj_with_floor;
            }
        } else {
            // section 0: 0x00..0x3F
            r = rampdown_adj_with_floor;
            g = rampup_adj_with_floor;
            b = brightness_floor;
        }
        return RGB888::from_r8g8b8(r,g,b);
    }
};


class OutputStream;


OutputStream & operator<<(OutputStream & os, const Binary & bn);

OutputStream & operator<<(OutputStream & os, const Gray & gs);

OutputStream & operator<<(OutputStream & os, const IGray & sgs);

OutputStream & operator<<(OutputStream & os, const RGB565 & rgb);

OutputStream & operator<<(OutputStream & os, const RGB888 & rgb);

OutputStream & operator<<(OutputStream & os, const LAB888 & lab);

OutputStream & operator<<(OutputStream & os, const HSV888 & hsv);

template<typename T>
concept is_monochrome = std::is_same_v<T, Binary> or std::is_same_v<T, Gray> or std::is_same_v<T, IGray>;

template<typename T>
concept is_rgb = std::is_same_v<T, RGB24> or std::is_same_v<T, RGB332> or std::is_same_v<T, RGB565> or std::is_same_v<T, RGB888> ;

template<typename T>
concept is_polychrome = is_rgb<T> or std::is_same_v<T, LAB888> or std::is_same_v<T, HSV888>;

template<typename T>
concept is_color = is_monochrome<T> or is_polychrome<T> or std::is_same_v<T, ColorEnum>;

}
