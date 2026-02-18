/*
* STC8H8K64U-45I-PDIP40 @40MHz
* LCD16080 ST7735
* sdcc 3drt.c --model-large
* https://liuliqiang.com/51/3drt.c
* by liuliqiang 2025-03-23
*/

#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"

#include "hal/bus/uart/hw_singleton.hpp"
#include "hal/bus/spi/hw_singleton.hpp"

#include "drivers/Display/Polychrome/ST7789/st7789.hpp"

#include "utils.hpp"
#include "dsp/siggen/noise/LCGNoiseSiggen.hpp"
#include "data.hpp"

#include "hal/sysmisc/rng/rng.hpp"

using namespace ymd;


#if 0

[[maybe_unused]] __no_inline
static void filter(const std::span<RGB565> row) {
    static constexpr size_t WINDOW_SIZE = 3; // 3x3 ��ֵ�˲�����
    static constexpr size_t HALF_WINDOW = WINDOW_SIZE / 2;


    if (row.size() < WINDOW_SIZE) {
        // ����г���С��3���޷�������ֵ�˲�
        return;
    }


    std::array<RGB565, WINDOW_SIZE> window;

    for (size_t i = 0; i < row.size(); ++i) {
        // �ռ������ڵ�����
        for (size_t j = 0; j < WINDOW_SIZE; ++j) {
            size_t index = i + j - HALF_WINDOW;
            if (index < 0) {
                index = 0;
            } else if (index >= row.size()) {
                index = row.size() - 1;
            }
            window[j] = row[index];
        }

        // �Դ����ڵ����ؽ�������
        std::sort(window.begin(), window.end());

        // ȡ��ֵ
        row[i] = window[HALF_WINDOW];
    }
}
#endif

static std::tuple<iq16, iq16> rand01_2(){
    static dsp::LcgNoiseSiggen noise;
    noise.update();
    // hal::rng.update();
    return noise.get_as_01x2();
    // const uint32_t temp = rng.update();
    // const uint32_t u0 = temp >> 16;
    // const uint32_t u1 = temp & 0xffff;
    // return {iq16(std::bit_cast<_iq<16>>(u0)), iq16(std::bit_cast<_iq<16>>(u1))};
    // return {0,0};
}

[[nodiscard]]
static constexpr RGB<iq16> get_relect_color(const int8_t i){
    switch(i){
        case 8:
        case 9:
            return RGB<iq16>(0.05_r, 0.65_r, 0.05_r);
        case 10:
        case 11:
            return RGB<iq16>(0.65_r, 0.05_r, 0.05_r);
        default:
            return RGB<iq16>{0.65_r, 0.65_r, 0.65_r};
    }
}


[[nodiscard]]
static Interaction<iq16> make_interaction(
    const Intersection<iq16> & intersection, 
    const Ray3<iq16> & ray, 
    std::span<const TriangleSurfaceCache<iq16>> co_triangles
){
    const auto & surface = co_triangles[intersection.i];
    return Interaction<iq16>{
        intersection.i,
        intersection.t,
        surface,
        ray.base + ray.direction * intersection.t,
        (ray.direction.dot(surface.normal) > 0) ? (-surface.normal) : (surface.normal) 
    };
}

template<size_t Q, typename D>
static constexpr bool not_in_one(const fixed<Q, D> & d){
    return (d < -1 or d > 1);
};

[[nodiscard]]
static __fast_inline iq16 tt_intersect(
    const Ray3<iq16> & ray, 
    const TriangleSurfaceCache<iq16> & surface
){
    const auto & E1 = surface.v1 - surface.v0;  // E1 = v1 - v0
    const auto & E2 = surface.v2 - surface.v0;  // E2 = v2 - v0

    const auto P = ray.direction.cross(E2);
    const auto determinant = P.dot(E1);

    if (std::abs(determinant) < EPSILON) return 0;

    const auto inv_determinant = 1 / determinant;

    const auto vec = ray.base - surface.v0;
    const auto u = P.dot(vec) * inv_determinant;
    if ((not_in_one(u))) [[unlikely]] return 0;

    const auto Q = vec.cross(E1);
    const auto v = Q.dot(ray.direction) * inv_determinant;
    if ((not_in_one(v) or u + v > 1)) [[unlikely]] return 0;

    return Q.dot(E2) * inv_determinant;
};

[[nodiscard]]
static __fast_inline bool bb_intersect_impl(const math::Vec3<iq16> & t0, const math::Vec3<iq16> & t1){
    return (vec3_compMin(t0.max_with(t1)) >= MAX(vec3_compMax(t0.min_with(t1)), 0));
};

[[nodiscard]]
static __fast_inline bool tb_intersect_impl (const math::Vec3<iq16> & t0, const math::Vec3<iq16> & t1){
    return (vec3_compMin(t0.max_with(t1)) >= MAX(vec3_compMax(t0.min_with(t1)), 0));
};


[[nodiscard]] __pure
static Intersection<iq16> intersect(
    const Ray3<iq16> & ray, 
    std::span<const TriangleSurfaceCache<iq16>> co_triangles
){
    Intersection<iq16> intersection = {
        -1,
        std::numeric_limits<iq16>::max()
    };

    const auto & base = ray.base;
    const auto inv_dir = math::Vec3<iq16>::from_rcp(ray.direction);

    const math::Vec3<iq16> t0 = (bbmin - base) * inv_dir;
    const math::Vec3<iq16> t1 = (bbmax - base) * inv_dir;

    
    if ((vec3_compMin(t0.max_with(t1)) >= MAX(vec3_compMax(t0.min_with(t1)), 0))){
        for (size_t k = 0; k < co_triangles.size(); k++){
            const auto isct = tt_intersect(ray, co_triangles[k]);
            if (isct > 0 and isct < intersection.t){
                intersection.t = isct;
                intersection.i = k;
            }
        }
    }

    return intersection;
}


[[nodiscard]] __pure
__fast_inline
static Option<std::pair<RGB<iq16>, iq16>> sample_bsdf(
    const Interaction<iq16> & interaction, 
    const Ray3<iq16> & ray, 
    const Quat<iq16> & rotation
){
    const auto wi_z = rotation.xform_up().dot(ray.direction);

    if (wi_z <= 0) return None;

    static constexpr auto inv_pi = iq16(INV_PI);
    const auto bsdf_pdf = wi_z * inv_pi;
    if(bsdf_pdf == 0) return None;

    return Some(std::make_pair(
        get_relect_color(interaction.i) * (INV_PI * std::abs(wi_z)),
        bsdf_pdf
    ));
}

[[nodiscard]] __pure
static Ray3<iq16> cos_weighted_hemi(
    const __restrict Interaction<iq16> & interaction, 
    const __restrict Quat<iq16> & rotation
){
    const auto [u0, u1] = rand01_2();

    const auto r = std::sqrt(u0);
    const auto azimuth = u1 * iq16(TAU);
    const auto [sin_a, cos_a] = sincos(azimuth);

    const auto v = math::Vec3<iq16>(
        iq16((r * cos_a)), 
        iq16((r * sin_a)), 
        iq16((std::sqrt(1 - u0)))
    );

    return Ray3<iq16>::from_base_and_dir(
        interaction.position + interaction.normal * EPSILON,
        rotation.xform(v)
    );
}

[[nodiscard]]
static Option<RGB<iq16>> sample_light(
    const __restrict Interaction<iq16> & interaction, 
    const __restrict Quat<iq16> & rotation, 
    std::span<const TriangleSurfaceCache<iq16>> co_triangles
){
    const auto [u0, u1] = rand01_2();

    const auto light_idx = u0 < 0.5_r ? 0 : 1;

    const auto su = std::sqrt(u0);

    const auto & light = co_triangles[light_idx];

    const auto linear_t = math::Vec3(
        (1 - su),
        (1 - u1) * su,
        u1 * su
    );

    const auto light_pos = math::Vec3{
        math::Vec3(light.v0.x, light.v1.x, light.v2.x).dot(linear_t),
        math::Vec3(light.v0.y, light.v1.y, light.v2.y).dot(linear_t),
        math::Vec3(light.v0.z, light.v1.z, light.v2.z).dot(linear_t)
    };

    const auto ray = Ray3<iq16>::from_start_and_stop(
        interaction.position + interaction.normal * EPSILON,
        light_pos
    );

    const iq16 cos_light_theta = -ray.direction.dot(light.normal);
    if (cos_light_theta <= 0) return None;

    const iq16 cos_theta = ray.direction.dot(interaction.normal);
    if (cos_theta <= 0) return None;

    const auto intersection = intersect(ray, co_triangles);
    if (intersection.i < 0 || intersection.i != light_idx) return None;

    const auto sample_opt = sample_bsdf(interaction, ray, rotation);
    if (sample_opt.is_none()) return None;
    const auto & [sample, bsdf_pdf] = sample_opt.unwrap();

    const iq16 light_pdf = (intersection.t * intersection.t) / (iq16(light_area) * cos_light_theta);
    const iq16 mis_weight = light_pdf / (light_pdf + bsdf_pdf);
    return Some((sample * lightColor * mis_weight * 2) / light_pdf);
}


static Quat<iq16> quat_from_normal(const math::Vec3<iq16>& normal)
{
    const auto ilen = inv_sqrt(1 + (normal.z + 2) * normal.z);
    return Quat<iq16>::from_xyzw(
        -normal.y * ilen,
        normal.x * ilen,
        0,
        (1 + normal.z) * ilen
    );
}

[[maybe_unused]]
static RGB<iq16> sampleRay(
    RGB<iq16> sample, 
    Ray3<iq16> ray, 
    std::span<const TriangleSurfaceCache<iq16>> co_triangles
){
    auto throughput = RGB<iq16>{1,1,1};
    uint16_t depth = 0;
    while (1){
        const auto intersection = intersect(ray, co_triangles);
        if (intersection.i < 0){
            return sample + throughput;
        }

        if (intersection.i < 2){
            if (depth == 0){
                sample += lightColor;
            }
            return sample;
        }

        const auto interaction = make_interaction(intersection, ray, co_triangles);
        const auto rotation = quat_from_normal(interaction.normal);

        const auto light_opt = sample_light(interaction, rotation, co_triangles);
        if(light_opt.is_some()) sample += light_opt.unwrap() * throughput;

        depth++;
        ray = cos_weighted_hemi(interaction, rotation);

        const auto sample_opt = sample_bsdf(interaction, ray, rotation);
        if (sample_opt.is_none()) return sample;

        const auto & [bsdf, bsdf_pdf] = sample_opt.unwrap();
        throughput *= (bsdf / bsdf_pdf);

        if (depth == max_depth) return sample;
    }
}


[[maybe_unused]]
static RGB<iq16> samplePixel(
    const uint x, 
    const uint y,
    std::span<const TriangleSurfaceCache<iq16>> co_triangles
){
    auto sample = RGB<iq16>::from_uninitialized();

    [[maybe_unused]] static constexpr auto uz = - iq16(0.5) / 
        std::tan(iq16((alpha * TAU / 360) * 0.5f));

    for (size_t i = 0; i < spp; i++){

        const iq16 ux = x * INV_LCD_H;
        const iq16 uy = y * INV_LCD_H;

        sample += 
        sampleRay(
            sample,
            Ray3<iq16>::from_base_and_dir(eye,math::Vec3<iq16>(ux - 0.5_r, 0.5_r - uy, uz)),
            co_triangles
        )
        // RGB(ux, uy, CLAMP(ux + uy, 0, 1))
        * inv_spp;

    }

    return sample;
    return RGB<iq16>{
        iq16(sample.r / (1 + sample.r)),
        iq16(sample.g / (1 + sample.g)),
        iq16(sample.b / (1 + sample.b))
    };
}

__no_inline
static RGB565 draw3drt(
    const uint x, 
    const uint y,
    std::span<const TriangleSurfaceCache<iq16>> co_triangles
){
    const auto sample = samplePixel(x, y, co_triangles);

    // const iq16 ux = x * INV_LCD_H;
    // const iq16 uy = y * INV_LCD_H;
    // const auto sample = RGB(ux, uy, CLAMP(ux + uy + param, 0, 1)) * inv_spp;

    return RGB565::from_r5g6b5(uint8_t(sample.r * 31), uint8_t(sample.g * 63), uint8_t(sample.b * 31));
    // return RGB565::(sample);
}


[[maybe_unused]]
__no_inline
static void render_row(
    const __restrict std::span<RGB565> row, 
    const uint y, 
    std::span<const TriangleSurfaceCache<iq16>> co_triangles
){
    // ASSERT(row.size() == LCD_W);
    // const auto s = sin(8 * clock::seconds());
    for (size_t x = 0; x < LCD_W; x++){
        row[x] = draw3drt(x, y, co_triangles);
    }

    // filter(row);
}


// #if 0

#define UART hal::usart2
using drivers::ST7789;
using drivers::ST7789_Transport;

static constexpr size_t LCD_SPI_FREQ_HZ = 72_MHz;
// static constexpr size_t LCD_SPI_FREQ_HZ = 72_MHz / 4;
// static constexpr size_t LCD_SPI_FREQ_HZ = 72_MHz / 16;
void light_tracking_main(void){

    UART.init({
        hal::USART2_REMAP_PA2_PA3,
        hal::NearestFreq(576000),
    });
    DEBUGGER.retarget(&UART);
    DEBUGGER.set_eps(4);
    // DEBUGGER.no_brackets();


    #ifdef CH32V30X
    auto & spi = hal::spi2;
    auto lcd_blk = hal::PC<7>();
    
    lcd_blk.outpp(HIGH);

    auto lcd_cs = hal::PD<6>();
    auto lcd_dc = hal::PD<7>();
    auto dev_rst = hal::PB<7>();
    #else
    auto & spi = hal::spi1;
    auto lcd_blk = hal::PA<10>();
    auto lcd_cs = hal::PA<15>();
    auto lcd_dc = hal::PA<11>();
    auto dev_rst = hal::PA<12>();
    

    lcd_blk.outpp(HIGH);
    #endif

    // lcd_cs.outpp();
    // while(true){
    //     lcd_cs = HIGH;
    //     lcd_cs = LOW;
    //     lcd_cs = HIGH;
    //     lcd_cs = LOW;
    //     lcd_cs = HIGH;
    //     lcd_cs = LOW;
    //     clock::delay(20us);
    //     for(size_t i = 0; i < 100; i++){
    //         lcd_cs.set();
    //         __nopn(4);
    //         lcd_cs.clr();
    //         __nopn(4);
    //     }
    //     clock::delay(20us);
    // }

    // spi.bind_cs_pin(lcd_cs, 0);
    const auto spi_rank = spi.allocate_cs_pin(&lcd_cs).unwrap();
    // spi.init(144_MHz, CommStrategy::Blocking);

    hal::spi1.init({
        .remap = hal::SPI1_REMAP_PB3_PB4_PB5_PA15,
        .baudrate = hal::NearestFreq(LCD_SPI_FREQ_HZ)
    });
    // spi.init(2_MHz, CommStrategy::Blocking, CommStrategy::Nil);
    (void)spi.set_bitorder(MSB);
    // spi.set_bitorder(std::endian::little
    // spi.init(36_MHz, CommStrategy::Blocking, CommStrategy::None);

    // ST7789 displayer({{spi, 0}, lcd_dc, dev_rst}, {240, 134});
    
    ST7789 displayer(
        ST7789_Transport{
            &spi, 
            spi_rank, 
            &lcd_dc, 
            &dev_rst
        }, 
        {240, 135}
    );

    DEBUG_PRINTLN("--------------");
    displayer.init(drivers::st7789_preset::_320X170{}).examine();

    displayer.fill(color_cast<RGB565>(ColorEnum::PINK)).examine();
    clock::delay(200ms);

    [[maybe_unused]]
    auto fill_displayer = [&]{
        // const auto u = micros();
        const auto st = math::sinpu(clock::seconds() * 2) * 0.5_r + 0.5_r;
        // displayer.setpos_unchecked({0,0});
        displayer.setarea_unchecked({math::Vec2u16{0,0}, math::Vec2u16{LCD_W, LCD_H}}).examine();
        for (uint y = 0; y < LCD_H; y++){
            std::array<RGB565, LCD_W> row;
            // row.fill(RGB565(Color<iq16>(0,int(y==0),0,0)));
            // row[0] = 
            for(size_t x = 0; x < row.size(); x++){
                auto & item = row[x];

                // constexpr auto c = Color<iq16>(1,1,0,0);
                // constexpr auto r8 = RGB888(c);
                // // constexpr auto r = RGB565(c); 
                // constexpr auto r = RGB565(r8); 

                // item = RGB565(Color<iq16>(iq16(y) / LCD_H, st, 0));
                // item = RGB565::from_u16(0x003f);
                // item = RGB565::from_u16(0xff00);
                // item = RGB565::from_r5g6b5(y, x, uint8_t(31 * st));
                item = RGB565::from_r5g6b5(y, x, uint8_t(31 * st));
                    // iq16(x) * INV_LCD_W,
                    // x % 2,
                    // 1,
                    
                    // 0, st, 0));
                    // 0, 0, 1));
            }
            // DEBUG_PRINTLN(std::span(reinterpret_cast<const uint16_t * >(row.data()), row.size()));
            
            // const auto u = micros();
            displayer.put_next_texture(math::Rect2u(Vec2i(0,y), Vec2i(LCD_W, 1)), row.data()).examine();
            // DEBUG_PRINTLN(micros() - u, int((uint64_t(row.size() * 16) * 1000000) / LCD_SPI_FREQ_HZ));

            // displayer.put_rect(math::Rect2u(Vec2i(0,y), Vec2i(LCD_W, 1)), ColorEnum::WHITE);
            // renderer.draw_rect(math::Rect2u(20, 0, 20, 40));
        }

        // const uint32_t use_us = micros() - u;
        // const auto fps = 1000000 / use_us;
        // DEBUG_PRINTLN(use_us,fps);
    };

    // while(true){
    //     fill();
    // }

    std::vector<TriangleSurfaceCache<iq16>> co_triangles(triangles.begin(), triangles.end());
    // for(uint i = 0; i < co_triangles.size(); i++) 
    //     co_triangles[i] = TriangleSurfaceCache<iq16>(triangles[i]);
    DEBUG_PRINTLN(clock::millis());


    auto render = [&](){
        const auto u = clock::micros();
        displayer.setarea_unchecked(math::Rect2u{Vec2u{0,0}, Vec2u{LCD_W, LCD_H}}).examine();
        for (uint y = 0; y < LCD_H; y++){

            std::array<RGB565, LCD_W> row;
            render_row(row, y, co_triangles);

            displayer.put_texture(math::Rect2u(Vec2i(0,y), Vec2i(LCD_W, 1)), row.data())
                .examine();
        }

        DEBUG_PRINTLN(clock::micros() - u);
    };

    while(true){
        render();
    }

    DEBUG_PRINTLN(clock::millis());


}

// #endif

