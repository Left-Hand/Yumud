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

#define spp 1
#define max_depth 2


#define INV_PI 0.318310f
#define EPSILON 0.000001f

// static real_t rand01(){
//     static dsp::LCGNoiseSiggen noise;
//     iq_t<16> temp;
//     temp.value.from_i32(noise.update());
//     return frac(temp);
// }

#define rand01() (float(rand()) / RAND_MAX)

static mat4_t T;



struct triangle_t
{
    Vector3_t<float> bbmin, bbmax;
    Vector3_t<float> v0, v1, v2;
    Vector3_t<float> E1, E2, normal;
};

struct intersection_t
{
    int8_t i;
    float t;
};
static struct intersection_t intersection;

struct ray_t
{
    Vector3_t<float> start;
    Vector3_t<float> direction;
    Vector3_t<float> inv_direction;
};
static struct ray_t ray;

struct interaction_t
{
    int8_t i;
    float t;
    const struct triangle_t* surface;
    Vector3_t<float> position;
    Vector3_t<float> normal;
};
static struct interaction_t interaction;




static void makeInteraction(void)
{
    interaction.i = intersection.i;
    interaction.t = intersection.t;
    interaction.surface = &((const struct triangle_t*)triangles)[intersection.i];
    vec3_mad_s(interaction.position, ray.start, ray.direction, intersection.t);
    vec3_assign(interaction.normal, interaction.surface->normal);

    if (dot_m(ray.direction, interaction.normal) < 0)
    {
        vec3_minus(interaction.normal, interaction.normal);
    }

    orthonormalBasis(interaction.normal);
}

static Vector3_t<float> bbmin;
static Vector3_t<float> bbmax;

static float t;
static Vector3_t<float> t0;
static Vector3_t<float> t1;
static Vector3_t<float> temp;
static uint8_t bb_intersect(void)
{
    vec3_sub(t0, bbmin, ray.start);
    t0 *= ray.inv_direction;

    vec3_sub(t1, bbmax, ray.start);
    t1 *= ray.inv_direction;

    vec3_min(temp, t0, t1);
    t = MAX(vec3_compMax(temp), 0);

    vec3_max(temp, t0, t1);
    return vec3_compMin(temp) >= t ? 1 : 0;
}

static const struct triangle_t* s;
static uint8_t tt_intersect(void)
{
    Vector3_t<float> P;
    P = ray.direction.cross(s->E2);
    const float determinant = dot(P, s->E1);
    if (determinant < EPSILON && determinant > -EPSILON)
    {
        return 0;
    }

    const float inv_determinant = 1 / determinant;

    Vector3_t<float> T;
    vec3_sub(T, ray.start, s->v0);
    const float u = dot(P, T) * inv_determinant;
    if (u > 1 || u < 0)
    {
        return 0;
    }

    Vector3_t<float> Q;
    Q = T.cross(s->E1);
    const float v = dot(Q, ray.direction) * inv_determinant;
    if (v > 1 || v < 0 || u + v > 1)
    {
        return 0;
    }

    t = dot(Q, s->E2) * inv_determinant;
    if (t <= 0)
    {
        return 0;
    }

    return 1;
}

static uint8_t tb_intersect(void)
{
    vec3_sub(t0, s->bbmin, ray.start);
    t0 *= ray.inv_direction;

    vec3_sub(t1, s->bbmax, ray.start);
    t1 *= ray.inv_direction;

    vec3_min(temp, t0, t1);
    t = MAX(vec3_compMax(temp), 0);

    vec3_max(temp, t0, t1);
    return vec3_compMin(temp) >= t ? 1 : 0;
}

static void intersect(void)
{
    intersection.t = FLT_MAX;
    intersection.i = -1;

    if (bb_intersect())
    {
        for (auto k = 0; k < 32; ++k)
        {
            s = &((const struct triangle_t*)triangles)[k];
            if (!tb_intersect())
                continue;
            if (tt_intersect())
            {
                if (t < intersection.t)
                {
                    intersection.t = t;
                    intersection.i = k;
                }
            }
        }
    }
}

static Vector3_t<float> reflectance;
static void Reflectance(int8_t i)
{
    if (i == 8 || i == 9)
    {
        vec3_assign_s3(reflectance, 0.05f, 0.65f, 0.05f);
    }
    else if (i == 10 || i == 11)
    {
        vec3_assign_s3(reflectance, 0.65f, 0.05f, 0.05f);
    }
    else
    {
        vec3_assign_s(reflectance, 0.65f);
    }
}

#define abs(x) ((x) > 0 ? x : -x)
static Vector3_t<float> bsdf_absIdotN;
static float bsdf_pdf;

static uint8_t sampleBSDF(void)
{
    Vector3_t<float> wi;
    Vector3_t<float> z;
    vec3_assign_s3(z, T.m[2][0], T.m[2][1], T.m[2][2]);
    wi.z = dot(z, ray.direction);

    if (wi.z <= 0)
    {
        return 0;
    }

    bsdf_pdf = wi.z * INV_PI;

    const float temp = INV_PI * abs(wi.z);
    Reflectance(interaction.i);
    vec3_mul_s(bsdf_absIdotN, reflectance, temp);

    return bsdf_pdf > 0 ? 1 : 0;
}

static float u0;
static float u1;
static float u2;

static Vector3_t<float> linear_r;
static Vector3_t<float> linear_x;
static Vector3_t<float> linear_y;
static Vector3_t<float> linear_z;
static Vector3_t<float> linear_t;
static void linearCombination(void)
{
    linear_r.x = dot(linear_x, linear_t);
    linear_r.y = dot(linear_y, linear_t);
    linear_r.z = dot(linear_z, linear_t);
}
static int8_t lightIdx;
static Vector3_t<float> light_pos;
static void lightPoint(void)
{
    const float su = sqrtf(u0);
    const float x = (1 - su);
    const float y = (1 - u1) * su;
    const float z = u1 * su;
    const struct triangle_t* light = &((const struct triangle_t*)triangles)[lightIdx];
    vec3_assign_s3(linear_t, x, y, z);
    vec3_assign_s3(linear_x, light->v0.x, light->v1.x, light->v2.x);
    vec3_assign_s3(linear_y, light->v0.y, light->v1.y, light->v2.y);
    vec3_assign_s3(linear_z, light->v0.z, light->v1.z, light->v2.z);
    linearCombination();
    vec3_assign(light_pos, linear_r);
}

static void cosWeightedHemi(void)
{
    u0 = float(rand01());
    u1 = float(rand01());

    const float r = sqrtf(u0);
    const float azimuth = u1 * TAU;

    Vector3_t<float> v;
    vec3_assign_s3(v, r * cosf(azimuth), r * sinf(azimuth), sqrtf(1 - u0));

    vec3_mad_s(ray.start, interaction.position, interaction.normal, EPSILON);

    Vector3_t<float> x;
    Vector3_t<float> y;
    Vector3_t<float> z;
    vec3_assign_s3(x, T.m[0][0], T.m[1][0], T.m[2][0]);
    vec3_assign_s3(y, T.m[0][1], T.m[1][1], T.m[2][1]);
    vec3_assign_s3(z, T.m[0][2], T.m[1][2], T.m[2][2]);
    ray.direction.x = dot(x, v);
    ray.direction.y = dot(y, v);
    ray.direction.z = dot(z, v);

    vec3_rcp(ray.inv_direction, ray.direction);
}

#define balanceHeuristic(a, b) ((a) / ((a) + (b)))
static Vector3_t<float> radiance;
static Vector3_t<float> lightColor;
#define light_area 0.0893f
static void sampleLight(void)
{
    vec3_assign_s(radiance, 0);

    u1 = float(rand01());
    u0 = float(rand01());
    u2 = float(rand01());

    lightIdx = u0 < 0.5f ? 0 : 1;

    lightPoint();
    vec3_mad_s(ray.start, interaction.position, interaction.normal, EPSILON);
    vec3_sub(ray.direction, light_pos, ray.start);
    normalize(ray.direction);
    vec3_rcp(ray.inv_direction, ray.direction);

    const struct triangle_t* light = &((const struct triangle_t*)triangles)[lightIdx];
    const float cos_light_theta = dot_m(ray.direction, light->normal);
    if (cos_light_theta <= 0.0)
    {
        return;
    }

    const float cos_theta = dot(ray.direction, interaction.normal);
    if (cos_theta <= 0)
    {
        return;
    }

    intersect();
    if (intersection.i == -1 || intersection.i != lightIdx)
    {
        return;
    }

    if (!sampleBSDF())
    {
        return;
    }

    const float light_pdf = (intersection.t * intersection.t) / (light_area * cos_light_theta);
    const float mis_weight = balanceHeuristic(light_pdf, bsdf_pdf);
    vec3_mul(radiance, bsdf_absIdotN, lightColor);
    vec3_mul_assign_s(radiance, mis_weight);
    vec3_div_assign_s(radiance, light_pdf);
    vec3_div_assign_s(radiance, 0.5f);
}
static uint16_t depth;
static Vector3_t<float> sample;
static void sampleRay(void)
{
    Vector3_t<float> throughput;
    vec3_assign_s(throughput, 1);

    depth = 0;
    while (1)
    {
        intersect();
        if (intersection.i == -1)
        {
            vec3_add_assign(sample, throughput);
            return;
        }

        if (intersection.i < 2)
        {
            if (depth == 0)
            {
                vec3_add_assign(sample, lightColor);
            }
            return;
        }

        makeInteraction();

        sampleLight();
        vec3_mad_assign(sample, radiance, throughput);

        depth++;
        cosWeightedHemi();
        if (!sampleBSDF())
        {
            return;
        }

        throughput *= bsdf_absIdotN;
        vec3_div_assign_s(throughput, bsdf_pdf);

        if (depth == max_depth)
        {
            return;
        }
    }
}


static Vector3_t<float> eye;
static Vector3_t<float> center;
static Vector3_t<float> up;
static Vector3_t<float> view_x;
static Vector3_t<float> view_y;
static Vector3_t<float> view_z;
static struct mat4_t view;
void precompute_rt(void)
{
    vec3_assign_s3(eye, 0, 1, 3.5f);
    vec3_assign_s3(center, 0, 1, 0);
    vec3_assign_s3(up, 0, 1, 0);

    lookat(view, eye, center, up);

    vec3_assign_s3(view_x, view.m[0][0], view.m[0][1], view.m[0][2]);
    vec3_assign_s3(view_y, view.m[1][0], view.m[1][1], view.m[1][2]);
    vec3_assign_s3(view_z, view.m[2][0], view.m[2][1], view.m[2][2]);

    vec3_assign_s3(lightColor, 200, 200, 200);
    vec3_assign_s3(bbmin, -1, 0, -1);
    vec3_assign_s3(bbmax, 1, 2.0f, 1);
}

#define LCD_W 160
#define LCD_H 80

#define alpha 45
static void samplePixel(const uint X, const uint Y)
{
    vec3_assign_s(sample, 0);

    constexpr auto Zc = -LCD_H * real_t(0.5) / tanf(real_t((alpha * TAU / 360) * 0.5f));

    for (auto i = 0; i < spp; i++)
    {
        u0 = float(rand01());
        u1 = float(rand01());

        const real_t Xw = X + u0;
        const real_t Yw = LCD_H - 1 - Y + u1;
        const real_t Xc = Xw - LCD_W * real_t(0.5);
        const real_t Yc = Yw - LCD_H * real_t(0.5);


        vec3_assign_s3(linear_t, float(Xc), float(Yc), float(Zc));
        vec3_assign_s3(linear_x, view_x.x, view_y.x, view_z.x);
        vec3_assign_s3(linear_y, view_x.y, view_y.y, view_z.y);
        vec3_assign_s3(linear_z, view_x.z, view_y.z, view_z.z);
        linearCombination();
        vec3_assign(ray.direction, linear_r);

        vec3_assign(ray.start, eye);
        normalize(ray.direction);
        vec3_rcp(ray.inv_direction, ray.direction);

        sampleRay();
    }

    vec3_div_assign_s(sample, spp);

    Vector3_t<float> one;
    vec3_assign_s(one, 1);

    Vector3_t<float> sample_one;
    vec3_add(sample_one, sample, one);

    vec3_div_assign(sample, sample_one);
}

static RGB565 draw3drt(const uint X, const uint Y){
    samplePixel(X, Y);
    // const uint16_t r = uint16_t(sample.x * 0.1_r);
    // const uint16_t g = uint16_t(sample.y * 0.3_r);
    // const uint16_t b = uint16_t(sample.z * 0.1_r);
    // const uint16_t c16 = (r << 11) | (g << 5) | b;
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

	precompute_rt();

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

    DEBUG_PRINTLN(micros());
}
