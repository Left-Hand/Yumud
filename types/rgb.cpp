#include "rgb.h"
#include "sys/stream/ostream.hpp"
// #include <cmath>

using namespace yumud;

#define __SSAT8(x) CLAMP(int8_t(x), -127, 127)
#define __USAT8(x) CLAMP(uint8_t(x), 0, 255)

scexpr real_t xyz_table[256] = {
    real_t(0.000000f ), real_t( 0.030353f), real_t( 0.060705f), real_t( 0.091058f), real_t( 0.121411f), real_t( 0.151763f), real_t( 0.182116f), real_t( 0.212469f),
    real_t(0.242822f ), real_t( 0.273174f), real_t( 0.303527f), real_t( 0.334654f), real_t( 0.367651f), real_t( 0.402472f), real_t( 0.439144f), real_t( 0.477695f),
    real_t(0.518152f ), real_t( 0.560539f), real_t( 0.604883f), real_t( 0.651209f), real_t( 0.699541f), real_t( 0.749903f), real_t( 0.802319f), real_t( 0.856813f),
    real_t(0.913406f ), real_t( 0.972122f), real_t( 1.032982f), real_t( 1.096009f), real_t( 1.161225f), real_t( 1.228649f), real_t( 1.298303f), real_t( 1.370208f),
    real_t(1.444384f ), real_t( 1.520851f), real_t( 1.599629f), real_t( 1.680738f), real_t( 1.764195f), real_t( 1.850022f), real_t( 1.938236f), real_t( 2.028856f),
    real_t(2.121901f ), real_t( 2.217388f), real_t( 2.315337f), real_t( 2.415763f), real_t( 2.518686f), real_t( 2.624122f), real_t( 2.732089f), real_t( 2.842604f),
    real_t(2.955683f ), real_t( 3.071344f), real_t( 3.189603f), real_t( 3.310477f), real_t( 3.433981f), real_t( 3.560131f), real_t( 3.688945f), real_t( 3.820437f),
    real_t(3.954624f ), real_t( 4.091520f), real_t( 4.231141f), real_t( 4.373503f), real_t( 4.518620f), real_t( 4.666509f), real_t( 4.817182f), real_t( 4.970657f),
    real_t(5.126946f ), real_t( 5.286065f), real_t( 5.448028f), real_t( 5.612849f), real_t( 5.780543f), real_t( 5.951124f), real_t( 6.124605f), real_t( 6.301002f),
    real_t(6.480327f ), real_t( 6.662594f), real_t( 6.847817f), real_t( 7.036010f), real_t( 7.227185f), real_t( 7.421357f), real_t( 7.618538f), real_t( 7.818742f),
    real_t(8.021982f ), real_t( 8.228271f), real_t( 8.437621f), real_t( 8.650046f), real_t( 8.865559f), real_t( 9.084171f), real_t( 9.305896f), real_t( 9.530747f),
    real_t(9.758735f ), real_t( 9.989873f), real_t(10.224173f), real_t(10.461648f), real_t(10.702310f), real_t(10.946171f), real_t(11.193243f), real_t(11.443537f),
    real_t(11.697067f), real_t(11.953843f), real_t(12.213877f), real_t(12.477182f), real_t(12.743768f), real_t(13.013648f), real_t(13.286832f), real_t(13.563333f),
    real_t(13.843162f), real_t(14.126329f), real_t(14.412847f), real_t(14.702727f), real_t(14.995979f), real_t(15.292615f), real_t(15.592646f), real_t(15.896084f),
    real_t(16.202938f), real_t(16.513219f), real_t(16.826940f), real_t(17.144110f), real_t(17.464740f), real_t(17.788842f), real_t(18.116424f), real_t(18.447499f),
    real_t(18.782077f), real_t(19.120168f), real_t(19.461783f), real_t(19.806932f), real_t(20.155625f), real_t(20.507874f), real_t(20.863687f), real_t(21.223076f),
    real_t(21.586050f), real_t(21.952620f), real_t(22.322796f), real_t(22.696587f), real_t(23.074005f), real_t(23.455058f), real_t(23.839757f), real_t(24.228112f),
    real_t(24.620133f), real_t(25.015828f), real_t(25.415209f), real_t(25.818285f), real_t(26.225066f), real_t(26.635560f), real_t(27.049779f), real_t(27.467731f),
    real_t(27.889426f), real_t(28.314874f), real_t(28.744084f), real_t(29.177065f), real_t(29.613827f), real_t(30.054379f), real_t(30.498731f), real_t(30.946892f),
    real_t(31.398871f), real_t(31.854678f), real_t(32.314321f), real_t(32.777810f), real_t(33.245154f), real_t(33.716362f), real_t(34.191442f), real_t(34.670406f),
    real_t(35.153260f), real_t(35.640014f), real_t(36.130678f), real_t(36.625260f), real_t(37.123768f), real_t(37.626212f), real_t(38.132601f), real_t(38.642943f),
    real_t(39.157248f), real_t(39.675523f), real_t(40.197778f), real_t(40.724021f), real_t(41.254261f), real_t(41.788507f), real_t(42.326767f), real_t(42.869050f),
    real_t(43.415364f), real_t(43.965717f), real_t(44.520119f), real_t(45.078578f), real_t(45.641102f), real_t(46.207700f), real_t(46.778380f), real_t(47.353150f),
    real_t(47.932018f), real_t(48.514994f), real_t(49.102085f), real_t(49.693300f), real_t(50.288646f), real_t(50.888132f), real_t(51.491767f), real_t(52.099557f),
    real_t(52.711513f), real_t(53.327640f), real_t(53.947949f), real_t(54.572446f), real_t(55.201140f), real_t(55.834039f), real_t(56.471151f), real_t(57.112483f),
    real_t(57.758044f), real_t(58.407842f), real_t(59.061884f), real_t(59.720179f), real_t(60.382734f), real_t(61.049557f), real_t(61.720656f), real_t(62.396039f),
    real_t(63.075714f), real_t(63.759687f), real_t(64.447968f), real_t(65.140564f), real_t(65.837482f), real_t(66.538730f), real_t(67.244316f), real_t(67.954247f),
    real_t(68.668531f), real_t(69.387176f), real_t(70.110189f), real_t(70.837578f), real_t(71.569350f), real_t(72.305513f), real_t(73.046074f), real_t(73.791041f),
    real_t(74.540421f), real_t(75.294222f), real_t(76.052450f), real_t(76.815115f), real_t(77.582222f), real_t(78.353779f), real_t(79.129794f), real_t(79.910274f),
    real_t(80.695226f), real_t(81.484657f), real_t(82.278575f), real_t(83.076988f), real_t(83.879901f), real_t(84.687323f), real_t(85.499261f), real_t(86.315721f),
    real_t(87.136712f), real_t(87.962240f), real_t(88.792312f), real_t(89.626935f), real_t(90.466117f), real_t(91.309865f), real_t(92.158186f), real_t(93.011086f),
    real_t(93.868573f), real_t(94.730654f), real_t(95.597335f), real_t(96.468625f), real_t(97.344529f), real_t(98.225055f), real_t(99.110210f), real_t(100.000000f)
};

template<arithmetic T>
struct XYZ_t{
    T x,y,z;
};

using XYZ = XYZ_t<real_t>;


__fast_inline constexpr static float fast_cbrtf(float x) {
    union {
        int ix; float x;
    }
    v;
    v.x = x;               // x can be viewed as int.
    v.ix = (v.ix >> 2) + (v.ix >> 4); // Approximate divide by 3.
    v.ix = v.ix + (v.ix >> 4);
    v.ix = v.ix + (v.ix >> 8);
    v.ix = 0x2a511cd0 + v.ix; // Initial guess.
    return v.x;
}

__fast_inline constexpr static int fast_floorf(real_t x) {
    return (int)floor(iq_t(x));
}

__fast_inline constexpr static auto xyz_gamma(const real_t x) -> real_t{
    return (x > 0.008856_r) ? iq_t::from(fast_cbrtf(float(x))) : ((x * 7.787037_r) + 0.137931_r);
}

__fast_inline constexpr static auto inv_xyz_gamma_to8(const real_t x) -> uint8_t{
    return __USAT8(fast_floorf((x > 0.0031308_r) ? (((1.055_r * 255) * pow(x, 0.416666_r)) - (0.055_r * 255)) : (x * 12.92_r * 255)));
}

__fast_inline constexpr static XYZ rgb888_to_xyz(const RGB888 & rgb){
    auto [r,g,b] = rgb;
    
    real_t r_lin = xyz_table[r];
    real_t g_lin = xyz_table[g];
    real_t b_lin = xyz_table[b];

    real_t x = ((r_lin * 0.4124_r) + (g_lin * 0.3576_r) + (b_lin * 0.1805_r)) * real_t(1.0f / 095.047f);
    real_t y = ((r_lin * 0.2126_r) + (g_lin * 0.7152_r) + (b_lin * 0.0722_r)) * real_t(1.0f / 100.000f);
    real_t z = ((r_lin * 0.0193_r) + (g_lin * 0.1192_r) + (b_lin * 0.9505_r)) * real_t(1.0f / 108.883f);


    return {xyz_gamma(x), xyz_gamma(y), xyz_gamma(z)};
}

__fast_inline constexpr static XYZ lab888_to_xyz(const LAB888 & lab){
    auto [l,a,b] = lab;
    real_t x = ((l + 16) * 0.008621_r) + (a * 0.002_r);
    real_t y = ((l + 16) * 0.008621_r);
    real_t z = ((l + 16) * 0.008621_r) - (b * 0.005_r);

    x = ((x > 0.206897_r) ? (x * x * x) : ((0.128419_r * x) - 0.017713_r)) * 095.047_r;
    y = ((y > 0.206897_r) ? (y * y * y) : ((0.128419_r * y) - 0.017713_r)) * 100.000_r;
    z = ((z > 0.206897_r) ? (z * z * z) : ((0.128419_r * z) - 0.017713_r)) * 108.883_r;

    return {x,y,z};
}

__fast_inline constexpr static RGB888 xyz_to_rgb888(const XYZ & xyz){
    auto [x,y,z] = xyz;
    real_t r_lin = ((x * (+3.2406_r)) + (y * (-1.5372_r)) + (z * (-0.4986_r))) * (0.01_r);
    real_t g_lin = ((x * (-0.9689_r)) + (y * (+1.8758_r)) + (z * (+0.0415_r))) * (0.01_r);
    real_t b_lin = ((x * (+0.0557_r)) + (y * (-0.2040_r)) + (z * (+1.0570_r))) * (0.01_r);



    return RGB888{
        inv_xyz_gamma_to8(r_lin),
        inv_xyz_gamma_to8(g_lin),
        inv_xyz_gamma_to8(b_lin),
    };
}


__fast_inline constexpr static LAB888 xyz_to_lab888(const XYZ & xyz){
    auto [xf, yf, zf] = xyz;

    return {
            CLAMP(uint8_t(fast_floorf(116 * yf) - 16), 0, 100),
            __SSAT8(int8_t(fast_floorf(500 * (xf - yf)))),
            __SSAT8(int8_t(fast_floorf(200 * (yf - zf))))
    };
}

HSV888::HSV888(const RGB888 & rgb){

    enum{
        HUE_RED = 0,
        HUE_ORANGE = 32,
        HUE_YELLOW = 64,
        HUE_GREEN = 96,
        HUE_AQUA = 128,
        HUE_BLUE = 160,
        HUE_PURPLE = 192,
        HUE_PINK = 224
    };

    auto scale8 = [](const uint8_t i, const uint8_t scale) -> uint8_t {return (((uint16_t)i) * (1+(uint16_t)(scale))) >> 8;};
    auto qsub8 = [](const uint8_t i, const uint8_t j) -> uint8_t {return MAX(int(i) - int(j), 0);};
    auto qadd8 = [](const uint8_t i, const uint8_t j) -> uint8_t {return MIN(int(i) + int(j), 255);};

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
    

    s = 255 - desat;
    
    if( s != 255 ) {
        // undo 'dimming' of saturation
        s = 255 - sqrt16( (255-s) * 256);
    }

    if( (r + g + b) == 0) {
        // we pick hue zero for no special reason
        h = 0;
        s = 0, 
        v = 255 - s;

        return;
    }
    
    // scale all channels up to compensate for desaturation
    if( s < 255) {
        if( s == 0) s = 1;
        uint32_t scaleup = 65535 / (s);
        r = ((uint32_t)(r) * scaleup) / 256;
        g = ((uint32_t)(g) * scaleup) / 256;
        b = ((uint32_t)(b) * scaleup) / 256;
    }
    
    uint16_t total = r + g + b;
    

    if( total < 255) {
        if( total == 0) total = 1;
        uint32_t scaleup = 65535 / (total);
        r = ((uint32_t)(r) * scaleup) / 256;
        g = ((uint32_t)(g) * scaleup) / 256;
        b = ((uint32_t)(b) * scaleup) / 256;
    }
    
    if( total > 255 ) {
        v = 255;
    } else {
        v = qadd8(desat,total);
        // undo 'dimming' of brightness
        if( v != 255) v = sqrt16( v * 256);
        // without lib8tion: real_t ... ew ... sqrt... double ew, or rather, ew ^ 0.5
        // if( v != 255) v = (256.0 * sqrt( (real_t)(v) / 256.0));
        
    }
    
    uint8_t highest = r;
    if( g > highest) highest = g;
    if( b > highest) highest = b;
    
    if( highest == r ) {
        // Red is highest.
        // Hue could be Purple/Pink-Red,Red-Orange,Orange-Yellow
        if( g == 0 ) {
            // if green is zero, we're in Purple/Pink-Red
            h = (HUE_PURPLE + HUE_PINK) / 2;
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
            uint8_t hueadv = rgadj / 2;
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
            h = HUE_AQUA + ((HUE_BLUE - HUE_AQUA) / 4);
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
}

RGB888::RGB888(const HSV888 & hsv){
    
    #define APPLY_DIMMING(X) (X)
    #define HSV_SECTION_6 (0x20)
    #define HSV_SECTION_3 (0x40)
    // Convert hue, saturation and brightness ( HSV/HSB ) to RGB
    // "Dimming" is used on saturation and brightness to make
    // the output more visually linear.

    // Apply dimming curves
    uint8_t value = APPLY_DIMMING( hsv.v);
    uint8_t saturation = hsv.s;

    // The brightness floor is minimum number that all of
    // R, G, and B will be set to.
    uint8_t invsat = APPLY_DIMMING( 255 - saturation);
    uint8_t brightness_floor = (value * invsat) / 256;

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

    uint8_t rampup_amp_adj   = (rampup   * color_amplitude) / (256 / 4);
    uint8_t rampdown_amp_adj = (rampdown * color_amplitude) / (256 / 4);

    // add brightness_floor offset to everything
    uint8_t rampup_adj_with_floor   = rampup_amp_adj   + brightness_floor;
    uint8_t rampdown_adj_with_floor = rampdown_amp_adj + brightness_floor;
    
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

}

RGB565::RGB565(const HSV888 & hsv):RGB565(RGB888(hsv)){;}
HSV888::HSV888(const RGB565 & rgb):HSV888(RGB888(rgb)){;}

LAB888::LAB888(const RGB888 & rgb){
    *this = xyz_to_lab888(rgb888_to_xyz(rgb));
}

LAB888::operator RGB888() const {
    return xyz_to_rgb888(lab888_to_xyz(*this));
}

OutputStream & operator<<(OutputStream & os, const Grayscale & value){
    return os << '(' << uint8_t(value) << ')';
}

OutputStream & operator<<(OutputStream & os, const RGB565 & value){
    return os << '(' << uint8_t(value.r) << ',' << uint8_t(value.g) << 'r' << uint8_t(value.b) << ')';
}

OutputStream & operator<<(OutputStream & os, const RGB888 & value){
    return os << '(' << uint8_t(value.r) << ',' << uint8_t(value.g) << 'r' << uint8_t(value.b) << ')';
}

OutputStream & operator<<(OutputStream & os, const Binary & value){
    return os << '(' << bool(value) << ')';
}
