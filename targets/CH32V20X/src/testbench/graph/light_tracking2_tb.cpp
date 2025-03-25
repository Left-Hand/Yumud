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

[[nodiscard]]

static std::tuple<interaction_t, mat4_t> makeInteraction(const intersection_t & intersection, const ray_t & ray){
    interaction_t interaction;

    interaction.i = intersection.i;
    interaction.t = intersection.t;
    interaction.surface = &((const struct triangle_t*)triangles)[intersection.i];
    interaction.position =  ray.start + ray.direction * intersection.t;
    interaction.normal = interaction.surface->normal;

    if (ray.direction.dot(interaction.normal) > 0){
        interaction.normal = -interaction.normal;
    }

    return {interaction, orthonormalBasis(interaction.normal)};
}

static __fast_inline bool tb_intersect (const ray_t & ray){
    const auto t0 = (bbmin - ray.start) * ray.inv_direction;
    const auto t1 = (bbmax - ray.start) * ray.inv_direction;

    return (vec3_compMin(t0.max_with(t1)) >= MAX(vec3_compMax(t0.min_with(t1)), 0));
};


static __fast_inline float tt_intersect(const ray_t & ray){
    const auto P = ray.direction.cross(s->E2);
    const float determinant = P.dot(s->E1);
    if (determinant < EPSILON && determinant > -EPSILON) return 0;

    const float inv_determinant = 1 / determinant;

    const auto T = ray.start - s->v0;
    const float u = P.dot(T) * inv_determinant;
    if (u > 1 || u < 0) return 0;

    const auto Q = T.cross(s->E1);
    const float v = Q.dot(ray.direction) * inv_determinant;
    if (v > 1 || v < 0 || u + v > 1) return 0;

    return Q.dot(s->E2) * inv_determinant;
};

static __fast_inline bool bb_intersect(const ray_t & ray){
    const auto t0 = (bbmin - ray.start) * ray.inv_direction;
    const auto t1 = (bbmax - ray.start) * ray.inv_direction;

    return vec3_compMin(t0.max_with(t1)) >= MAX(vec3_compMax(t0.min_with(t1)), 0);;
};


static void intersect(const ray_t & ray){
    intersection.t = FLT_MAX;
    intersection.i = -1;

    if (bb_intersect(ray)){
        for (auto k = 0; k < 32; ++k)
        {
            s = &((const struct triangle_t*)triangles)[k];
            if (!tb_intersect(ray))
                continue;
            const auto isct = tt_intersect(ray);
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
}




static std::optional<Vector3_t<float>> sampleBSDF(const interaction_t & interaction, const ray_t & ray, const mat4_t & T){
    const auto z = Vector3_t(T.m[2][0], T.m[2][1], T.m[2][2]);
    const float wi_z = z.dot(ray.direction);

    if (wi_z <= 0) return std::nullopt;

    bsdf_pdf = wi_z * float(INV_PI);
    if(bsdf_pdf == 0) return std::nullopt;

    return Reflectance(interaction.i) *(INV_PI * std::abs(wi_z));
}

[[nodiscard]]
static ray_t cosWeightedHemi(const interaction_t & interaction, const mat4_t & T)
{
    const auto u0 = real_t(rand01());
    const auto u1 = real_t(rand01());

    const auto r = sqrtf(u0);
    const auto azimuth = u1 * real_t(TAU);
    const auto [sin_a, cos_a] = sincos(azimuth);

    const auto v = Vector3_t<float>(
        float((r * cos_a)     ), 
        float((r * sin_a)     ), 
        float((sqrtf(1 - u0)))
    );

    ray_t ray;
    ray.start =  interaction.position + interaction.normal * EPSILON;

    ray.direction.x = Vector3_t(T.m[0][0], T.m[1][0], T.m[2][0]).dot(v);
    ray.direction.y = Vector3_t(T.m[0][1], T.m[1][1], T.m[2][1]).dot(v);
    ray.direction.z = Vector3_t(T.m[0][2], T.m[1][2], T.m[2][2]).dot(v);

    ray.inv_direction = Vector3_t<float>::from_rcp(ray.direction);
    return ray;
}


static std::optional<Vector3_t<float>> sampleLight(const interaction_t & interaction, const mat4_t & T){
    const auto u1 = float(rand01());
    const auto u0 = float(rand01());

    const auto lightIdx = u0 < 0.5f ? 0 : 1;

    const float su = sqrtf(u0);


    const struct triangle_t* light = &((const struct triangle_t*)triangles)[lightIdx];

    const auto linear_t = Vector3_t(
        (1 - su),
        (1 - u1) * su,
        u1 * su
    );

    const auto light_pos = Vector3_t{
        Vector3_t(light->v0.x, light->v1.x, light->v2.x).dot(linear_t),
        Vector3_t(light->v0.y, light->v1.y, light->v2.y).dot(linear_t),
        Vector3_t(light->v0.z, light->v1.z, light->v2.z).dot(linear_t)
    };

    ray_t ray;
    ray.start = interaction.position + interaction.normal * EPSILON;
    ray.direction = (light_pos - ray.start).normalized();
    ray.inv_direction = Vector3_t<float>::from_rcp(ray.direction);

    const float cos_light_theta = -ray.direction.dot(light->normal);
    if (cos_light_theta <= 0){
        return std::nullopt;
    }

    const float cos_theta = ray.direction.dot(interaction.normal);
    if (cos_theta <= 0)
    {
        return std::nullopt;
    }

    intersect(ray);
    if (intersection.i == -1 || intersection.i != lightIdx){
        return std::nullopt;
    }

    const auto sample_opt = sampleBSDF(interaction, ray, T);
    if (!sample_opt){
        return std::nullopt;
    }

    const float light_pdf = (intersection.t * intersection.t) / (light_area * cos_light_theta);
    const float mis_weight = light_pdf / (light_pdf + bsdf_pdf);
    return (sample_opt.value() * lightColor * mis_weight * 2) / light_pdf;
}

static void sampleRay(Vector3_t<float> & sample, ray_t ray)
{
    auto throughput = Vector3_t<float>::from_ones(1);

    uint16_t depth = 0;
    while (1)
    {
        intersect(ray);
        if (intersection.i == -1)
        {
            sample += throughput;
            return;
        }

        if (intersection.i < 2)
        {
            if (depth == 0)
            {
                sample += lightColor;
            }
            return;
        }

        const auto [interaction, T] = makeInteraction(intersection, ray);

        const auto radiance = sampleLight(interaction, T).value_or(Vector3_t<float>::from_ones(0));
        sample += radiance * throughput;

        depth++;
        ray =cosWeightedHemi(interaction, T);

        const auto sample_opt = sampleBSDF(interaction, ray, T);
        if (!sample_opt)
        {
            return;
        }

        throughput *= sample_opt.value() / bsdf_pdf;

        if (depth == max_depth)
        {
            return;
        }
    }
}



static Vector3_t<float> samplePixel(const uint X, const uint Y)
{
    auto sample = Vector3_t<float>::from_ones(0);

    static constexpr auto Zc = -LCD_H * real_t(0.5) / tanf(real_t((alpha * TAU / 360) * 0.5f));

    for (size_t i = 0; i < spp; i++)
    {
        const auto u0 = float(rand01());
        const auto u1 = float(rand01());

        const real_t Xw = X + u0;
        const real_t Yw = LCD_H - 1 - Y + u1;
        const real_t Xc = Xw - LCD_W * real_t(0.5);
        const real_t Yc = Yw - LCD_H * real_t(0.5);


        const auto linear_t = Vector3_t(float(Xc), float(Yc), float(Zc));

        const auto temp_direction = Vector3_t{
            Vector3_t(view_x.x, view_y.x, view_z.x).dot(linear_t),
            Vector3_t(view_x.y, view_y.y, view_z.y).dot(linear_t),
            Vector3_t(view_x.z, view_y.z, view_z.z).dot(linear_t)
        }.normalized();

        sampleRay(
            sample, 
            ray_t{
                .start = eye,
                .direction = temp_direction,
                .inv_direction = Vector3_t<float>::from_rcp(temp_direction)
            }
        );
    }

    sample *= inv_spp;

    return sample / (sample + Vector3_t<float>::ONE);
}

static RGB565 draw3drt(const uint X, const uint Y){
    const auto sample = samplePixel(X, Y);
    return RGB565(uint8_t(sample.x * 255), uint8_t(sample.y * 255), uint8_t(sample.z * 255));
}

static void render_row(const std::span<RGB565> row, const uint y){
    // ASSERT(row.size() == LCD_W);

    for (uint x = 0; x < LCD_W; x++){
        row[x] = draw3drt(x, y);
    }
}
#define UART hal::uart2
using drivers::ST7789;

void light_tracking_main(void){

    UART.init(576000);
    DEBUGGER.retarget(&UART);
    DEBUGGER.noBrackets();

    DEBUG_PRINTLN(micros());


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
