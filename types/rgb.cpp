#include "rgb.h"
#include "sys/stream/ostream.hpp"
#include <cmath>

#define __SSAT8(x) CLAMP(int8_t(x), -127, 127)
#define __USAT8(x) CLAMP(uint8_t(x), 0, 255)

scexpr float xyz_table[256] = {
    0.000000f,  0.030353f,  0.060705f,  0.091058f,  0.121411f,  0.151763f,  0.182116f,  0.212469f,
    0.242822f,  0.273174f,  0.303527f,  0.334654f,  0.367651f,  0.402472f,  0.439144f,  0.477695f,
    0.518152f,  0.560539f,  0.604883f,  0.651209f,  0.699541f,  0.749903f,  0.802319f,  0.856813f,
    0.913406f,  0.972122f,  1.032982f,  1.096009f,  1.161225f,  1.228649f,  1.298303f,  1.370208f,
    1.444384f,  1.520851f,  1.599629f,  1.680738f,  1.764195f,  1.850022f,  1.938236f,  2.028856f,
    2.121901f,  2.217388f,  2.315337f,  2.415763f,  2.518686f,  2.624122f,  2.732089f,  2.842604f,
    2.955683f,  3.071344f,  3.189603f,  3.310477f,  3.433981f,  3.560131f,  3.688945f,  3.820437f,
    3.954624f,  4.091520f,  4.231141f,  4.373503f,  4.518620f,  4.666509f,  4.817182f,  4.970657f,
    5.126946f,  5.286065f,  5.448028f,  5.612849f,  5.780543f,  5.951124f,  6.124605f,  6.301002f,
    6.480327f,  6.662594f,  6.847817f,  7.036010f,  7.227185f,  7.421357f,  7.618538f,  7.818742f,
    8.021982f,  8.228271f,  8.437621f,  8.650046f,  8.865559f,  9.084171f,  9.305896f,  9.530747f,
    9.758735f,  9.989873f, 10.224173f, 10.461648f, 10.702310f, 10.946171f, 11.193243f, 11.443537f,
    11.697067f, 11.953843f, 12.213877f, 12.477182f, 12.743768f, 13.013648f, 13.286832f, 13.563333f,
    13.843162f, 14.126329f, 14.412847f, 14.702727f, 14.995979f, 15.292615f, 15.592646f, 15.896084f,
    16.202938f, 16.513219f, 16.826940f, 17.144110f, 17.464740f, 17.788842f, 18.116424f, 18.447499f,
    18.782077f, 19.120168f, 19.461783f, 19.806932f, 20.155625f, 20.507874f, 20.863687f, 21.223076f,
    21.586050f, 21.952620f, 22.322796f, 22.696587f, 23.074005f, 23.455058f, 23.839757f, 24.228112f,
    24.620133f, 25.015828f, 25.415209f, 25.818285f, 26.225066f, 26.635560f, 27.049779f, 27.467731f,
    27.889426f, 28.314874f, 28.744084f, 29.177065f, 29.613827f, 30.054379f, 30.498731f, 30.946892f,
    31.398871f, 31.854678f, 32.314321f, 32.777810f, 33.245154f, 33.716362f, 34.191442f, 34.670406f,
    35.153260f, 35.640014f, 36.130678f, 36.625260f, 37.123768f, 37.626212f, 38.132601f, 38.642943f,
    39.157248f, 39.675523f, 40.197778f, 40.724021f, 41.254261f, 41.788507f, 42.326767f, 42.869050f,
    43.415364f, 43.965717f, 44.520119f, 45.078578f, 45.641102f, 46.207700f, 46.778380f, 47.353150f,
    47.932018f, 48.514994f, 49.102085f, 49.693300f, 50.288646f, 50.888132f, 51.491767f, 52.099557f,
    52.711513f, 53.327640f, 53.947949f, 54.572446f, 55.201140f, 55.834039f, 56.471151f, 57.112483f,
    57.758044f, 58.407842f, 59.061884f, 59.720179f, 60.382734f, 61.049557f, 61.720656f, 62.396039f,
    63.075714f, 63.759687f, 64.447968f, 65.140564f, 65.837482f, 66.538730f, 67.244316f, 67.954247f,
    68.668531f, 69.387176f, 70.110189f, 70.837578f, 71.569350f, 72.305513f, 73.046074f, 73.791041f,
    74.540421f, 75.294222f, 76.052450f, 76.815115f, 77.582222f, 78.353779f, 79.129794f, 79.910274f,
    80.695226f, 81.484657f, 82.278575f, 83.076988f, 83.879901f, 84.687323f, 85.499261f, 86.315721f,
    87.136712f, 87.962240f, 88.792312f, 89.626935f, 90.466117f, 91.309865f, 92.158186f, 93.011086f,
    93.868573f, 94.730654f, 95.597335f, 96.468625f, 97.344529f, 98.225055f, 99.110210f, 100.000000f
};

struct xyz_t{
    float x,y,z;
};


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

__fast_inline constexpr static int fast_floorf(float x) {
    return (int)floorf(x);
}

__fast_inline constexpr static auto xyz_gamma(const float x) -> float{
    return (x > 0.008856f) ? fast_cbrtf(x) : ((x * 7.787037f) + 0.137931f);
}

__fast_inline constexpr static auto inv_xyz_gamma_to8(const float x) -> uint8_t{
    return __USAT8(fast_floorf((x > 0.0031308f) ? (((1.055f * 255) * powf(x, 0.416666f)) - (0.055f * 255)) : (x * 12.92f * 255)));
}

__fast_inline constexpr static xyz_t rgb888_to_xyz(const RGB888 & rgb){
    auto [r,g,b] = rgb;

    // int32_t r_q16 = xyz_table_q16[r];
    // int32_t g_q16 = xyz_table_q16[g];
    // int32_t b_q16 = xyz_table_q16[b];

    // float x = Q16TOF((((r_q16 * Q16(0.4124f)) + (g_q16 * Q16(0.3576f)) + (b_q16 * Q16(0.1805f))) >> 16) * Q16(1.0f / 095.047f) >> 16);
    // float y = Q16TOF((((r_q16 * Q16(0.2126f)) + (g_q16 * Q16(0.7152f)) + (b_q16 * Q16(0.0722f))) >> 16) * Q16(1.0f / 100.000f) >> 16);
    // float z = Q16TOF((((r_q16 * Q16(0.0193f)) + (g_q16 * Q16(0.1192f)) + (b_q16 * Q16(0.9505f))) >> 16) * Q16(1.0f / 108.883f) >> 16);
    
    // return {xyz_gamma(x), xyz_gamma(y), xyz_gamma(z)};
    
    float r_lin = xyz_table[r];
    float g_lin = xyz_table[g];
    float b_lin = xyz_table[b];

    float x = ((r_lin * 0.4124f) + (g_lin * 0.3576f) + (b_lin * 0.1805f)) * (1.0f / 095.047f);
    float y = ((r_lin * 0.2126f) + (g_lin * 0.7152f) + (b_lin * 0.0722f)) * (1.0f / 100.000f);
    float z = ((r_lin * 0.0193f) + (g_lin * 0.1192f) + (b_lin * 0.9505f)) * (1.0f / 108.883f);


    return {xyz_gamma(x), xyz_gamma(y), xyz_gamma(z)};
}

__fast_inline constexpr static xyz_t lab888_to_xyz(const LAB888 & lab){
    auto [l,a,b] = lab;
    float x = ((l + 16) * 0.008621f) + (a * 0.002f);
    float y = ((l + 16) * 0.008621f);
    float z = ((l + 16) * 0.008621f) - (b * 0.005f);

    x = ((x > 0.206897f) ? (x * x * x) : ((0.128419f * x) - 0.017713f)) * 095.047f;
    y = ((y > 0.206897f) ? (y * y * y) : ((0.128419f * y) - 0.017713f)) * 100.000f;
    z = ((z > 0.206897f) ? (z * z * z) : ((0.128419f * z) - 0.017713f)) * 108.883f;

    return {x,y,z};
}

__fast_inline constexpr static RGB888 xyz_to_rgb888(const xyz_t & xyz){
    auto [x,y,z] = xyz;
    float r_lin = ((x * +3.2406f) + (y * -1.5372f) + (z * -0.4986f)) * (0.01f);
    float g_lin = ((x * -0.9689f) + (y * +1.8758f) + (z * +0.0415f)) * (0.01f);
    float b_lin = ((x * +0.0557f) + (y * -0.2040f) + (z * +1.0570f)) * (0.01f);



    return RGB888{
        inv_xyz_gamma_to8(r_lin),
        inv_xyz_gamma_to8(g_lin),
        inv_xyz_gamma_to8(b_lin),
    };
}


__fast_inline constexpr static LAB888 xyz_to_lab888(const xyz_t & xyz){
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
        // without lib8tion: float ... ew ... sqrt... double ew, or rather, ew ^ 0.5
        // if( v != 255) v = (256.0 * sqrt( (float)(v) / 256.0));
        
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
