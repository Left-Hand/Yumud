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





scexpr RGB Reflectance(int8_t i)
{
    if (i == 8 || i == 9){
        return RGB(0.05_r, 0.65_r, 0.05_r);
    }
    else if (i == 10 || i == 11){
        return RGB(0.65_r, 0.05_r, 0.05_r);
    }
    else{
        return RGB{0.65_r, 0.65_r, 0.65_r};
    }
}


[[nodiscard]]
static std::tuple<interaction_t<real_t>, mat4_t<real_t>> makeInteraction(const intersection_t<real_t> & intersection, const ray_t<real_t> & ray){
    interaction_t<real_t> interaction{
        intersection.i,
        intersection.t,
        triangles[intersection.i],
        ray.start + ray.direction * intersection.t,
        interaction.surface.normal
    };


    if (ray.direction.dot(interaction.normal) > 0){
        interaction.normal = -interaction.normal;
    }

    return {interaction, orthonormalBasis(interaction.normal)};
}




[[nodiscard]]
static __fast_inline real_t tt_intersect(const ray_t<real_t> & ray, const triangle_t & s){
    const auto P = ray.direction.cross(s.E2);
    const auto determinant = P.dot(s.E1);
    if (determinant < EPSILON && determinant > -EPSILON) return 0;

    const auto inv_determinant = 1 / determinant;

    const auto vec = ray.start - s.v0;
    const auto u = P.dot(vec) * inv_determinant;
    if (u > 1 or u < 0) return 0;

    const auto Q = vec.cross(s.E1);
    const auto v = Q.dot(ray.direction) * inv_determinant;
    if (v > 1 or v < 0 or u + v > 1) return 0;

    return Q.dot(s.E2) * inv_determinant;
};

[[nodiscard]]
static __fast_inline bool bb_intersect(const Vector3_t<real_t> & start, const Vector3_t<real_t> & inv_dir){
    const Vector3_t<real_t> t0 = (bbmin - start) * inv_dir;
    const Vector3_t<real_t> t1 = (bbmax - start) * inv_dir;

    return vec3_compMin(t0.max_with(t1)) >= MAX(vec3_compMax(t0.min_with(t1)), 0);;
};

[[nodiscard]]
static __fast_inline bool tb_intersect (const Vector3_t<real_t> & start, const Vector3_t<real_t> & inv_dir){
    const Vector3_t<real_t> t0 = (bbmin - start) * inv_dir;
    const Vector3_t<real_t> t1 = (bbmax - start) * inv_dir;

    return (vec3_compMin(t0.max_with(t1)) >= MAX(vec3_compMax(t0.min_with(t1)), 0));
};


[[nodiscard]]
static intersection_t<real_t> intersect(const ray_t<real_t> & ray){
    intersection_t<real_t> intersection;

    intersection.t = std::numeric_limits<real_t>::max();
    intersection.i = -1;

    const auto & start = ray.start;
    const auto inv_dir = Vector3_t<real_t>::from_rcp(ray.direction);
    if (bb_intersect(start, inv_dir)){
        for (size_t k = 0; k < triangles.size(); ++k)
        {
            if (!tb_intersect(start, inv_dir))
                continue;
            const auto isct = tt_intersect(ray, triangles[k]);
            if (isct > 0)
            {
                if (isct < intersection.t)
                {
                    intersection.t = isct;
                    intersection.i = k;
                }
            }
        }
    }

    return intersection;
}


[[nodiscard]]
__fast_inline
static std::optional<std::tuple<RGB, real_t>> sampleBSDF(const interaction_t<real_t> & interaction, const ray_t<real_t> & ray, const mat4_t<real_t> & transform){
    const auto wi_z = transform.vz().dot(ray.direction);

    if (wi_z <= 0) return std::nullopt;

    static constexpr auto inv_pi = real_t(INV_PI);
    const auto bsdf_pdf = wi_z * inv_pi;
    if(bsdf_pdf == 0) return std::nullopt;

    return std::make_tuple(
        Reflectance(interaction.i) * (INV_PI * std::abs(wi_z)),
        bsdf_pdf
    );
}

[[nodiscard]]
static ray_t<real_t> cosWeightedHemi(const interaction_t<real_t> & interaction, const mat4_t<real_t> & transform)
{
    const auto u0 = real_t(rand01());
    const auto u1 = real_t(rand01());

    const auto r = sqrtf(u0);
    const auto azimuth = u1 * real_t(TAU);
    const auto [sin_a, cos_a] = sincos(azimuth);

    const auto v = Vector3_t<real_t>(
        real_t((r * cos_a)), 
        real_t((r * sin_a)), 
        real_t((sqrtf(1 - u0)))
    );

    return ray_t<real_t>::from_start_and_dir(
        interaction.position + interaction.normal * EPSILON,
    
        Vector3_t<real_t>{
            transform.vx().dot(v),
            transform.vy().dot(v),
            transform.vz().dot(v)
        }
    );
}

[[nodiscard]]
static std::optional<RGB> sampleLight(const interaction_t<real_t> & interaction, const mat4_t<real_t> & transform){
    const auto u1 = real_t(rand01());
    const auto u0 = real_t(rand01());

    const auto lightIdx = u0 < 0.5_r ? 0 : 1;

    const real_t su = sqrtf(u0);

    const triangle_t & light = triangles[lightIdx];

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

    const auto ray = ray_t<real_t>::from_start_and_stop(
        interaction.position + interaction.normal * EPSILON,
        light_pos
    );

    const real_t cos_light_theta = -ray.direction.dot(light.normal);
    if (cos_light_theta <= 0) return std::nullopt;

    const real_t cos_theta = ray.direction.dot(interaction.normal);
    if (cos_theta <= 0) return std::nullopt;

    const auto intersection = intersect(ray);
    if (intersection.i == -1 || intersection.i != lightIdx) return std::nullopt;

    const auto sample_opt = sampleBSDF(interaction, ray, transform);
    if (!sample_opt) return std::nullopt;
    const auto & [sample, bsdf_pdf] = sample_opt.value();

    const real_t light_pdf = (intersection.t * intersection.t) / (real_t(light_area) * cos_light_theta);
    const real_t mis_weight = light_pdf / (light_pdf + bsdf_pdf);
    return (sample * lightColor * mis_weight * 2) / light_pdf;
}

static RGB sampleRay(RGB sample, ray_t<real_t> ray){
    auto throughput = RGB{1,1,1};
    uint16_t depth = 0;
    while (1){
        const auto intersection = intersect(ray);
        if (intersection.i == -1){
            return sample + throughput;
        }

        if (intersection.i < 2){
            if (depth == 0){
                sample += lightColor;
            }
            return sample;
        }

        const auto [interaction, transform] = makeInteraction(intersection, ray);

        const auto light_opt = sampleLight(interaction, transform);
        const auto radiance = light_opt.value_or(RGB(0, 0, 0));
        sample += radiance * throughput;

        depth++;
        ray = cosWeightedHemi(interaction, transform);

        const auto sample_opt = sampleBSDF(interaction, ray, transform);
        if (!sample_opt) return sample;

        const auto & [bsdf, bsdf_pdf] = sample_opt.value();
        throughput *= (bsdf / bsdf_pdf);

        if (depth == max_depth) return sample;
    }
}



static RGB samplePixel(const uint X, const uint Y)
{
    auto sample = RGB();

    // static constexpr auto Zc = - real_t(0.5) / tanf(real_t((alpha * TAU / 360) * 0.5f));
    static constexpr auto Zc = - real_t(0.5) / tanf(real_t((alpha * TAU / 360) * 0.5f));

    for (size_t i = 0; i < spp; i++){

        const real_t Xc = (X - (LCD_W >> 1)) * INV_LCD_H;
        const real_t Yc = (LCD_H - 1 - Y - (LCD_H >> 1)) * INV_LCD_H;


        // const auto linear_t = 

        // const auto temp_direction = Vector3_t<q8>{
        //     Vector3_t<float>(1, 0, 0).dot(linear_t),
        //     Vector3_t<float>(0, 1, 0).dot(linear_t),
        //     Vector3_t<float>(0, 0, 1).dot(linear_t)
        // }.normalized();

        sample += sampleRay(
            sample,
            ray_t<real_t>::from_start_and_dir(
                eye,
                Vector3_t<q16>(Xc, Yc, Zc)
                // temp_direction
            )
        ) * inv_spp;
    }

    return RGB{
        real_t(sample.r / (1 + sample.r)),
        real_t(sample.g / (1 + sample.g)),
        real_t(sample.b / (1 + sample.b))
    };
    // return sample;
}

static RGB565 draw3drt(const uint X, const uint Y){
    const auto sample = samplePixel(X, Y);
    return RGB565(uint8_t(sample.r * 255), uint8_t(sample.g * 255), uint8_t(sample.b * 255));
}

static void render_row(const std::span<RGB565> row, const uint y){
    ASSERT(row.size() == LCD_W);

    for (uint x = 0; x < LCD_W; x++){
        row[x] = draw3drt(x, y);
    }
}


#define UART hal::uart2
using drivers::ST7789;

void light_tracking_main(void){

    UART.init(576000);
    DEBUGGER.retarget(&UART);
    DEBUGGER.setEps(4);
    // DEBUGGER.noBrackets();

    DEBUG_PRINTLN(micros());
    DEBUG_PRINTLN(view_x, view_y, view_z);


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

    for (uint y = 0; y < LCD_H; y++){
        std::array<RGB565, LCD_W> row;
        render_row(row, y);
        // DEBUG_PRINTLN(std::span(reinterpret_cast<const uint16_t * >(row.data()), row.size()));
        tftDisplayer.put_texture(Rect2i(Vector2i(0,y), Vector2i(LCD_W, 1)), row.data());
        // tftDisplayer.put_rect(Rect2i(Vector2i(0,y), Vector2i(LCD_W, 1)), ColorEnum::WHITE);
        // renderer.draw_rect(Rect2i(20, 0, 20, 40));
    }

    DEBUG_PRINTLN(millis());

}
