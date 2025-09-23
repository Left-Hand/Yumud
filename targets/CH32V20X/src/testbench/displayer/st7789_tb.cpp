
#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/math/realmath.hpp"

#include "hal/bus/uart/uarthw.hpp"
#include "hal/bus/spi/spihw.hpp"

#include "drivers/Display/Polychrome/ST7789/st7789.hpp"
#include "types/image/painter/painter_base.hpp"


using namespace ymd;
using namespace ymd::drivers; 

namespace ymd::drivers{
namespace details{
PRO_DEF_MEM_DISPATCH(_Memfunc_PutPixel, putpixel_unchecked);
PRO_DEF_MEM_DISPATCH(_Memfunc_PutTexture, puttexture_unchecked);
PRO_DEF_MEM_DISPATCH(_Memfunc_PutRect, putrect_unchecked);
}


// using TColor = RGB565;

template<typename TColor>
struct DrawTargetFacade : pro::facade_builder
    ::add_convention<details::_Memfunc_PutPixel, 
    Result<void, DisplayerError>(const Vec2<uint16_t> pos, const TColor color)>
    ::add_convention<details::_Memfunc_PutTexture, 
    Result<void, DisplayerError>(const Rect2<uint16_t> rect, const TColor * pcolor)>
    ::add_convention<details::_Memfunc_PutRect, 
    Result<void, DisplayerError>(const Rect2<uint16_t> rect, const TColor color)>
    ::build {};
}


DEF_ERROR_WITH_KINDS(MyError, ST7789::Error, PainterBase::Error)



#define DBG_UART hal::uart2


void st7789_main(void){

    DBG_UART.init({576000});
    DEBUGGER.retarget(&DBG_UART);
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

    spi.init({144_MHz});
    // spi.init(36_MHz, CommStrategy::Blocking, CommStrategy::None);

    // ST7789 tft({{spi, 0}, lcd_dc, dev_rst}, {240, 134});
    drivers::ST7789 tft(
		drivers::ST7789_Phy{
			&spi, 
			spi.allocate_cs_gpio(&lcd_cs).unwrap(), 
			&lcd_dc, 
			&dev_rst
		}, 
		{240, 135});

	auto target = pro::make_proxy<DrawTargetFacade<RGB565>>(tft);

    DEBUG_PRINTLN("--------------");

	tft.init(drivers::st7789_preset::_240X135{}).examine();

    tft.fill(color_cast<RGB565>(ColorEnum::PINK)).examine();
    clock::delay(200ms);

	while (1){
		auto m = clock::millis();

		// DEBUG_PRINTLN(clock::millis());
	}
}
