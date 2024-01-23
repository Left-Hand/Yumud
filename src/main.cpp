#include "ST7789/st7789.h"
#include "stdio.h"
#include "../math/real.hpp"

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

// #define IMAGE_W 240
// #define IMAGE_H 32

// uint16_t buf[IMAGE_H][IMAGE_W];
volatile uint16_t color = 0xffff;

#define PCOUNT 8

int cube2d[PCOUNT][2];

uint16_t hsvToRgb565(real_t h, real_t s, real_t v) {

    real_t r = real_t(0);
    real_t g = real_t(0);
    real_t b = real_t(0);
    real_t c = v * s;
    real_t x = c * (real_t(1) - std::abs(std::fmod(h / real_t(60), real_t(2)) - real_t(1)));

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


    uint16_t rgb565 = ((uint16_t)(int)(r * real_t(31)) << 11) | ((uint16_t)(int)(g *real_t(63)) << 5) | ((uint16_t)(int)(b *real_t(31)));

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
  real_t tx,ty,temp;
  tx=v[0]-rx;
  ty=v[1]-ry;
  temp=tx*std::cos(q)-ty*std::sin(q);
  ty=tx*std::sin(q)+ty*std::cos(q);
  tx=temp;
  v[0]=tx+rx;
  v[1]=ty+ry;
}
void yrotate(real_t v[3], real_t q, real_t rx, real_t rz) {
  real_t tx,tz,temp;
  tx=v[0]-rx;
  tz=v[2]-rz;
  temp=tz*std::cos(q)-tx*std::sin(q);
  tx=tz*std::sin(q)+tx*std::cos(q);
  tz=temp;
  v[0]=tx+rx;
  v[2]=tz+rz;
}
void xrotate(real_t v[3], real_t q, real_t ry, real_t rz) {
  real_t ty,tz,temp;
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


extern "C"{
int main(){
    // printf("main\r\n");
    RCC_PCLK1Config(RCC_HCLK_Div1);
    RCC_PCLK2Config(RCC_HCLK_Div1);
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

    Systick_Init();
    USART_Printf_Init(921600);

    // printf("start Init\r\n");
    LCD_Init();
    printf("init done\r\n");
    uint32_t begin_m = 0;

    LCD_Fill_Screen(0);

    real_t a = real_t(0);
    real_t b = real_t(0);
    real_t c = real_t(0);

    while(1){
        // for(real_t t = 0; t += 0.31415926f; t < 2 * 3.1415926f){
        //     LCD_Draw_Line(W/2, H/2, W/2 + W/2 * std::cos(t + a), H/2 + H/2 * std::sin(t + a), 0xffff);
        // }
        // printf("%.3f, %.3f, %.3f\r\n", real_t(a), real_t(b), real_t(c));
        // color = 0;
        // draw_cube(real_t(2.9), c, real_t(0), real_t(0));
        // draw_cube(real_t(4.5), b, c, a);
        // draw_cube(real_t(12.1), c, a, b);
        // LCD_Fill_Screen(0);

        a+=real_t(0.04f);
        b+=real_t(0.03f);
        c+=real_t(0.008f);

        color = hsvToRgb565(std::fmod(a*real_t(30), real_t(360)), real_t(1), real_t(1));
        draw_cube(real_t(2.9), c, real_t(0), real_t(0));
        draw_cube(real_t(4.5), b, c, a);
        draw_cube(real_t(12.7), c, a, b);
        // delay(8);
        // delayMicroseconds(8100);
        printf("Fps: %d\r\n", (int)(1000000 / ((micros() - begin_m))));
        begin_m = micros();
    }
}
}