#include "ST7789/st7789.h"
#include "stdio.h"
#include "../types/real.hpp"
#include "../types/string/String.hpp"
#include "../types/complex/complex_t.hpp"
#include "../types/color/color_t.hpp"
#include "../types/matrix/matrix.hpp"
#include "MLX90640/MLX90640_API.h"
// #include <complex>

// using real_t = real_t;
using Complex = Complex_t<real_t>;
using Color = Color_t<real_t>;

void Systick_Init(void)
{
    CHECK_INIT

    SysTick->SR  = 0;
    SysTick->CTLR= 0;
    SysTick->CNT = 0;
    SysTick->CMP = SystemCoreClock / 1000 - 1;
    SysTick->CTLR= 0xF;

    NVIC_SetPriority(SysTicK_IRQn,0xFF);
    NVIC_EnableIRQ(SysTicK_IRQn);
}

RGB565 color = 0xffff;
const RGB565 white = 0xffff;
const RGB565 black = 0;
const RGB565 red = RGB565(31,0,0);
const RGB565 green = RGB565(0,63,0);
const RGB565 blue = RGB565(0,0,31);

uint64_t begin_u = 0;
uint64_t begin_m = 0;

struct DebugMeausres{
    uint32_t t_base;
    uint32_t t1;
    uint32_t t2;
    uint32_t t3;
}debugMeausres;

real_t delta = real_t(0);
real_t fps = real_t(0);
real_t fps_filted = real_t(0);
real_t t = real_t(0);

#define PCOUNT 8

int cube2d[PCOUNT][2];

RGB565 hsvToRgb565(real_t h, real_t s, real_t v) {
    h = std::fmod(h, real_t(360));
    real_t r = real_t(0);
    real_t g = real_t(0);
    real_t b = real_t(0);
    real_t c = v * s;
    real_t x = c - c * std::abs(std::fmod(h / 60, real_t(2)) - 1);

    uint16_t ih = (int)h;
    if (ih >= 0 && ih < 60) {
        r = c;
        g = x;
    } else if (ih < 120) {
        r = x;
        g = c;
    } else if (ih < 180) {
        g = c;
        b = x;
    } else if (ih < 240) {
        g = x;
        b = c;
    } else if (ih < 300) {
        r = x;
        b = c;
    } else if (ih < 360){
        r = c;
        b = x;
    }

    RGB565 rgb565 = RGB565(
        (uint8_t)(int)(r * 31),
        (uint8_t)(int)(g * 63),
        (uint8_t)(int)(b * 31)
    );

    return rgb565;
}

RGB565 hToRgb565(real_t h) {
    h = std::fmod(h, real_t(360));
    real_t r = real_t(0);
    real_t g = real_t(0);
    real_t b = real_t(0);
    real_t c = real_t(1);
    real_t x = c - c * std::abs(std::fmod(h / 60, real_t(2)) - 1);

    uint16_t ih = (int)h;
    if (ih >= 0 && ih < 60) {
        r = c;
        g = x;
    } else if (ih < 120) {
        r = x;
        g = c;
    } else if (ih < 180) {
        g = c;
        b = x;
    } else if (ih < 240) {
        g = x;
        b = c;
    } else if (ih < 300) {
        r = x;
        b = c;
    } else if (ih < 360){
        r = c;
        b = x;
    }

    RGB565 rgb565 = RGB565(
        (uint8_t)(int)(31 * r),
        (uint8_t)(int)(63 * g),
        (uint8_t)(int)(31 * b)
    );

    return rgb565;
}

real_t cube3d[PCOUNT][3]={
  { real_t(-0.5f), real_t(+0.5f), real_t(-0.5f) },
  { real_t(+0.5f), real_t(+0.5f), real_t(-0.5f) },
  { real_t(-0.5f), real_t(-0.5f), real_t(-0.5f) },
  { real_t(+0.5f), real_t(-0.5f), real_t(-0.5f) },
  { real_t(-0.5f), real_t(+0.5f), real_t(+0.5f) },
  { real_t(+0.5f), real_t(+0.5f), real_t(+0.5f) },
  { real_t(-0.5f), real_t(-0.5f), real_t(+0.5f) },
  { real_t(+0.5f), real_t(-0.5f), real_t(+0.5f) }
};
void zrotate(real_t v[3], real_t q, real_t rx, real_t ry) {
  real_t tx = real_t(0);
  real_t ty = real_t(0);
  real_t temp = real_t(0);
  tx=v[0]-rx;
  ty=v[1]-ry;
  temp=tx*std::cos(q)-ty*std::sin(q);
  ty=tx*std::sin(q)+ty*std::cos(q);
  tx=temp;
  v[0]=tx+rx;
  v[1]=ty+ry;
}
void yrotate(real_t v[3], real_t q, real_t rx, real_t rz) {
  real_t tx = real_t(0);
  real_t tz = real_t(0);
  real_t temp = real_t(0);
  tx=v[0]-rx;
  tz=v[2]-rz;
  temp=tz*std::cos(q)-tx*std::sin(q);
  tx=tz*std::sin(q)+tx*std::cos(q);
  tz=temp;
  v[0]=tx+rx;
  v[2]=tz+rz;
}
void xrotate(real_t v[3], real_t q, real_t ry, real_t rz) {
  real_t ty = real_t(0);
  real_t tz = real_t(0);
  real_t temp = real_t(0);
  ty=v[1]-ry;
  tz=v[2]-rz;
  temp=ty*std::cos(q)-tz*std::sin(q);
  tz=ty*std::sin(q)+tz*std::cos(q);
  ty=temp;
  v[1]=ty+ry;
  v[2]=tz+rz;
}
void draw_line(int p1, int p2) {
  LCD_Draw_Line(cube2d[p1][0], cube2d[p1][1], cube2d[p2][0], cube2d[p2][1], color);
}

void draw_cube(real_t cz, real_t a, real_t b, real_t c) {
  for (int i=0; i<PCOUNT; i++) {
    real_t *v=cube3d[i];
    real_t tmp[3]={ real_t(v[0]), real_t(v[1]),real_t(v[2]) };
    // printf("%.3f, %.3f, %.3f\r\n", real_t(tmp[0]), real_t(tmp[1]), real_t(tmp[2]));
    
    xrotate(tmp, a, real_t(0), real_t(0));
    yrotate(tmp, b, real_t(0), real_t(0));
    zrotate(tmp, c, real_t(0), real_t(0));
    real_t z=tmp[2]+cz;
    cube2d[i][0]=(int)(tmp[0]/real_t(z)*real_t(W) + real_t(W) / real_t(2));
    cube2d[i][1]=(int)(tmp[1]/real_t(z)*real_t(H)+real_t(H) / real_t(2));
  }
  draw_line(0, 1); draw_line(0, 2); draw_line(0, 4); 
  draw_line(1, 5); draw_line(1, 3);
  draw_line(2, 6); draw_line(2, 3);
  draw_line(4, 6); draw_line(4, 5);
  draw_line(7, 6); draw_line(7, 3); draw_line(7, 5);
}


void renderTest1(){
    static real_t a = real_t(0);
    static real_t b = real_t(0);
    static real_t c = real_t(0);

    a+=real_t(0.04f);
    b+=real_t(0.03f);
    c+=real_t(0.008f);

    draw_cube(real_t(2.9), c, real_t(0), real_t(0));
    draw_cube(real_t(4.5), b, c, a);
    draw_cube(real_t(12.7), c, a, b);

    RGB232 arr[2] = {0};
    LCD_Printf(0, 8, white, "d:%d", sizeof(arr));
    // LCD_Printf(0, 8, 0xffff, "a:%.3f, b:%.3f, c:%.3f", float(a), float(b), float(c));

    LCD_Draw_Hollow_Circle(W/2, H/2, 100, color);
    LCD_Draw_Hollow_Ellipse(W/2, H/2, 100, 119, color);

    for(int i = 0; i< 4; i++){
        LCD_Draw_Filled_Circle(i * 20 + 10, 230, 9, color);
    }

    for(int i = 0; i< 4; i++){
        LCD_Draw_Filled_Ellipse(i * 30 + 100, 230, 14, 9, color);
    }

    for(int i = 0; i< 4; i++){
        LCD_Draw_Filled_Triangle(i * 20, 220, i * 20 + 19, 220,i * 20 + 9, 200, color);
    }

    for(int i = 0; i< 4; i++){
        LCD_Draw_Hollow_Triangle(i * 30 + 85, 220, i * 30 + 85 + 29, 220,i * 30 + 85 + 15, 200, color);
    }
}

void renderTest2(){
    static int Vx = 5;
    static int Vy = 8;
    static int x = W/2;
    static int y = H/2;
    const int r = 10;

    // LCD_Fill_Screen(0);
    
    // LCD_Printf(0, 0, 0xffff, "FPS:%.3f", float(fps));
    LCD_Draw_Filled_Circle(x,y,r,black); 
    LCD_Draw_Filled_Circle(x,y,r,color); 
    // LCD_Draw_Hollow_Ellipse(x, y, 8, 6, color);
    // LCD_Draw_Filled_Rect(0, 0, W/2, 8, 0);


    x=x+Vx;
    y=y+Vy;
    if(x<=r || x>=W-r)
        Vx=-Vx;
    if(y<=r || y>=H-r)
        Vy=-Vy;
}

real_t waveform(real_t x){
    real_t s1x = std::sin(x);
    real_t c1x = std::cos(x);
    real_t s2x = 2 * s1x * c1x;
    real_t c2x = real_t(1) - 2 * s1x * s1x;
    real_t s3x = s1x * c2x + c1x * s2x;
    real_t c3x = c1x * c2x - s1x * s2x;
    real_t s5x = s2x * c3x + c2x * s3x;

    // s1x += s3x / 3;
    // s1x += s5x / 5;
    // return s1x;
    return (s1x + s3x / 3 + s5x / 5);
    // return real_t(0.5f) - std::fmod(x/4, real_t(1));
    // return (std::sin(x) + std::sin(real_t(3) * x) / real_t(3) + std::sin(real_t(5) * x) / real_t(5));
}
void renderTest3(){
    // static int Vx = 5;
    // static int Vy = 8;
    // static int x = W/2;
    // static int y = H/2;
    // const int r = 10;
    const real_t omiga = real_t(58);
    const real_t phi = real_t(0.2);
    const real_t amp = real_t(80);
    const real_t window = real_t(10);
    
    real_t base_angle = omiga*t + phi;
    
    // LCD_Printf(0, 0, 0xffff, "FPS:%.3f", float(fps));
    for(uint16_t x = 0; x < W - 1; x++){
        // debugMeausres.t_base = micros();
        int16_t y0 = (int)(amp * waveform(base_angle + real_t(x - W/2) / real_t(W/2) * (real_t(window) / real_t(2)))) + H/2;
        // debugMeausres.t1 = micros();
        int16_t y1 = (int)(amp * waveform(base_angle + real_t(x + 1 - W/2) / real_t(W/2) * (real_t(window) / real_t(2)))) + H/2;
        // debugMeausres.t2 = micros();
        // RGB565 temp_color = hsvToRgb565(real_t(360) * std::frac(t / real_t(3)) + real_t(0.1)*real_t(x) + real_t(0.2) * 
        // real_t(y0), real_t(1), real_t(1));
        // debugMeausres.t3 = micros();

        if(y0 == y1){
            LCD_Draw_Pixel(x, y0, color);
        }else{
            LCD_Draw_Vertical_Line(x, y0, y1 - y0, color);
        }

        // LCD_Printf(0, 8, 0xffff, "%d, %d, %d", debugMeausres.t1 - debugMeausres.t_base, debugMeausres.t2 - debugMeausres.t_base, debugMeausres.t3 - debugMeausres.t_base);
    }
    // LCD_Draw_Filled_Rect(0, 0, W/2, 8, 0);
}

#define MAXCOUNT 23

uint8_t inter_fract(const real_t & ux, const real_t & uy){
    const real_t & cx = ux;
    const real_t & cy = uy;

    real_t zx = real_t(0);
    real_t zy = real_t(0);
    
    real_t zx_2 = real_t(0);
    real_t zy_2 = real_t(0);

    uint8_t count = 0;

    while ((zx_2 + zy_2 < real_t(4)) && (count < MAXCOUNT))
    {
        zx_2 = zx * zx;
        zy_2 = zy * zy;

        real_t tempx = zx_2 - zy_2 + cx;
        zy = 2 * zx * zy + cy;
        zx = tempx;

        count = count + 1;
    }

    return count;
}

uint8_t inter_fract(const Complex & c){

    Complex z = Complex();
    uint8_t count = 0;
    real_t zx_2 = real_t(0);
    real_t zy_2 = real_t(0);
    while ((zx_2 + zy_2 < real_t(4)) && (count < MAXCOUNT))
    {
        zx_2 = z.real * z.real;
        zy_2 = z.imag * z.imag;
        real_t & zx = z.real;
        real_t & zy = z.imag;
        z = Complex(zx_2 - zy_2 + c.real, 2 * zx * zy + c.imag);
        count = count + 1;
    }
    // Color a = Color<real_t>(1,1,1);
    return count;
}

uint8_t inter_fract2(const Complex & c){

    Complex z(real_t(0), real_t(0));
    uint8_t count = 0;
    // real_t res = real_t();
    // real_t ims = real_t();

    while ((z < real_t(4)) && (count < MAXCOUNT))
    {
        // res = z.real_squared();
        // ims = z.imag_squared();

        // z = Complex(res - ims + c.real, z.real * z.imag * 2 + c.imag);
        z = z*z + c;
        count = count + 1;
    }
    // Color a = Color<real_t>(1,1,1);
    return count;
}

void fractal(real_t left, real_t top, real_t xside, real_t yside)
{

	real_t xstep = xside / W;
	real_t ystep = yside / H;

    real_t cy = top - ystep;
	for (uint8_t y = 0; y < H; y++) {
        
        uint8_t last_count;
        cy += ystep;

        real_t cx = left - xstep;
        RGB565 buf[W];

		for (uint8_t x = 0; x < W; x++){
			cx += xstep;

            // uint8_t count = inter_fract(cx, cy);
            uint8_t count = inter_fract2(Complex(cx,cy));

            if((count != last_count) || (x == 0)) 
                color = hToRgb565(std::fmod(real_t(count * 9), real_t(360)));
            last_count = count;

            buf[x] = color;
		}
        // if(changed){
        LCD_Draw_Textured_Line(0, y, W, buf);
        // LCD_Draw_Textured_Line(0, H - y - 1, W, buf);
        // }else{
        //     LCD_Draw_Vertical_Line(0, y, W, buf[0]);
        //     LCD_Draw_Vertical_Line(0, H - y - 1, W, buf[0]);
        // }
	}
}

void renderTest4(){
    CHECK_INIT
    const float scale = 1.0;
	const real_t left = real_t(0.5)*scale;
	const real_t top = real_t(-1)*scale;
	const real_t xside = real_t(-2)*scale;
	const real_t yside = real_t(2)*scale;

	fractal(left, top, xside, yside);
}

int main(){
    RCC_PCLK1Config(RCC_HCLK_Div1);
    RCC_PCLK2Config(RCC_HCLK_Div1);
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

    Systick_Init();
    USART_Printf_Init(921600);

    MLX90640_Init();

    LCD_Init();

    Color c1 = Color(0.2, 0.8, 0.9);
    // Complex complex1 {real_t(0),real_t(1)};
    // Complex deltacomplex {real_t(0.8),real_t(0.6)};
    while(1){



        // renderTest1();
        // renderTest2();
        // complex1/
        LCD_Fill_Screen(black);
        begin_u = micros();
        // begin_m = millis();

        // LCD_Fill_Screen((RGB565)0);

        renderTest4();
        // color = hsvToRgb565(std::fmod(t, real_t(2))*1180, real_t(1), real_t(1));

        // complex1 *= deltacomplex;
        // LCD_Printf(0, 8, white, "(%.3f, %.3f)", float(complex1.real()), float(complex1.imag()));
        // printf("%.3f, %.3f\r\n", float(complex1.real()), float(complex1.imag()));


        uint64_t delta_u = (micros() - begin_u);
        // uint64_t delta_m = (millis() - begin_m);

        delta = real_t(delta_u / 1000000.0f);

        Matrix m1 = Matrix({1,0});
        if(delta){
            fps = real_t(1) / delta;
            if(!fps_filted){
                fps_filted = (fps_filted * 19 + fps) / 20;
            }else{
                fps_filted = fps;
            }
        }

        LCD_Printf(170, 240 - 1 - 8, black, (String("fps: ") + String(fps) + String("ms") + String(c1.get_h())).c_str());
        // LCD_Printf()
        // printf("%d, %d\r\n", (int)(fps), (int)(fps_filted));
        t += delta;
        // delay(20);
        while(1);
        // delayMicroseconds(1200);
    }
}
