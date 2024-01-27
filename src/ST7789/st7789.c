
#include "st7789.h"

SPI_InitTypeDef  SPI_InitStructure;

void LCD_GPIO_Init(void){
    CHECK_INIT

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |
        RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure = {0};

    //PA15 DC

    GPIO_InitStructure.GPIO_Pin = LCD_DC_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(LCD_DC_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = LCD_RES_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(LCD_RES_PORT, &GPIO_InitStructure);

    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
    GPIO_PinRemapConfig(GPIO_Remap_SPI1, ENABLE);
}

void LCD_Write_Const_16b(uint16_t data, uint32_t length){
    SPI1_Write_Const_16b(data, length);
}

void LCD_Write_Pool_16b(uint16_t * data, uint32_t length){
    SPI1_Write_Pool_16b(data, length);
}

void LCD_Init(){

    LCD_GPIO_Init();					 
    SPI1_Init();

    #ifdef SPI1_USE_DMA
    DMA1_CH3_Init((void *)(&ConstData), (void *)(&SPI1->DATAR));	
    #endif

    LCD_RESET_RES
    delayMicroseconds(5);
    LCD_SET_RES

    LCD_Write_Command(0x01);

    delayMicroseconds(5);
	LCD_Write_Command(0x11);
	
    delayMicroseconds(5);
	LCD_Write_Command(0x3A);
	LCD_Write_Data_8b(0x55);
	LCD_Write_Command(0x36);
	LCD_Write_Data_8b(0x00);
	LCD_Write_Command(0x21);
	LCD_Write_Command(0x13);
	LCD_Write_Command(0x29);
	
}

void LCD_Write_Data_8b(uint8_t data){
   LCD_ON_DATA
   SPI1_Write_8b(data);
}

void LCD_Write_Data_16b(uint16_t data){
   LCD_ON_DATA
   SPI1_Write_8b((uint8_t)(data >> 8));

   SPI1_Write_8b((uint8_t)data);

}

void LCD_Write_Command(uint8_t cmd){
    LCD_ON_COMMAND
    SPI1_Write_8b(cmd);
}

void LCD_Set_Postion(uint16_t x0, uint16_t y0){
	LCD_Write_Command(0x2a);
    LCD_Write_Data_16b(x0);
	
	LCD_Write_Command(0x2b);
    LCD_Write_Data_16b(y0);

    LCD_Write_Command(0x2c);
}

void LCD_Set_Window(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h){
	LCD_Write_Command(0x2a);
    LCD_Write_Data_16b(x0);
    LCD_Write_Data_16b(x0 + w);
	
	LCD_Write_Command(0x2b);
    LCD_Write_Data_16b(y0);
    LCD_Write_Data_16b(y0 + h);

    LCD_Write_Command(0x2c);
}

void LCD_Draw_Pixel(int16_t x0, int16_t y0, RGB565 color){
    if(x0 < 0 || y0 < 0 || x0 >= W || y0 >= H) return;

	LCD_Set_Postion(x0, y0);
    LCD_Write_Data_16b(color.data);
}

void LCD_Cont_Pixel(RGB565 color){
    LCD_Write_Data_16b(color.data);
}

void LCD_Draw_Pixel_Unwarpped(int16_t x0, int16_t y0, RGB565 color){
	LCD_Write_Command(0x2a);
    LCD_Write_Data_16b(x0);

	LCD_Write_Command(0x2b);
    LCD_Write_Data_16b(y0);

    LCD_Write_Command(0x2c);
    LCD_Write_Data_16b(color.data);
}

void LCD_Draw_Hrizon_Line(int16_t x0, int16_t y0, int16_t l, RGB565 color){
    int16_t xa = MAX(MIN(x0, x0 + l), 0);
    int16_t xb = MIN(MAX(x0, x0 + l), W);
    if(xb <= xa || y0 < 0 || y0 >= H) return;

    uint16_t dx = xb - xa;
    LCD_Set_Postion(xa, y0);

    LCD_ON_DATA
    LCD_Write_Const_16b(color.data, dx);
}

void LCD_Draw_Vertical_Line(int16_t x0, int16_t y0, int16_t l, RGB565 color){
    int16_t ya = MAX(MIN(y0, y0 + l), 0);
    int16_t yb = MIN(MAX(y0, y0 + l), H);
    if(x0 < 0 || x0 >= W || ya >= yb) return;

    
    for(uint16_t y = ya; y < yb; y++){
        LCD_Set_Postion(x0, y);
        LCD_Write_Data_16b(color.data);
    }
}

void LCD_Draw_Filled_Rect(int16_t x0, int16_t y0, int16_t w, int16_t h, RGB565 color){
    int16_t xa = MAX(MIN(x0, x0 + w), 0);
    int16_t ya = MAX(MIN(y0, y0 + h), 0);
    int16_t xb = MIN(MAX(x0, x0 + w), W);
    int16_t yb = MIN(MAX(y0, y0 + h), H);

    if(xb < 0 || xa >= W || xa == xb ||
        yb < 0 || ya >= H || ya == yb) return;

    uint16_t dx = xb - xa;

    for(uint16_t y = ya; y < yb; y++){
        LCD_Draw_Hrizon_Line(xa, y, dx, color);
    }
}

void LCD_Draw_Hollow_Rect(int16_t x0, int16_t y0, int16_t w, int16_t h, RGB565 color){
    int16_t xa = MAX(MIN(x0, x0 + w), 0);
    int16_t ya = MAX(MIN(y0, y0 + h), 0);
    int16_t xb = MIN(MAX(x0, x0 + w), W);
    int16_t yb = MIN(MAX(y0, y0 + h), H);

    uint16_t dx = xb - xa;
    uint16_t dy = yb - ya;

    if(xb < 0 || xa >= W || xa == xb ||
        yb < 0 || ya >= H || ya == yb) return;

    if(dy >= 2){
        LCD_Draw_Hrizon_Line(xa, ya, dx, color);
        LCD_Draw_Hrizon_Line(xa, yb, dx, color);
        LCD_Draw_Vertical_Line(xa, ya + 1, dy - 2, color);
        LCD_Draw_Vertical_Line(xb, ya + 1, dy - 2, color);
    }else{
        LCD_Draw_Hrizon_Line(xa, ya, dx, color);
    }
}

void LCD_Draw_Line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, RGB565 color){
    if(y0 == y1){
        LCD_Draw_Hrizon_Line(x0, y0, x1 - x0, color);
        return;
    }else if (x0 == x1){
        LCD_Draw_Vertical_Line(x0, y0, y1 - y0, color);
        return;
    }
    
    int16_t dx=ABS(x1-x0);
    int8_t sx=(x1 > x0)? 1: -1;
    int16_t dy=ABS(y1-y0);
    int8_t sy=(y1 > y0)? 1 : -1;

    int16_t err=(dx>dy ? dx : -dy)/2;
    int16_t e2;
    
    int16_t x = x0;
    int16_t y = y0;

    while(1){
        LCD_Draw_Pixel_Unwarpped(x, y, color);
        if (x==x1 && y==y1) 
            break;

        e2=err;
        if (e2>-dx) { 
            err-=dy; 
            x+=sx; 
        }
        if (e2<dy) { 
            err+=dx; 
            y+=sy; 
        }
    }
}

void LCD_Fill_Screen(RGB565 color){
    for(uint16_t y = 0; y < H; y++)
        LCD_Draw_Hrizon_Line(0,y,W,color);
}

void LCD_Draw_Textured_Line(int16_t x0, int16_t y0, int16_t l, RGB565 * buf){
    if(y0 < 0 || l == 0) return;
    if((x0 < 0 && l < 0) || (x0 >= W && l > 0)) return;

    int16_t xa = MAX(MIN(x0, x0 + l), 0);
    int16_t xb = MIN(MAX(x0, x0 + l), W);


    uint16_t times;
    uint16_t x;

    if(x0 < 0){
        x = 0;
        times = xb - x;
    }else if(x0 >= W){
        x = xa;
        times = W - 1 - x;
    }else{
        x = xa;
        times = xb - x;
    }

    LCD_Set_Postion(x, y0);
    LCD_ON_DATA;
    LCD_Write_Pool_16b(&(buf[l-times].data), times);
}

void LCD_Draw_Image(int16_t x0, int16_t y0, int16_t w, int16_t h, RGB565 * buf){
    uint16_t xa = MIN(x0, x0 + w);
    int16_t xb = MIN(MAX(x0, x0 + w), W);
    uint16_t ya = MIN(y0, y0 + h);
    int16_t yb = MIN(MAX(y0, y0 + h), H);

    if( xb >= W || xa == xb || yb >= H || ya == yb) return;

    // uint16_t dx = xb - xa;
    // uint16_t dy = yb - ya;

    LCD_Set_Postion(xa, ya);

    LCD_ON_DATA
    LCD_Write_Pool_16b(&(buf[0].data), w*h);
}

void LCD_Draw_Hollow_Circle(int16_t x0, int16_t y0, int16_t r, RGB565 color){
	if(r<0)
		return;
    int16_t x = r - 1;
    int16_t y = 0;
    int16_t dx = 1;
    int16_t dy = 1;
    int16_t err=dx - 2 * r;

    while (x>=y) {
        LCD_Draw_Pixel(x0-x, y0+y, color);
        LCD_Draw_Pixel(x0+x, y0+y, color);
        LCD_Draw_Pixel(x0-y, y0+x, color);
        LCD_Draw_Pixel(x0+y, y0+x, color);
        LCD_Draw_Pixel(x0-x, y0-y, color);
        LCD_Draw_Pixel(x0+x, y0-y, color);
        LCD_Draw_Pixel(x0-y, y0-x, color);
        LCD_Draw_Pixel(x0+y, y0-x, color);

        if (err<=0) {
            y++;
            err += dy;
            dy += 2;
        }
        if (err>0) {
            x--;
            dx += 2;
            err += dx-r * 2;
        }
    }
}

void LCD_Draw_Filled_Circle(int16_t x0, int16_t y0, int16_t r, RGB565 color){
	if(r<0)
		return;

    int16_t x = r - 1;
    int16_t y = 0;
    int16_t dx = 1;
    int16_t dy = 1;
    int16_t err=dx - 2 * r;

    while (x>=y) {
        LCD_Draw_Hrizon_Line(x0 - x, y0 + y, 2*x, color);
        LCD_Draw_Hrizon_Line(x0 - y, y0 + x, 2*y, color);
        LCD_Draw_Hrizon_Line(x0 - x, y0 - y, 2*x, color);
        LCD_Draw_Hrizon_Line(x0 - y, y0 - x, 2*y, color);

        if (err<=0) {
            y++;
            err += dy;
            dy += 2;
        }
        if (err>0) {
            x--;
            dx += 2;
            err += dx-r * 2;
        }
    }
}

void LCD_Draw_Hollow_Ellipse(int16_t x0, int16_t y0, int16_t rx, int16_t ry, RGB565 color) {
    if (rx<2) return;
    if (ry<2) return;
    if (rx == ry) return LCD_Draw_Hollow_Circle(x0, y0, rx, color);
    int16_t x, y;
    int32_t rx2 = rx * rx;
    int32_t ry2 = ry * ry;
    int32_t fx2 = 4 * rx2;
    int32_t fy2 = 4 * ry2;
    int32_t s;

    for (x = 0, y = ry, s = 2*ry2+rx2*(1-2*ry); ry2*x <= rx2*y; x++) {
        // These are ordered to minimise coordinate changes in x or y
        // LCD_Draw_Pixel can then send fewer bounding box commands
        LCD_Draw_Pixel(x0 + x, y0 + y, color);
        LCD_Draw_Pixel(x0 - x, y0 + y, color);
        LCD_Draw_Pixel(x0 - x, y0 - y, color);
        LCD_Draw_Pixel(x0 + x, y0 - y, color);
        if (s >= 0) {
            s += fx2 * (1 - y);
            y--;
        }
        s += ry2 * ((4 * x) + 6);
    }

    for (x = rx, y = 0, s = 2*rx2+ry2*(1-2*rx); rx2*y <= ry2*x; y++) {
        LCD_Draw_Pixel(x0 + x, y0 + y, color);
        LCD_Draw_Pixel(x0 - x, y0 + y, color);
        LCD_Draw_Pixel(x0 - x, y0 - y, color);
        LCD_Draw_Pixel(x0 + x, y0 - y, color);
        if (s >= 0)
        {
            s += fy2 * (1 - x);
            x--;
        }
        s += rx2 * ((4 * y) + 6);
    }
}

void LCD_Draw_Filled_Ellipse(int16_t x0, int16_t y0, int16_t rx, int16_t ry, RGB565 color){
    if (rx<2) return;
    if (ry<2) return;
    if (rx == ry) return LCD_Draw_Filled_Circle(x0, y0, rx, color);

    int16_t x, y;
    int32_t rx2 = rx * rx;
    int32_t ry2 = ry * ry;
    int32_t fx2 = 4 * rx2;
    int32_t fy2 = 4 * ry2;
    int32_t s;

    for (x = 0, y = ry, s = 2*ry2+rx2*(1-2*ry); ry2*x <= rx2*y; x++) {
        LCD_Draw_Hrizon_Line(x0 - x, y0 - y, x + x + 1, color);
        LCD_Draw_Hrizon_Line(x0 - x, y0 + y, x + x + 1, color);

        if (s >= 0) {
            s += fx2 * (1 - y);
            y--;
        }
        s += ry2 * ((4 * x) + 6);
    }

    for (x = rx, y = 0, s = 2*rx2+ry2*(1-2*rx); rx2*y <= ry2*x; y++) {
        LCD_Draw_Hrizon_Line(x0 - x, y0 - y, x + x + 1, color);
        LCD_Draw_Hrizon_Line(x0 - x, y0 + y, x + x + 1, color);

        if (s >= 0) {
            s += fy2 * (1 - x);
            x--;
        }
        s += rx2 * ((4 * y) + 6);
    }
}

void LCD_Draw_Hollow_Triangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, RGB565 color){
    LCD_Draw_Line(x0, y0, x1, y1, color);
    LCD_Draw_Line(x0, y0, x2, y2, color);
    LCD_Draw_Line(x1, y1, x2, y2, color);  
}

void LCD_Draw_Filled_Triangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, RGB565 color){
    int32_t a, b, y, last;

    if (y0 > y1) {
        SWAP(y0, y1, int16_t);
        SWAP(x0, x1, int16_t);
    }
    if (y1 > y2) {
        SWAP(y2, y1, int16_t);
        SWAP(x2, x1, int16_t);
    }
    if (y0 > y1) {
        SWAP(y0, y1, int16_t);
        SWAP(x0, x1, int16_t);
    }

    if (y0 == y2) { // Handle awkward all-on-same-line case as its own thing
        a = b = x0;
        if (x1 < a)      a = x1;
        else if (x1 > b) b = x1;
        if (x2 < a)      a = x2;
        else if (x2 > b) b = x2;
        LCD_Draw_Hrizon_Line(a, y0, b - a + 1, color);
        return;
    }


    int32_t
    dx01 = x1 - x0,
    dy01 = y1 - y0,
    dx02 = x2 - x0,
    dy02 = y2 - y0,
    dx12 = x2 - x1,
    dy12 = y2 - y1,
    sa   = 0,
    sb   = 0;

    if (y1 == y2) last = y1;  // Include y1 scanline
    else         last = y1 - 1; // Skip it

    for (y = y0; y <= last; y++) {
        a   = x0 + sa / dy01;
        b   = x0 + sb / dy02;
        sa += dx01;
        sb += dx02;

        if (a > b) SWAP(a, b, int16_t);
        LCD_Draw_Hrizon_Line(a, y, b - a + 1, color);
    }

    sa = dx12 * (y - y1);
    sb = dx02 * (y - y0);
    for (; y <= y2; y++) {
        a   = x1 + sa / dy12;
        b   = x0 + sb / dy02;
        sa += dx12;
        sb += dx02;

        if (a > b) SWAP(a, b, int16_t);
        LCD_Draw_Hrizon_Line(a, y, b - a + 1, color);
    }
}

void LCD_Draw_Char(int16_t x0, int16_t y0, RGB565 color, char chr){
    const uint8_t font_w = 6;
    const uint8_t font_h = 8;
    uint8_t * ptr = (uint8_t *)&font_en[MAX(chr - 32, 0)];
    for(uint8_t i = 0; i < font_w; i++){
        uint8_t b_data = ptr[i];
        uint8_t mask = 0x01;
        for(uint8_t j = 0; j < font_h; j++){
            if(b_data & mask){
                LCD_Draw_Pixel(x0 + i, y0 + j, color);
            }
            mask <<= 1;
        }
    }
}

void LCD_Draw_String(int16_t x0, int16_t y0, RGB565 color, char * str){
    const uint8_t font_w = 6;
    const uint8_t font_space = 1;
    char * str_ptr = str;

    for(int16_t x = x0; x < W; x += (font_w + font_space)){
        if(*str_ptr){
            LCD_Draw_Char(x, y0, color, *str_ptr);
        }else{
            break;
        }
        str_ptr++;
    }
}

void LCD_Draw_Const_String(int16_t x0, int16_t y0, RGB565 color, const char * str){
    LCD_Draw_String(x0, y0, color, (char *)str);
}

void LCD_Printf(int16_t x0, int16_t y0, RGB565 color, const char *format, ...){
    va_list pArgs;
    // char * str = (char *)malloc(64);
    char str[64] = {0};

    va_start(pArgs,format);
    vsnprintf(str,64,format,pArgs); 
    va_end(pArgs);

    LCD_Draw_String(x0, y0, color, str);

    // free(str);
}

