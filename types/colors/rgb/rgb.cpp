#include "rgb.hpp"
#include "core/stream/ostream.hpp"
#include "core/math/realmath.hpp"



//https://www.zhihu.com/question/27417946/answer/1253126563

static inline int32_t clamp_to_0(int32_t x) { 
	return ((-x) >> 31) & x; 
}
static inline int32_t clamp_to_255(int32_t x) {
	return (((255 - x) >> 31) | x) & 255;
}

static inline uint32_t next_power_of_2(uint32_t x) {
	x--;
	x |= x >> 1; 
	x |= x >> 2; 
	x |= x >> 4; 
	x |= x >> 8; 
	x |= x >> 16; 
	x++;
	return x;
}

// if ((int32_t)(((uint32_t)x - (uint32_t)minx) | ((uint32_t)maxx - (uint32_t)x)) > = 0)


#define __SSAT8(x) CLAMP(int8_t(x), -127, 127)
#define __USAT8(x) CLAMP(uint8_t(x), 0, 255)

static constexpr uint16_t xyz_table[256] = {
    uint16_t(0.000000f  * 256), uint16_t( 0.030353f * 256), uint16_t( 0.060705f * 256), uint16_t( 0.091058f * 256), 
    uint16_t( 0.121411f * 256), uint16_t( 0.151763f * 256), uint16_t( 0.182116f * 256), uint16_t( 0.212469f * 256),
    uint16_t(0.242822f  * 256), uint16_t( 0.273174f * 256), uint16_t( 0.303527f * 256), uint16_t( 0.334654f * 256), 
    uint16_t( 0.367651f * 256), uint16_t( 0.402472f * 256), uint16_t( 0.439144f * 256), uint16_t( 0.477695f * 256),
    uint16_t(0.518152f  * 256), uint16_t( 0.560539f * 256), uint16_t( 0.604883f * 256), uint16_t( 0.651209f * 256), 
    uint16_t( 0.699541f * 256), uint16_t( 0.749903f * 256), uint16_t( 0.802319f * 256), uint16_t( 0.856813f * 256),
    uint16_t(0.913406f  * 256), uint16_t( 0.972122f * 256), uint16_t( 1.032982f * 256), uint16_t( 1.096009f * 256), 
    uint16_t( 1.161225f * 256), uint16_t( 1.228649f * 256), uint16_t( 1.298303f * 256), uint16_t( 1.370208f * 256),
    uint16_t(1.444384f  * 256), uint16_t( 1.520851f * 256), uint16_t( 1.599629f * 256), uint16_t( 1.680738f * 256), 
    uint16_t( 1.764195f * 256), uint16_t( 1.850022f * 256), uint16_t( 1.938236f * 256), uint16_t( 2.028856f * 256),
    uint16_t(2.121901f  * 256), uint16_t( 2.217388f * 256), uint16_t( 2.315337f * 256), uint16_t( 2.415763f * 256), 
    uint16_t( 2.518686f * 256), uint16_t( 2.624122f * 256), uint16_t( 2.732089f * 256), uint16_t( 2.842604f * 256),
    uint16_t(2.955683f  * 256), uint16_t( 3.071344f * 256), uint16_t( 3.189603f * 256), uint16_t( 3.310477f * 256), 
    uint16_t( 3.433981f * 256), uint16_t( 3.560131f * 256), uint16_t( 3.688945f * 256), uint16_t( 3.820437f * 256),
    uint16_t(3.954624f  * 256), uint16_t( 4.091520f * 256), uint16_t( 4.231141f * 256), uint16_t( 4.373503f * 256), 
    uint16_t( 4.518620f * 256), uint16_t( 4.666509f * 256), uint16_t( 4.817182f * 256), uint16_t( 4.970657f * 256),
    uint16_t(5.126946f  * 256), uint16_t( 5.286065f * 256), uint16_t( 5.448028f * 256), uint16_t( 5.612849f * 256), 
    uint16_t( 5.780543f * 256), uint16_t( 5.951124f * 256), uint16_t( 6.124605f * 256), uint16_t( 6.301002f * 256),
    uint16_t(6.480327f  * 256), uint16_t( 6.662594f * 256), uint16_t( 6.847817f * 256), uint16_t( 7.036010f * 256), 
    uint16_t( 7.227185f * 256), uint16_t( 7.421357f * 256), uint16_t( 7.618538f * 256), uint16_t( 7.818742f * 256),
    uint16_t(8.021982f  * 256), uint16_t( 8.228271f * 256), uint16_t( 8.437621f * 256), uint16_t( 8.650046f * 256), 
    uint16_t( 8.865559f * 256), uint16_t( 9.084171f * 256), uint16_t( 9.305896f * 256), uint16_t( 9.530747f * 256),
    uint16_t(9.758735f  * 256), uint16_t( 9.989873f * 256), uint16_t(10.224173f * 256), uint16_t(10.461648f * 256), 
    uint16_t(10.702310f * 256), uint16_t(10.946171f * 256), uint16_t(11.193243f * 256), uint16_t(11.443537f * 256),
    uint16_t(11.697067f * 256), uint16_t(11.953843f * 256), uint16_t(12.213877f * 256), uint16_t(12.477182f * 256), 
    uint16_t(12.743768f * 256), uint16_t(13.013648f * 256), uint16_t(13.286832f * 256), uint16_t(13.563333f * 256),
    uint16_t(13.843162f * 256), uint16_t(14.126329f * 256), uint16_t(14.412847f * 256), uint16_t(14.702727f * 256), 
    uint16_t(14.995979f * 256), uint16_t(15.292615f * 256), uint16_t(15.592646f * 256), uint16_t(15.896084f * 256),
    uint16_t(16.202938f * 256), uint16_t(16.513219f * 256), uint16_t(16.826940f * 256), uint16_t(17.144110f * 256), 
    uint16_t(17.464740f * 256), uint16_t(17.788842f * 256), uint16_t(18.116424f * 256), uint16_t(18.447499f * 256),
    uint16_t(18.782077f * 256), uint16_t(19.120168f * 256), uint16_t(19.461783f * 256), uint16_t(19.806932f * 256), 
    uint16_t(20.155625f * 256), uint16_t(20.507874f * 256), uint16_t(20.863687f * 256), uint16_t(21.223076f * 256),
    uint16_t(21.586050f * 256), uint16_t(21.952620f * 256), uint16_t(22.322796f * 256), uint16_t(22.696587f * 256), 
    uint16_t(23.074005f * 256), uint16_t(23.455058f * 256), uint16_t(23.839757f * 256), uint16_t(24.228112f * 256),
    uint16_t(24.620133f * 256), uint16_t(25.015828f * 256), uint16_t(25.415209f * 256), uint16_t(25.818285f * 256), 
    uint16_t(26.225066f * 256), uint16_t(26.635560f * 256), uint16_t(27.049779f * 256), uint16_t(27.467731f * 256),
    uint16_t(27.889426f * 256), uint16_t(28.314874f * 256), uint16_t(28.744084f * 256), uint16_t(29.177065f * 256), 
    uint16_t(29.613827f * 256), uint16_t(30.054379f * 256), uint16_t(30.498731f * 256), uint16_t(30.946892f * 256),
    uint16_t(31.398871f * 256), uint16_t(31.854678f * 256), uint16_t(32.314321f * 256), uint16_t(32.777810f * 256), 
    uint16_t(33.245154f * 256), uint16_t(33.716362f * 256), uint16_t(34.191442f * 256), uint16_t(34.670406f * 256),
    uint16_t(35.153260f * 256), uint16_t(35.640014f * 256), uint16_t(36.130678f * 256), uint16_t(36.625260f * 256), 
    uint16_t(37.123768f * 256), uint16_t(37.626212f * 256), uint16_t(38.132601f * 256), uint16_t(38.642943f * 256),
    uint16_t(39.157248f * 256), uint16_t(39.675523f * 256), uint16_t(40.197778f * 256), uint16_t(40.724021f * 256), 
    uint16_t(41.254261f * 256), uint16_t(41.788507f * 256), uint16_t(42.326767f * 256), uint16_t(42.869050f * 256),
    uint16_t(43.415364f * 256), uint16_t(43.965717f * 256), uint16_t(44.520119f * 256), uint16_t(45.078578f * 256), 
    uint16_t(45.641102f * 256), uint16_t(46.207700f * 256), uint16_t(46.778380f * 256), uint16_t(47.353150f * 256),
    uint16_t(47.932018f * 256), uint16_t(48.514994f * 256), uint16_t(49.102085f * 256), uint16_t(49.693300f * 256), 
    uint16_t(50.288646f * 256), uint16_t(50.888132f * 256), uint16_t(51.491767f * 256), uint16_t(52.099557f * 256),
    uint16_t(52.711513f * 256), uint16_t(53.327640f * 256), uint16_t(53.947949f * 256), uint16_t(54.572446f * 256), 
    uint16_t(55.201140f * 256), uint16_t(55.834039f * 256), uint16_t(56.471151f * 256), uint16_t(57.112483f * 256),
    uint16_t(57.758044f * 256), uint16_t(58.407842f * 256), uint16_t(59.061884f * 256), uint16_t(59.720179f * 256), 
    uint16_t(60.382734f * 256), uint16_t(61.049557f * 256), uint16_t(61.720656f * 256), uint16_t(62.396039f * 256),
    uint16_t(63.075714f * 256), uint16_t(63.759687f * 256), uint16_t(64.447968f * 256), uint16_t(65.140564f * 256), 
    uint16_t(65.837482f * 256), uint16_t(66.538730f * 256), uint16_t(67.244316f * 256), uint16_t(67.954247f * 256),
    uint16_t(68.668531f * 256), uint16_t(69.387176f * 256), uint16_t(70.110189f * 256), uint16_t(70.837578f * 256), 
    uint16_t(71.569350f * 256), uint16_t(72.305513f * 256), uint16_t(73.046074f * 256), uint16_t(73.791041f * 256),
    uint16_t(74.540421f * 256), uint16_t(75.294222f * 256), uint16_t(76.052450f * 256), uint16_t(76.815115f * 256), 
    uint16_t(77.582222f * 256), uint16_t(78.353779f * 256), uint16_t(79.129794f * 256), uint16_t(79.910274f * 256),
    uint16_t(80.695226f * 256), uint16_t(81.484657f * 256), uint16_t(82.278575f * 256), uint16_t(83.076988f * 256), 
    uint16_t(83.879901f * 256), uint16_t(84.687323f * 256), uint16_t(85.499261f * 256), uint16_t(86.315721f * 256),
    uint16_t(87.136712f * 256), uint16_t(87.962240f * 256), uint16_t(88.792312f * 256), uint16_t(89.626935f * 256), 
    uint16_t(90.466117f * 256), uint16_t(91.309865f * 256), uint16_t(92.158186f * 256), uint16_t(93.011086f * 256),
    uint16_t(93.868573f * 256), uint16_t(94.730654f * 256), uint16_t(95.597335f * 256), uint16_t(96.468625f * 256), 
    uint16_t(97.344529f * 256), uint16_t(98.225055f * 256), uint16_t(99.110210f * 256), uint16_t(100.00000f * 256)
};

namespace ymd{
template<arithmetic T>
struct XYZ{
    T x,y,z;
};



//快速计算立方根
__fast_inline static constexpr float fast_cbrtf(float x) {
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

__fast_inline static constexpr auto xyz_gamma(const real_t x) -> real_t{
    return (x > 0.008856_r) ? q16::from(fast_cbrtf(float(x))) : ((x * 7.787037_r) + 0.137931_r);
}

__fast_inline static constexpr auto inv_xyz_gamma_to8(const real_t x) -> uint8_t{
    if((x > 0.0031308_r)) 
        return __USAT8(uint8_t(((1.055_r * 255) * pow(x, 0.416666_r)) - (0.055_r * 255)));
    else
        return __USAT8(uint8_t(x * 12.92_r * 255));
}


__fast_inline static constexpr std::tuple<real_t, real_t, real_t> lab888_to_xyz(const LAB888 & lab){
    const auto l = lab.l;
    const auto a = lab.a;
    const auto b = lab.b;
    const real_t x = ((l + 16) * 0.008621_r) + (a * 0.002_r);
    const real_t y = ((l + 16) * 0.008621_r);
    const real_t z = ((l + 16) * 0.008621_r) - (b * 0.005_r);


    return {
        ((x > 0.206897_r) ? (x * x * x) : ((0.128419_r * x) - 0.017713_r)) * 095.047_r,
        ((y > 0.206897_r) ? (y * y * y) : ((0.128419_r * y) - 0.017713_r)) * 100.000_r,
        ((z > 0.206897_r) ? (z * z * z) : ((0.128419_r * z) - 0.017713_r)) * 108.883_r
    };
}

__fast_inline static constexpr XYZ<real_t> rgb888_to_xyz(const RGB888 & rgb){
    const auto [r,g,b] = rgb;
    
    const real_t r_lin = q16(xyz_table[r]) >> 8;
    const real_t g_lin = q16(xyz_table[g]) >> 8;
    const real_t b_lin = q16(xyz_table[b]) >> 8;

    const real_t x = ((r_lin * 0.4124_r) + (g_lin * 0.3576_r) + (b_lin * 0.1805_r)) * real_t(1.0f / 095.047f);
    const real_t y = ((r_lin * 0.2126_r) + (g_lin * 0.7152_r) + (b_lin * 0.0722_r)) * real_t(1.0f / 100.000f);
    const real_t z = ((r_lin * 0.0193_r) + (g_lin * 0.1192_r) + (b_lin * 0.9505_r)) * real_t(1.0f / 108.883f);


    return {xyz_gamma(x), xyz_gamma(y), xyz_gamma(z)};
}



__fast_inline static constexpr RGB888 xyz_to_rgb888(const auto & xyz){
    auto [x,y,z] = xyz;
    real_t r_lin = ((x * (+3.2406_r)) + (y * (-1.5372_r)) + (z * (-0.4986_r))) * (0.01_r);
    real_t g_lin = ((x * (-0.9689_r)) + (y * (+1.8758_r)) + (z * (+0.0415_r))) * (0.01_r);
    real_t b_lin = ((x * (+0.0557_r)) + (y * (-0.2040_r)) + (z * (+1.0570_r))) * (0.01_r);



    return RGB888::from_r8g8b8(
        inv_xyz_gamma_to8(r_lin),
        inv_xyz_gamma_to8(g_lin),
        inv_xyz_gamma_to8(b_lin)
    );
}


__fast_inline constexpr static LAB888 xyz_to_lab888(const XYZ<real_t> & xyz){
    auto [xf, yf, zf] = xyz;

    return LAB888::from_l8a8b8(
            CLAMP( uint8_t(int(116 * yf) - 16), 0, 100),
            __SSAT8(int8_t(int(500 * (xf - yf)))),
            __SSAT8(int8_t(int(200 * (yf - zf))))
    );
}

OutputStream & operator<<(OutputStream & os, const Binary & bn){
    return os << '(' << bn.is_white() << ')';
}

OutputStream & operator<<(OutputStream & os, const Gray & gs){
    return os << '(' << uint8_t(gs) << ')';
}

OutputStream & operator<<(OutputStream & os, const IGray & sgs){
    return os << '(' << int8_t(sgs) << ')';
}

#define OS_RGB    return os << '(' << uint8_t(rgb.r) << os.splitter() << uint8_t(rgb.g) << os.splitter() << uint8_t(rgb.b) << ')';
#define OS_XXX(u,v,w)    return os << '(' << uint8_t(u) << os.splitter() << uint8_t(v) << os.splitter() << uint8_t(w) << ')';

OutputStream & operator<<(OutputStream & os, const RGB565 & rgb){OS_RGB}

OutputStream & operator<<(OutputStream & os, const RGB888 & rgb){OS_RGB}

OutputStream & operator<<(OutputStream & os, const LAB888 & lab){{OS_XXX(lab.l, lab.a, lab.b)};}

OutputStream & operator<<(OutputStream & os, const HSV888 & hsv){{OS_XXX(hsv.h, hsv.s, hsv.v)};}

#undef OS_RGB
#undef OS_XXX


LAB888::LAB888(const RGB888 & rgb){
    *this = xyz_to_lab888(rgb888_to_xyz(rgb));
}

LAB888::operator RGB888() const {
    const auto [x,y,z] = lab888_to_xyz(*this);
    return xyz_to_rgb888(std::make_tuple(x,y,z));
}

}