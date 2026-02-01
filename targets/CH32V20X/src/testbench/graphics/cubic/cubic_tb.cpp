/*
* STC8H8K64U-45I-PDIP40 @40MHz
* LCD16080 ST7735
* sdcc 3dcolor.c --model-large
* https://liuliqiang.com/51/3dcolor.c
* by liuliqiang 2025-03-01
*/

#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/math/realmath.hpp"

#include "hal/bus/uart/hw_singleton.hpp"
#include "hal/bus/spi/hw_singleton.hpp"

#include "algebra/vectors/vec3.hpp"

#include "drivers/Display/Polychrome/ST7789/st7789.hpp"

#include "dsp/siggen/noise/LCGNoiseSiggen.hpp"
#include "data.hpp"

using namespace ymd;


#define RADIANS(x) ((3.1415926f / 180) * (x))

static constexpr size_t LCD_W = 96*2;
static constexpr size_t LCD_H = 112;
static constexpr float fovy = RADIANS(45.0f);
static constexpr float aspect = (float)LCD_W / LCD_H;
static constexpr float znear = 0.1f;
static constexpr float zfar = 10.0f;
static constexpr math::Vec3<float> eye = {2.5, 2.5, 2.5};
static constexpr math::Vec3<float> center = {0,0,0};
static constexpr math::Vec3<float> up = {0,1,0};



#define mat4_mul_mat4(r, a, b) \
{ \
	for (size_t k = 0; k < 4; ++k) \
	{ \
		for (size_t l = 0; l < 4; ++l) \
		{ \
			r.m[k][l] = 0; \
			for (size_t n = 0; n < 4; ++n) \
			{ \
				r.m[k][l] += a.m[n][l] * b.m[k][n]; \
			} \
		} \
	} \
}

#define mat4_mul_vec4(r, a, b) \
{ \
	for (k = 0; k < 4; ++k) \
	{ \
		r.v[k] = 0; \
		for (l = 0; l < 4; ++l) \
		{ \
			r.v[k] += a.m[l][k] * b.v[l]; \
		} \
	} \
}

#define mat4_mul_vec3_h(r, a, b) \
{ \
	for (size_t k = 0; k < 4; ++k) \
	{ \
		r.v[k] = a.m[3][k]; \
		for (size_t l = 0; l < 3; ++l) \
		{ \
			r.v[k] += a.m[l][k] * b[l]; \
		} \
	} \
}

#define mat4_mul_vec3(r, a, b) \
{ \
	for (k = 0; k < 3; ++k) \
	{ \
		r.v[k] = 0; \
		for (l = 0; l < 3; ++l) \
		{ \
			r.v[k] += a.m[l][k] * b[l]; \
		} \
	} \
}


#define perspective(proj, fovy, aspect, znear, zfar) \
{ \
	const float cot = 1.0f / tanf(fovy / 2.0f); \
	const float sum = (znear + zfar); \
	const float sub = (znear - zfar); \
 \
	proj.m[0][0] = cot / aspect; \
	proj.m[0][1] = 0.0f; \
	proj.m[0][2] = 0.0f; \
	proj.m[0][3] = 0.0f; \
 \
	proj.m[1][0] = 0.0f; \
	proj.m[1][1] = cot; \
	proj.m[1][2] = 0.0f; \
	proj.m[1][3] = 0.0f; \
 \
	proj.m[2][0] = 0.0f;\
	proj.m[2][1] = 0.0f;\
	proj.m[2][2] = sum / sub; \
	proj.m[2][3] = -1.0f; \
 \
	proj.m[3][0] = 0.0f; \
	proj.m[3][1] = 0.0f; \
	proj.m[3][2] = 2.0f * znear * zfar / sub; \
	proj.m[3][3] = 0.0f; \
}

#define lookat(view, eye, center, up) \
{ \
	const auto zaxis = (eye - center).normalized(); \
	const auto xaxis = (up.cross(zaxis).normalized()); \
	const auto yaxis = zaxis.cross(xaxis); \
 \
	view.m[0][0] = xaxis.x; \
	view.m[0][1] = yaxis.x; \
	view.m[0][2] = zaxis.x; \
	view.m[0][3] = 0.0f; \
 \
	view.m[1][0] = xaxis.y; \
	view.m[1][1] = yaxis.y; \
	view.m[1][2] = zaxis.y; \
	view.m[1][3] = 0.0f; \
 \
	view.m[2][0] = xaxis.z; \
	view.m[2][1] = yaxis.z; \
	view.m[2][2] = zaxis.z; \
	view.m[2][3] = 0.0f; \
 \
	view.m[3][0] = -xaxis.dot(eye); \
	view.m[3][1] = -yaxis.dot(eye); \
	view.m[3][2] = -zaxis.dot(eye); \
	view.m[3][3] = 1.0f; \
}

#define rotate(r, a, x, y, z) \
{ \
	const float c = cosf(a); \
	const float s = sinf(a); \
	const float onec = 1.0f - c; \
 \
	r.m[0][0] = x * x * onec + c; \
	r.m[0][1] = y * x * onec + z * s; \
	r.m[0][2] = x * z * onec - y * s; \
	r.m[0][3] = 0.0f; \
 \
	r.m[1][0] = x * y * onec - z * s; \
	r.m[1][1] = y * y * onec + c; \
	r.m[1][2] = y * z * onec + x * s; \
	r.m[1][3] = 0.0f; \
 \
	r.m[2][0] = x * z * onec + y * s; \
	r.m[2][1] = y * z * onec - x * s; \
	r.m[2][2] = z * z * onec + c; \
	r.m[2][3] = 0.0f; \
 \
	r.m[3][0] = 0.0f; \
	r.m[3][1] = 0.0f; \
	r.m[3][2] = 0.0f; \
	r.m[3][3] = 1.0f; \
}

#define perpdot0(a, b) ((0 - a.x) * (b.y - a.y) - (0 - a.y) * (b.x - a.x))

#define bbox(v) \
		{xmin = MAX(0, MIN(xmin, v.x));} \
		{xmax = MIN((uint8_t)(LCD_W - 1), MAX(xmax, v.x));} \
		{ymin = MAX(0, MIN(triangle.ymin, v.y));} \
		{ymax = MIN((uint8_t)(LCD_H - 1), MAX(ymax, v.y));}

static constexpr size_t TILE_W = 32;
static constexpr size_t TILE_H = 16;


using buffer_t = std::array<std::array<RGB565, TILE_W>, TILE_H>;
static buffer_t colorbuffer;
// static uint8_t clipX0;
// static uint8_t clipY0;
// static uint8_t clipX1;
// static uint8_t clipY1;

struct vertex_t
{
	float rhw;
	math::Vec2<int> point = math::Vec2<int>::ZERO;
};

struct triangle_t
{
	uint8_t v0;
	uint8_t v1;
	uint8_t v2;
	math::Vec2<float> texcoord0 = math::Vec2<float>::ZERO;
	math::Vec2<float> texcoord1 = math::Vec2<float>::ZERO;
	math::Vec2<float> texcoord2 = math::Vec2<float>::ZERO;
	int16_t W12;
	int16_t W20;
	int16_t W01;
	float area_r;
	int8_t I12;
	int8_t I20;
	int8_t I01;
	int8_t J12;
	int8_t J20;
	int8_t J01;
	uint8_t xmin;
	uint8_t xmax;
	uint8_t ymin;
	uint8_t ymax;
};

static std::array<vertex_t, vertex.size()> vVertex;
static std::array<triangle_t, cubic_index.size() / 3> vTriangle;
static uint8_t triangleIdx;

static struct mat4_t mvp;
static struct mat4_t vp;
// static int16_t area;

static void makeTriangle(void)
{
	for (size_t i = 0; i < vertex.size(); i++)
	{
		std::array<float, 4> position;

		for (size_t k = 0; k < 4; ++k) { 
			position[k] = mvp.m[3][k]; 
			for (size_t l = 0; l < 3; ++l) { 
				position[k] += mvp.m[l][k] * vertex[i][l]; 
			} 
		} 

		const auto rhw = 1.0f / position[3];
		const auto point_x = (position[0] * rhw + 1.0f) * LCD_W * 0.5f;
		const auto point_y = (1.0f - position[1] * rhw) * LCD_H * 0.5f;
		vVertex[i].point.x = (int16_t)(point_x + 0.5f);
		vVertex[i].point.y = (int16_t)(point_y + 0.5f);
		vVertex[i].rhw = rhw;
	}

	triangleIdx = 0;
	for (size_t i = 0; i < cubic_index.size(); i += 3)
	{
		auto & triangle = vTriangle[triangleIdx];
		const auto v0 = cubic_index[i + 0];
		const auto v1 = cubic_index[i + 1];
		const auto v2 = cubic_index[i + 2];

		triangle.W12 = perpdot0(vVertex[v1].point, vVertex[v2].point);
		triangle.W20 = perpdot0(vVertex[v2].point, vVertex[v0].point);
		triangle.W01 = perpdot0(vVertex[v0].point, vVertex[v1].point);
		const auto area = triangle.W12 + triangle.W20 + triangle.W01;
		if (area <= 0)
			continue;

		triangle.area_r = 1.0f / area;
		triangle.v0 = v0;
		triangle.v1 = v1;
		triangle.v2 = v2;
		triangle.I12 = vVertex[v2].point.y - vVertex[v1].point.y;
		triangle.I20 = vVertex[v0].point.y - vVertex[v2].point.y;
		triangle.I01 = vVertex[v1].point.y - vVertex[v0].point.y;
		triangle.J12 = -(vVertex[v2].point.x - vVertex[v1].point.x);
		triangle.J20 = -(vVertex[v0].point.x - vVertex[v2].point.x);
		triangle.J01 = -(vVertex[v1].point.x - vVertex[v0].point.x);
		auto xmin = (uint8_t)(LCD_W - 1);
		auto xmax = 0;
		auto ymin = (uint8_t)(LCD_H - 1);
		auto ymax = 0;

		bbox(vVertex[v0].point);
		bbox(vVertex[v1].point);
		bbox(vVertex[v2].point);

		triangle.xmin = xmin;
		triangle.xmax = xmax;
		triangle.ymin = ymin;
		triangle.ymax = ymax;
		triangle.texcoord0.x = texcoord[i + 0].x * vVertex[v0].rhw;
		triangle.texcoord0.y = texcoord[i + 0].y * vVertex[v0].rhw;
		triangle.texcoord1.x = texcoord[i + 1].x * vVertex[v1].rhw;
		triangle.texcoord1.y = texcoord[i + 1].y * vVertex[v1].rhw;
		triangle.texcoord2.x = texcoord[i + 2].x * vVertex[v2].rhw;
		triangle.texcoord2.y = texcoord[i + 2].y * vVertex[v2].rhw;
		triangleIdx++;
	}
}

static void drawTriangle(const math::Rect2u & clip)
{
	for (size_t k = 0; k < triangleIdx; k++)
	{
		const auto & triangle = vTriangle[k];
		auto xmin = triangle.xmin;
		auto xmax = triangle.xmax;
		auto ymin = triangle.ymin;
		auto ymax = triangle.ymax;

		const auto clipX0 = clip.x();
		const auto clipY0 = clip.y();
		const auto clipX1 = clip.x() + clip.w();
		const auto clipY1 = clip.y() + clip.h();

		if (xmin < clipX0) xmin = clipX0;
		if (xmin >= clipX1) continue;
		if (xmax < clipX0) continue;
		if (xmax >= clipX1) xmax = clipX1 - 1;
		if (ymin < clipY0) ymin = clipY0;
		if (ymin >= clipY1) continue;
		if (ymax < clipY0) continue;
		if (ymax >= clipY1) ymax = clipY1 - 1;

		const auto v0 = triangle.v0;
		const auto v1 = triangle.v1;
		const auto v2 = triangle.v2;

		const auto I12offset = triangle.I12 * xmin;
		const auto I20offset = triangle.I20 * xmin;
		const auto I01offset = triangle.I01 * xmin;
		const auto J12offset = triangle.J12 * ymin;
		const auto J20offset = triangle.J20 * ymin;
		const auto J01offset = triangle.J01 * ymin;

		auto wy12 = triangle.W12 + J12offset;
		auto wy20 = triangle.W20 + J20offset;
		auto wy01 = triangle.W01 + J01offset;

		for (size_t y = ymin; y <= ymax; y++)
		{
			auto w12 = wy12 + I12offset;
			auto w20 = wy20 + I20offset;
			auto w01 = wy01 + I01offset;

			for (size_t x = xmin; x <= xmax; x++)
			{
				if ((w12 >= 0) and (w20 >= 0) and (w01 >= 0)){
					const auto uv = math::Vec3{
						w12 * triangle.area_r,
						w20 * triangle.area_r,
						w01 * triangle.area_r,
					};

					math::Vec3<float> vertex_rhw = {
						vVertex[v0].rhw,
						vVertex[v1].rhw,
						vVertex[v2].rhw,
					};

					const auto uv_rhw = uv / vertex_rhw.dot(uv);
					const auto u = math::Vec3(triangle.texcoord0.x, triangle.texcoord1.x, triangle.texcoord2.x).dot(uv_rhw);
					const auto texcoordV = math::Vec3(triangle.texcoord0.y, triangle.texcoord1.y, triangle.texcoord2.y);
					const auto v = texcoordV.dot(uv_rhw);
					const auto texcolor = doge[uint(u * 31 + 0.5f)][uint(v * 31 + 0.5f)];
					colorbuffer[y - clipY0][x - clipX0] = RGB565::from_u16(texcolor);
				}

				w12 += triangle.I12;
				w20 += triangle.I20;
				w01 += triangle.I01;
			}

			wy12 += triangle.J12;
			wy20 += triangle.J20;
			wy01 += triangle.J01;
		}
	}
}





static void precompute_1(void)
{
	struct mat4_t proj;
	{

		perspective(proj, fovy, aspect, znear, zfar);
	}

	struct mat4_t view;
	{

		lookat(view, eye, center, up);
	}

	mat4_mul_mat4(vp, proj, view);
}
static void precompute_2(const float angle)
{
	struct mat4_t r;
	{
		const float a = RADIANS(angle);
		rotate(r, a, 0, 1, 0);
	}

	mat4_mul_mat4(mvp, vp, r);
}

#define UART hal::usart2

void cubic_main(void){

    DEBUGGER_INST.init({
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

    spi.init({
		.remap = hal::SPI1_REMAP_PA5_PA6_PA7_PA4,
		.baudrate = hal::NearestFreq(144_MHz)
	});

    drivers::ST7789 tft{
		drivers::ST7789_Transport{
			&spi, 
			spi.allocate_cs_pin(&lcd_cs).unwrap(), 
			&lcd_dc, 
			&dev_rst
		},
		{240, 135}
	};

    DEBUG_PRINTLN("--------------");

	tft.init(drivers::st7789_preset::_240X135{}).examine();
    tft.fill(color_cast<RGB565>(ColorEnum::PINK)).examine();
    clock::delay(200ms);
	precompute_1();

	float angle = 0;

	while (1){
		auto m = clock::millis();
		precompute_2(angle += 4);
		makeTriangle();

		static constexpr auto M = (LCD_W/TILE_W);
		static constexpr auto N = (LCD_H/TILE_H);
		for (size_t i = 0; i < M; ++i)
		{
			for (size_t j = 0; j < N; ++j)
			{

				// drawTriangle(buffer);
				for (uint8_t x = 0; x < TILE_W; x++)
				{
					for (uint8_t y = 0; y < TILE_H; y++)
					{
						colorbuffer[y][x] = RGB565::from_u16(0);
					}
				}


				const auto clip = math::Rect2u(
					math::Vec2u{LCD_W * i / M,
					LCD_H * j / N},
					math::Vec2u{LCD_W / M,
					LCD_H / N}
				);

				// DEBUG_PRINTLN(clip);

				drawTriangle(clip);

				tft.put_texture(clip, &colorbuffer[0][0]).examine();

			}
		}
		DEBUG_PRINTLN(clock::millis() - m);
	}
}
