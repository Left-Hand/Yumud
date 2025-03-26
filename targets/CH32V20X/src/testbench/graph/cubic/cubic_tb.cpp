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

#include "hal/bus/uart/uarthw.hpp"
#include "hal/bus/spi/spihw.hpp"

#include "drivers/Display/Polychrome/ST7789/st7789.hpp"

#include "dsp/siggen/noise/LCGNoiseSiggen.hpp"
#include "data.hpp"

using namespace ymd;

#define WIRE 0
#define DEPTH 0

static constexpr size_t LCD_W = 96;
static constexpr size_t LCD_H = 80;


#define RADIANS(x) ((3.1415926f / 180) * (x))


static uint8_t k, l, n;
#define mat4_mul_mat4(r, a, b) \
{ \
	for (k = 0; k < 4; ++k) \
	{ \
		for (l = 0; l < 4; ++l) \
		{ \
			r.m[k][l] = 0; \
			for (n = 0; n < 4; ++n) \
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
	for (k = 0; k < 4; ++k) \
	{ \
		r.v[k] = a.m[3][k]; \
		for (l = 0; l < 3; ++l) \
		{ \
			r.v[k] += a.m[l][k] * b.v[l]; \
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
			r.v[k] += a.m[l][k] * b.v[l]; \
		} \
	} \
}

#define dot(a, b) (a.x * b.x + a.y * b.y + a.z * b.z)

#define cross(r, a, b) \
{ \
	r.x = a.y * b.z - a.z * b.y; \
	r.y = a.z * b.x - a.x * b.z; \
	r.z = a.x * b.y - a.y * b.x; \
}

#define normalize(v) \
{ \
	const float r = sqrtf(dot(v, v)); \
	if (r > 0.0f) \
	{ \
		v.x /= r; \
		v.y /= r; \
		v.z /= r; \
	} \
}

#define vec3_sub(r, a, b) \
{ \
	r.x = a.x - b.x; \
	r.y = a.y - b.y; \
	r.z = a.z - b.z; \
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
	struct vec3_t zaxis; \
	vec3_sub(zaxis, eye, center); \
	normalize(zaxis); \
 \
	struct vec3_t xaxis; \
	cross(xaxis, up, zaxis); \
	normalize(xaxis); \
 \
	struct vec3_t yaxis; \
	cross(yaxis, zaxis, xaxis); \
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
	view.m[3][0] = -dot(xaxis, eye); \
	view.m[3][1] = -dot(yaxis, eye); \
	view.m[3][2] = -dot(zaxis, eye); \
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



#define TILE_W 32
#define TILE_H 16


using buffer_t = std::array<std::array<uint16_t, TILE_W>, TILE_H>;
static buffer_t colorbuffer;
static uint8_t clipX0;
static uint8_t clipY0;
static uint8_t clipW;
static uint8_t clipH;

struct vertex_t
{
	float rhw;
	struct vec2i_t point;
#if DEPTH
	float point_z;
#endif
};
struct triangle_t
{
	uint8_t v0;
	uint8_t v1;
	uint8_t v2;
	struct vec2_t texcoord0;
	struct vec2_t texcoord1;
	struct vec2_t texcoord2;
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
// static std::array<triangle_t, cubic_index.size() / 3> vTriangle;
static std::array<triangle_t, cubic_index.size()> vTriangle;
static uint8_t triangleIdx;

static struct mat4_t mvp;
static float rhw;
static struct vec4_t position;
static float point_x;
static float point_y;
static uint8_t v0;
static uint8_t v1;
static uint8_t v2;
static int16_t area;
static uint8_t xmin;
static uint8_t xmax;
static uint8_t ymin;
static uint8_t ymax;
static void makeTriangle(void)
{
	for (size_t i = 0; i < vertex.size(); i++)
	{

		mat4_mul_vec3_h(position, mvp, vertex[i]);
		rhw = 1.0f / position.w;
		point_x = (position.x * rhw + 1.0f) * LCD_W * 0.5f;
		point_y = (1.0f - position.y * rhw) * LCD_H * 0.5f;
#if DEPTH
		vVertex[i].point_z = (1.0f - position.z * rhw) * 0.5f;
#endif
		vVertex[i].point.x = (int16_t)(point_x + 0.5f);
		vVertex[i].point.y = (int16_t)(point_y + 0.5f);
		vVertex[i].rhw = rhw;
	}

	triangleIdx = 0;
	for (size_t i = 0; i < cubic_index.size(); i += 3)
	{
		auto & triangle = vTriangle[triangleIdx];
		v0 = cubic_index[i + 0];
		v1 = cubic_index[i + 1];
		v2 = cubic_index[i + 2];
#define perpdot0(a, b) ((0 - a.x) * (b.y - a.y) - (0 - a.y) * (b.x - a.x))
		triangle.W12 = perpdot0(vVertex[v1].point, vVertex[v2].point);
		triangle.W20 = perpdot0(vVertex[v2].point, vVertex[v0].point);
		triangle.W01 = perpdot0(vVertex[v0].point, vVertex[v1].point);
		area = triangle.W12 + triangle.W20 + triangle.W01;
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
		xmin = (uint8_t)(LCD_W - 1);
		xmax = 0;
		ymin = (uint8_t)(LCD_H - 1);
		ymax = 0;
#define bbox(v) \
		{xmin = MAX(0, MIN(xmin, v.x));} \
		{xmax = MIN((uint8_t)(LCD_W - 1), MAX(xmax, v.x));} \
		{ymin = MAX(0, MIN(triangle.ymin, v.y));} \
		{ymax = MIN((uint8_t)(LCD_H - 1), MAX(ymax, v.y));}
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
static struct vec3_t uv;
static float u, v;
static int16_t I12offset;
static int16_t I20offset;
static int16_t I01offset;
static int16_t J12offset;
static int16_t J20offset;
static int16_t J01offset;
static int16_t wy12;
static int16_t wy20;
static int16_t wy01;
static int16_t w12;
static int16_t w20;
static int16_t w01;
#if WIRE
static void setPixel(int16_t x, int16_t y)
{
	if (x < clipX0 || x >= clipW || y < clipY0 || y >= clipH)
		return;
	colorbuffer[bufferID].buffer[y - clipY0][x - clipX0] = 0xffff;
}
static inline int16_t abs(int16_t x) { return x > 0 ? x : -x; }
static void plotLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1)
{
	int16_t dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
	int16_t dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
	int16_t err = dx + dy, e2; /* error value e_xy */

	for (;;) {  /* loop */
		setPixel(x0, y0);
		if (x0 == x1 && y0 == y1) break;
		e2 = 2 * err;
		if (e2 >= dy) { err += dy; x0 += sx; } /* e_xy+e_x > 0 */
		if (e2 <= dx) { err += dx; y0 += sy; } /* e_xy+e_y < 0 */
	}
}
#endif
static void drawTriangle(void)
{
	for (k = 0; k < triangleIdx; k++)
	{
		const auto & triangle = vTriangle[k];
		xmin = triangle.xmin;
		xmax = triangle.xmax;
		ymin = triangle.ymin;
		ymax = triangle.ymax;
		if (xmin < clipX0) xmin = clipX0;
		if (xmin >= clipW) continue;
		if (xmax < clipX0) continue;
		if (xmax >= clipW) xmax = clipW - 1;
		if (ymin < clipY0) ymin = clipY0;
		if (ymin >= clipH) continue;
		if (ymax < clipY0) continue;
		if (ymax >= clipH) ymax = clipH - 1;

		v0 = triangle.v0;
		v1 = triangle.v1;
		v2 = triangle.v2;

		I12offset = triangle.I12 * xmin;
		I20offset = triangle.I20 * xmin;
		I01offset = triangle.I01 * xmin;

		J12offset = triangle.J12 * ymin;
		J20offset = triangle.J20 * ymin;
		J01offset = triangle.J01 * ymin;

		wy12 = triangle.W12 + J12offset;
		wy20 = triangle.W20 + J20offset;
		wy01 = triangle.W01 + J01offset;

		for (size_t y = ymin; y <= ymax; y++)
		{
			w12 = wy12 + I12offset;
			w20 = wy20 + I20offset;
			w01 = wy01 + I01offset;

			for (size_t x = xmin; x <= xmax; x++)
			{
				do
				{
					if ((w12 < 0) || (w20 < 0) || (w01 < 0))
						break;

					uv.x = w12 * triangle.area_r;
					uv.y = w20 * triangle.area_r;
					uv.z = w01 * triangle.area_r;

					struct vec3_t vertex_rhw;
					vertex_rhw.x = vVertex[v0].rhw;
					vertex_rhw.y = vVertex[v1].rhw;
					vertex_rhw.z = vVertex[v2].rhw;
					const float rhw_r = 1.0f / dot(vertex_rhw, uv);
					struct vec3_t uv_rhw;
					uv_rhw.x = uv.x * rhw_r;
					uv_rhw.y = uv.y * rhw_r;
					uv_rhw.z = uv.z * rhw_r;
#if DEPTH
					struct vec3_t point_z;
					point_z.x = vVertex[v0].point_z;
					point_z.y = vVertex[v1].point_z;
					point_z.z = vVertex[v2].point_z;
					const float pointz = dot(point_z, uv_rhw);
					const uint16_t depth = pointz * 0xffff;
					if (depth < depthbuffer.buffer[y - clipY0][x - clipX0])
						break;
					depthbuffer.buffer[y - clipY0][x - clipX0] = depth;
#endif
					struct vec3_t texcoordU;
					texcoordU.x = triangle.texcoord0.x;
					texcoordU.y = triangle.texcoord1.x;
					texcoordU.z = triangle.texcoord2.x;
					u = dot(texcoordU, uv_rhw);
					struct vec3_t texcoordV;
					texcoordV.x = triangle.texcoord0.y;
					texcoordV.y = triangle.texcoord1.y;
					texcoordV.z = triangle.texcoord2.y;
					v = dot(texcoordV, uv_rhw);
					{
						const uint16_t texcolor = doge[(uint8_t)(u * 31 + 0.5f)][(uint8_t)(v * 31 + 0.5f)];
						colorbuffer[y - clipY0][x - clipX0] = texcolor;
					}

				} while (0);

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

static struct mat4_t vp;

static constexpr float fovy = RADIANS(45.0f);
static constexpr float aspect = (float)LCD_W / LCD_H;
static constexpr float znear = 0.1f;
static constexpr float zfar = 10.0f;
static constexpr vec3_t eye = {2.5, 2.5, 2.5};
static constexpr vec3_t center = {0,0,0};
static constexpr vec3_t up = {0,1,0};

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
static float angle;
static void precompute_2(void)
{
	struct mat4_t r;
	{
		const float a = RADIANS(angle);
		rotate(r, a, 0, 1, 0);
	}

	mat4_mul_mat4(mvp, vp, r);
}

#define UART hal::uart2

void cubic_main(void){

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
    drivers::ST7789 tftDisplayer({{spi, 0}, lcd_dc, dev_rst}, {240, 135});
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
		// tftDisplayer.set
    }

    tftDisplayer.fill(ColorEnum::PINK);
    delay(200);
	precompute_1();
	
	std::array<std::array<uint16_t, 32>, 32> tex;
	memcpy(tex.begin()->begin(), &doge[0], 32 * 32 * 2);
	for(auto & row : tex){
		for(auto & item : row)item = uint16_t(item >> 8) | uint16_t(item << 8);
	}
	while (1){
		tftDisplayer.put_texture(Rect2_t<uint16_t>(160, 80, 32, 32), 
		// tftDisplayer.put_texture(Rect2i::from_cross(Vector2_t<uint16_t>(32 + clipX0, clipY0), Vector2_t<uint16_t>(32, 16)), 
			reinterpret_cast<const RGB565 *>(&tex[0][0]));

		precompute_2();
		makeTriangle();
		angle += 2;

		static constexpr auto M = (LCD_W/TILE_W);
		static constexpr auto N = (LCD_H/TILE_H);
		for (size_t i = 0; i < M; ++i)
		{
			for (size_t j = 0; j < N; ++j)
			{
				clipX0 = LCD_W * i / M;
				clipY0 = LCD_H * j / N;
				clipW = clipX0 + LCD_W / M;
				clipH = clipY0 + LCD_H / N;
				// drawTriangle(buffer);
				for (uint8_t x = 0; x < TILE_W; x++)
				{
					for (uint8_t y = 0; y < TILE_H; y++)
					{
						colorbuffer[y][x] = 0;
					}
				}
				drawTriangle();

				tftDisplayer.put_texture(Rect2i::from_cross(Vector2_t<uint16_t>(32 + clipX0, clipY0), Vector2_t<uint16_t>(32 + clipW, clipH)), 
				// tftDisplayer.put_texture(Rect2i::from_cross(Vector2_t<uint16_t>(32 + clipX0, clipY0), Vector2_t<uint16_t>(32, 16)), 
					reinterpret_cast<const RGB565 *>(&colorbuffer[0][0]));

			}
		}
	}
}
