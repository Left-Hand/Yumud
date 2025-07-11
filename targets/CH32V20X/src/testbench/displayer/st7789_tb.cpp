
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
    Result<void, DisplayerError>(const Vector2<uint16_t> pos, const TColor color)>
    ::add_convention<details::_Memfunc_PutTexture, 
    Result<void, DisplayerError>(const Rect2<uint16_t> rect, const TColor * pcolor)>
    ::add_convention<details::_Memfunc_PutRect, 
    Result<void, DisplayerError>(const Rect2<uint16_t> rect, const TColor color)>
    ::build {};
}


#if 0
// 辅助宏：生成单个枚举的别名
#define IMPORT_ENUM_SINGLE(EnumType) \
	using enum EnumType;\

// 递归终止条件（无参数时）
#define IMPORT_ENUMS(...) \
    EXPAND(IMPORT_ENUMS_IMPL(__VA_ARGS__))

// 递归展开宏
#define IMPORT_ENUMS_IMPL(FirstEnum, ...) \
    IMPORT_ENUM_SINGLE(FirstEnum) \
    IF_HAS_ARGS(__VA_ARGS__)(IMPORT_ENUMS_IMPL)(__VA_ARGS__)

// 辅助宏：检查是否有剩余参数
#define IF_HAS_ARGS(...) IF_HAS_ARGS_IMPL(__VA_ARGS__)
#define IF_HAS_ARGS_IMPL(...) ARG_SECOND(__VA_ARGS__, HAS_ARGS, NO_ARGS)
#define ARG_SECOND(_, Second, ...) Second
#define HAS_ARGS(...) EXPAND
#define NO_ARGS(...)

// 确保宏完全展开
#define EXPAND(...) __VA_ARGS__
#endif

DEF_ERROR_WITH_KINDS(MyError, ST7789::Error, PainterBase::Error)

void static_test(){
	constexpr MyError err = ST7789::Error::CantSetup;
	constexpr MyError err2 = PainterBase::Error::CropRectIsNone;

	static_assert(err.is<ST7789::Error>());
	static_assert(err2.is<PainterBase::Error>());
}

#define DBG_UART hal::uart2
void st7789_main(void){

    DBG_UART.init({576000});
    DEBUGGER.retarget(&DBG_UART);
    DEBUGGER.set_eps(4);
    // DEBUGGER.no_brackets();


    #ifdef CH32V30X
    auto & spi = spi2;
    auto & lcd_blk = portC[7];
    
    lcd_blk.outpp(HIGH);

    auto & lcd_cs = portD[6];
    auto & lcd_dc = portD[7];
    auto & dev_rst = portB[7];
    #else
    auto & spi = hal::spi1;
    auto & lcd_blk = hal::portA[10];
    auto & lcd_cs = hal::portA[15];
    auto & lcd_dc = hal::portA[11];
    auto & dev_rst = hal::portA[12];


    
    
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

	tft.init(drivers::ST7789_Presets::_240X135).examine();
    tft.fill(RGB565(ColorEnum::PINK)).examine();
    clock::delay(200ms);

	while (1){
		auto m = clock::millis();

		DEBUG_PRINTLN(clock::millis());
	}
}
