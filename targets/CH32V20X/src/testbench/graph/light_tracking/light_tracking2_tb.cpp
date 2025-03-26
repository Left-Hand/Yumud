/*
* STC8H8K64U-45I-PDIP40 @40MHz
* LCD16080 ST7735
* sdcc 3drt.c --model-large
* https://liuliqiang.com/51/3drt.c
* by liuliqiang 2025-03-23
*/

#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"

#include "hal/bus/uart/uarthw.hpp"
#include "hal/bus/spi/spihw.hpp"

#include "drivers/Display/Polychrome/ST7789/st7789.hpp"

#include "utils.hpp"
#include "dsp/siggen/noise/LCGNoiseSiggen.hpp"
#include "data.hpp"

#include "hal/rng/rng.hpp"


[[maybe_unused]] __no_inline
static void filter(const std::span<RGB565> row) {
    static constexpr size_t WINDOW_SIZE = 3; // 3x3 中值滤波窗口
    static constexpr size_t HALF_WINDOW = WINDOW_SIZE / 2;


    if (row.size() < WINDOW_SIZE) {
        // 如果行长度小于3，无法进行中值滤波
        return;
    }


    std::array<RGB565, WINDOW_SIZE> window;

    for (size_t i = 0; i < row.size(); ++i) {
        // 收集窗口内的像素
        for (size_t j = 0; j < WINDOW_SIZE; ++j) {
            size_t index = i + j - HALF_WINDOW;
            if (index < 0) {
                index = 0;
            } else if (index >= row.size()) {
                index = row.size() - 1;
            }
            window[j] = row[index];
        }

        // 对窗口内的像素进行排序
        std::sort(window.begin(), window.end());

        // 取中值
        row[i] = window[HALF_WINDOW];
    }
}


static std::tuple<real_t, real_t> rand01_2(){
    static dsp::LcgNoiseSiggen noise;
    // noise.update();
    // hal::rng.update();
    // return noise.get_as_01_2();
    const uint32_t temp = rng.update();
    const uint32_t u0 = temp >> 16;
    const uint32_t u1 = temp & 0xffff;
    return {real_t(std::bit_cast<_iq<16>>(u0)), real_t(std::bit_cast<_iq<16>>(u1))};
    // return {0,0};
}

[[nodiscard]]
scexpr RGB get_relect_color(const int8_t i){
    switch(i){
        case 8:
        case 9:
            return RGB(0.05_r, 0.65_r, 0.05_r);
        case 10:
        case 11:
            return RGB(0.65_r, 0.05_r, 0.05_r);
        default:
            return RGB{0.65_r, 0.65_r, 0.65_r};
    }
}


[[nodiscard]]
static Interaction_t<real_t> make_interaction(
    const Intersection_t<real_t> & intersection, 
    const Ray3_t<real_t> & ray, 
    std::span<const TriangleSurfaceCache_t<real_t>> co_triangles)
{
    const auto & surface = co_triangles[intersection.i];
    return Interaction_t<real_t>{
        intersection.i,
        intersection.t,
        surface,
        ray.start + ray.direction * intersection.t,
        (ray.direction.dot(surface.normal) > 0) ? (-surface.normal) : (surface.normal) 
    };
}

template<size_t Q>
__fast_inline scexpr bool not_in_one(const iq_t<Q> x){
    return std::bit_cast<int32_t>(x.value) & (~((1 << Q) - 1));
} 


[[nodiscard]]
static __fast_inline real_t tt_intersect(const Ray3_t<real_t> & ray, const TriangleSurfaceCache_t<real_t> & surface){
    const auto & E1 = surface.v1 - surface.v0;  // E1 = v1 - v0
    const auto & E2 = surface.v2 - surface.v0;  // E2 = v2 - v0

    const auto P = ray.direction.cross(E2);
    const auto determinant = P.dot(E1);

    if (std::abs(determinant) < EPSILON) return 0;

    const auto inv_determinant = 1 / determinant;

    const auto vec = ray.start - surface.v0;
    const auto u = P.dot(vec) * inv_determinant;
    if (unlikely(not_in_one(u))) return 0;

    const auto Q = vec.cross(E1);
    const auto v = Q.dot(ray.direction) * inv_determinant;
    if (unlikely(not_in_one(v) or u + v > 1)) return 0;

    return Q.dot(E2) * inv_determinant;
};

[[nodiscard]]
static __fast_inline bool bb_intersect_impl(const Vector3_t<real_t> & t0, const Vector3_t<real_t> & t1){
    return (vec3_compMin(t0.max_with(t1)) >= MAX(vec3_compMax(t0.min_with(t1)), 0));
};

[[nodiscard]]
static __fast_inline bool tb_intersect_impl (const Vector3_t<real_t> & t0, const Vector3_t<real_t> & t1){
    return (vec3_compMin(t0.max_with(t1)) >= MAX(vec3_compMax(t0.min_with(t1)), 0));
};

[[nodiscard]]
static __fast_inline bool bb_intersect(const Vector3_t<real_t> & start, const Vector3_t<real_t> & inv_dir){
    const Vector3_t<real_t> t0 = (bbmin - start) * inv_dir;
    const Vector3_t<real_t> t1 = (bbmax - start) * inv_dir;

    return bb_intersect_impl(t0, t1);
};



[[nodiscard]] __pure
static __fast_inline bool tb_intersect (const Vector3_t<real_t> & start, const Vector3_t<real_t> & inv_dir){
    const Vector3_t<real_t> t0 = (bbmin - start) * inv_dir;
    const Vector3_t<real_t> t1 = (bbmax - start) * inv_dir;

    return tb_intersect_impl(t0, t1);
};


[[nodiscard]] __pure
static Intersection_t<real_t> intersect(const Ray3_t<real_t> & ray, std::span<const TriangleSurfaceCache_t<real_t>> co_triangles){
    Intersection_t<real_t> intersection = {
        -1,
        std::numeric_limits<real_t>::max()
    };

    const auto & start = ray.start;
    const auto inv_dir = Vector3_t<real_t>::from_rcp(ray.direction);

    const Vector3_t<real_t> t0 = (bbmin - start) * inv_dir;
    const Vector3_t<real_t> t1 = (bbmax - start) * inv_dir;

    
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
static std::optional<std::tuple<RGB, real_t>> sample_bsdf(const Interaction_t<real_t> & interaction, const Ray3_t<real_t> & ray, const Quat_t<real_t> & rotation){
    const auto wi_z = rotation.xform_up().dot(ray.direction);

    if (wi_z <= 0) return std::nullopt;

    static constexpr auto inv_pi = real_t(INV_PI);
    const auto bsdf_pdf = wi_z * inv_pi;
    if(bsdf_pdf == 0) return std::nullopt;

    return std::make_tuple(
        get_relect_color(interaction.i) * (INV_PI * std::abs(wi_z)),
        bsdf_pdf
    );
}

[[nodiscard]] __pure
static Ray3_t<real_t> cos_weighted_hemi(const __restrict Interaction_t<real_t> & interaction, const __restrict Quat_t<real_t> & rotation)
{
    const auto [u0, u1] = rand01_2();

    const auto r = sqrtf(u0);
    const auto azimuth = u1 * real_t(TAU);
    const auto [sin_a, cos_a] = sincos(azimuth);

    const auto v = Vector3_t<real_t>(
        real_t((r * cos_a)), 
        real_t((r * sin_a)), 
        real_t((sqrtf(1 - u0)))
    );

    return Ray3_t<real_t>::from_start_and_dir(
        interaction.position + interaction.normal * EPSILON,
        rotation.xform(v)
    );
}

[[nodiscard]]
static std::optional<RGB> sample_light(const __restrict Interaction_t<real_t> & interaction, const __restrict Quat_t<real_t> & rotation, std::span<const TriangleSurfaceCache_t<real_t>> co_triangles){
    const auto [u0, u1] = rand01_2();

    const auto light_idx = u0 < 0.5_r ? 0 : 1;

    const auto su = sqrtf(u0);

    const auto & light = co_triangles[light_idx];

    const auto linear_t = Vector3_t(
        (1 - su),
        (1 - u1) * su,
        u1 * su
    );

    const auto light_pos = Vector3_t{
        Vector3_t(light.v0.x, light.v1.x, light.v2.x).dot(linear_t),
        Vector3_t(light.v0.y, light.v1.y, light.v2.y).dot(linear_t),
        Vector3_t(light.v0.z, light.v1.z, light.v2.z).dot(linear_t)
    };

    const auto ray = Ray3_t<real_t>::from_start_and_stop(
        interaction.position + interaction.normal * EPSILON,
        light_pos
    );

    const real_t cos_light_theta = -ray.direction.dot(light.normal);
    if (cos_light_theta <= 0) return std::nullopt;

    const real_t cos_theta = ray.direction.dot(interaction.normal);
    if (cos_theta <= 0) return std::nullopt;

    const auto intersection = intersect(ray, co_triangles);
    if (intersection.i < 0 || intersection.i != light_idx) return std::nullopt;

    const auto sample_opt = sample_bsdf(interaction, ray, rotation);
    if (!sample_opt) return std::nullopt;
    const auto & [sample, bsdf_pdf] = sample_opt.value();

    const real_t light_pdf = (intersection.t * intersection.t) / (real_t(light_area) * cos_light_theta);
    const real_t mis_weight = light_pdf / (light_pdf + bsdf_pdf);
    return (sample * lightColor * mis_weight * 2) / light_pdf;
}

// 生成从默认Z轴(0,0,1)旋转到法线方向的四元数
static Quat_t<real_t> quat_from_normal(const Vector3_t<real_t>& normal)
{
    const auto ilen = isqrt(1 + (normal.z + 2) * normal.z);
    return Quat_t<real_t>(
        -normal.y,
        normal.x,
        0,
        real_t(1) + normal.z
    ) * ilen; // 最后一步归一化（可合并到后续操作中）
}

static RGB sampleRay(RGB sample, Ray3_t<real_t> ray, std::span<const TriangleSurfaceCache_t<real_t>> co_triangles){
    auto throughput = RGB{1,1,1};
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
        if(light_opt) sample += light_opt.value() * throughput;

        depth++;
        ray = cos_weighted_hemi(interaction, rotation);

        const auto sample_opt = sample_bsdf(interaction, ray, rotation);
        if (!sample_opt) return sample;

        const auto & [bsdf, bsdf_pdf] = sample_opt.value();
        throughput *= (bsdf / bsdf_pdf);

        if (depth == max_depth) return sample;
    }
}



static RGB samplePixel(const uint x, const uint y, std::span<const TriangleSurfaceCache_t<real_t>> co_triangles){
    auto sample = RGB();

    static constexpr auto uz = - real_t(0.5) / tanf(real_t((alpha * TAU / 360) * 0.5f));

    for (size_t i = 0; i < spp; i++){

        const real_t ux = x * INV_LCD_H;
        const real_t uy = y * INV_LCD_H;

        sample += 
        // sampleRay(
        //     sample,
        //     Ray3_t<real_t>::from_start_and_dir(eye,Vector3_t<real_t>(ux - 0.5_r, 0.5_r - uy, uz)),
        //     co_triangles
        // )
        RGB(ux, uy, CLAMP(ux + uy, 0, 1))
        * inv_spp;

    }

    return sample;
    return RGB{
        real_t(sample.r / (1 + sample.r)),
        real_t(sample.g / (1 + sample.g)),
        real_t(sample.b / (1 + sample.b))
    };
}

__no_inline
static RGB565 draw3drt(const uint x, const uint y, std::span<const TriangleSurfaceCache_t<real_t>> co_triangles, const auto param){
    // const auto sample = samplePixel(x, y, co_triangles);

    const real_t ux = x * INV_LCD_H;
    const real_t uy = y * INV_LCD_H;
    const auto sample = RGB(ux, uy, CLAMP(ux + uy + param, 0, 1)) * inv_spp;

    return RGB565(uint8_t(sample.r * 31), uint8_t(sample.g * 63), uint8_t(sample.b * 31));
}


__no_inline
static void render_row(const __restrict std::span<RGB565> row, const uint y, std::span<const TriangleSurfaceCache_t<real_t>> co_triangles){
    // ASSERT(row.size() == LCD_W);
    const auto s = sin(8 * time());
    for (size_t x = 0; x < LCD_W; x++){
        row[x] = draw3drt(x, y, co_triangles, s);
    }

    // filter(row);
}

// [[maybe_unused]]
// static std::vector<RGB565> render_row_v2(const uint w, const uint y){
//     std::vector<RGB565> row(w);

//     for (uint x = 0; x < LCD_W; x++){
//         row[x] = draw3drt(x, y);
//     }

//     // filter(row);

//     return row;
// }


#define UART hal::uart2
using drivers::ST7789;

void light_tracking_main(void){

    UART.init(576000);
    DEBUGGER.retarget(&UART);
    DEBUGGER.setEps(4);
    // DEBUGGER.noBrackets();


    #ifdef CH32V30X
    auto & spi = spi2;
    auto & lcd_blk = portC[7];
    
    lcd_blk.outpp(HIGH);

    auto & lcd_cs = portD[6];
    auto & lcd_dc = portD[7];
    auto & dev_rst = portB[7];
    #else
    auto & spi = spi1;
    auto & lcd_blk = portA[10];
    auto & lcd_cs = portA[15];
    auto & lcd_dc = portA[11];
    auto & dev_rst = portA[12];
    
    
    lcd_blk.outpp(HIGH);
    #endif

    


    spi.bind_cs_pin(lcd_cs, 0);
    spi.init(144_MHz, CommStrategy::Blocking);
    // spi.init(36_MHz, CommStrategy::Blocking, CommStrategy::None);

    // ST7789 tftDisplayer({{spi, 0}, lcd_dc, dev_rst}, {240, 134});
    ST7789 tftDisplayer({{spi, 0}, lcd_dc, dev_rst}, {240, 135});
    DEBUG_PRINTLN("--------------");

    {
        tftDisplayer.init();


        if(true ){
        // if(false){
            tftDisplayer.set_flip_x(false);
            tftDisplayer.set_flip_y(true);
            tftDisplayer.set_swap_xy(true);
            tftDisplayer.set_display_offset({40, 52}); 
        }else{
            tftDisplayer.set_flip_x(true);
            tftDisplayer.set_flip_y(true);
            tftDisplayer.set_swap_xy(false);
            tftDisplayer.set_display_offset({52, 40}); 
        }
        tftDisplayer.set_format_rgb(true);
        tftDisplayer.set_flush_dir_h(false);
        tftDisplayer.set_flush_dir_v(false);
        tftDisplayer.set_inversion(true);
    }

    tftDisplayer.fill(ColorEnum::PINK);
    delay(200);

    for (uint y = 0; y < LCD_H; y++){
        std::array<RGB565, LCD_W> row;
        row.fill(RGB565(Color_t<real_t>(0,1,0,0)));
        // DEBUG_PRINTLN(std::span(reinterpret_cast<const uint16_t * >(row.data()), row.size()));
        tftDisplayer.put_texture(Rect2i(Vector2i(0,y), Vector2i(LCD_W, 1)), row.data());
        // tftDisplayer.put_rect(Rect2i(Vector2i(0,y), Vector2i(LCD_W, 1)), ColorEnum::WHITE);
        // renderer.draw_rect(Rect2i(20, 0, 20, 40));
    }


    std::vector<TriangleSurfaceCache_t<real_t>> co_triangles(triangles.begin(), triangles.end());
    // for(uint i = 0; i < co_triangles.size(); i++) 
    //     co_triangles[i] = TriangleSurfaceCache_t<real_t>(triangles[i]);
    DEBUG_PRINTLN(millis());
    auto render = [&](){
        for (uint y = 0; y < LCD_H; y++){

            std::array<RGB565, LCD_W> row;
            render_row(row, y, co_triangles);

            // const auto row = render_row_v2(LCD_W, y);
            // DEBUG_PRINTLN(std::span(reinterpret_cast<const uint16_t * >(row.data()), row.size()));
            tftDisplayer.put_texture(Rect2i(Vector2i(0,y), Vector2i(LCD_W, 1)), row.data());
            // DEBUG_PRINTLN(uint8_t(row[50].b));

            // tftDisplayer.put_rect(Rect2i(Vector2i(0,y), Vector2i(LCD_W, 1)), ColorEnum::WHITE);
            // renderer.draw_rect(Rect2i(20, 0, 20, 40));
        }
    };

    // for(size_t i = 0; i < 100; i++) render();
    while(true) render();

    DEBUG_PRINTLN(millis());

}
