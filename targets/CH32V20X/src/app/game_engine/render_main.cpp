

#include "src/testbench/tb.h"

#include "core/clock/time.hpp"
#include "core/utils/nth.hpp"
#include "core/utils/stdrange.hpp"
#include "core/utils/data_iter.hpp"
#include "core/utils/rescaler.hpp"
#include "core/stream/fixed_string_stream.hpp"

#include "hal/gpio/gpio_port.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/timer/timer.hpp"
#include "hal/analog/adc/adcs/adc1.hpp"
#include "hal/bus/uart/uartsw.hpp"
#include "hal/gpio/gpio.hpp"
#include "hal/bus/spi/spihw.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/bus/i2c/i2csw.hpp"



#include "types/image/painter/painter.hpp"
#include "types/colors/rgb/rgb.hpp"
#include "types/regions/rect2.hpp"

#include "types/vectors/quat.hpp"
#include "types/image/image.hpp"
#include "types/image/font/font.hpp"

#include "types/shapes/bresenham_iter.hpp"
#include "types/shapes/rotated_rect.hpp"
#include "types/shapes/box_rect.hpp"
#include "types/regions/Segment2.hpp"
#include "types/shapes/triangle2.hpp"  
#include "types/shapes/line_iter.hpp"
#include "types/shapes/annular_sector.hpp"
#include "types/shapes/horizon_spectrum.hpp"
#include "types/shapes/triangle2.hpp"
#include "types/shapes/oval2.hpp"
#include "types/shapes/gridmap2.hpp"
#include "types/shapes/rounded_rect2.hpp"

#include "drivers/Display/Polychrome/ST7789/st7789.hpp"
#include "drivers/IMU/Axis6/MPU6050/mpu6050.hpp"
#include "drivers/IMU/Magnetometer/QMC5883L/qmc5883l.hpp"

#include "robots/rpc/rpc.hpp"
#include "robots/repl/repl_service.hpp"
#include "robots/mock/mock_burshed_motor.hpp"

#include "frame_buffer.hpp"
#include "core/string/utils/strconv2.hpp"


namespace ymd{
template<typename T>
struct is_placed_t<Segment2<T>>:std::true_type{};


struct MonoFont7x7 final{
    using font_item_t = font_res::chfont_7x7_item_t;

    constexpr MonoFont7x7(){;}
	uint32_t get_row_pixels(const wchar_t token, const uint8_t row_nth) {

		if(token != last_token_){
            last_token_ = token;
            p_last_font_item_ = find_font_item(token);

            if(p_last_font_item_){
                for(uint8_t i = 0; i < 7; i++){
                    buf[i] = p_last_font_item_->data[i];
                }
            }
		}

		if(p_last_font_item_){
			return buf[row_nth];
		}else{
			return 0u;
		}
	}

    constexpr Vec2u16 size() const {
        return Vec2u16{7,7};
    }
private:
    wchar_t last_token_ = 0;
    const font_item_t * p_last_font_item_ = nullptr;
    uint8_t buf[7] = {0};


    const font_item_t * find_font_item(wchar_t code) const{
        size_t left = 0;
        const auto & res = font_res::chfont_7x7;
        size_t right = std::size(res) - 1;
        
        while (left <= right) {
            // size_t mid = left + ((right - left) >> 1);
            size_t mid = ((right + left) >> 1);
            
            if (res[mid].code == code) {
                return &res[mid];
            } else if (res[mid].code < code) {
                left = mid + 1;
            } else {
                right = mid - 1;
            }
        }
        
        return nullptr;
    }
};

struct MonoFont8x5 final{
    constexpr MonoFont8x5() = default;
	static uint32_t get_row_pixels(const wchar_t token, const uint8_t row_nth) {
        auto & row_data = font_res::enfont_8x5[MAX(token - ' ', 0)];
        uint32_t row_mask = 0;

        for(uint8_t x = 0; x < 5; x++){
            row_mask |= uint32_t(bool(uint8_t(row_data[x]) & uint8_t(1 << row_nth))) << (x);
        }

        return row_mask;
	}


    static constexpr Vec2u16 size() {
        return Vec2u16{5,8};
    }
public:
};

struct MonoFont16x8 final{
    constexpr MonoFont16x8() = default;
	uint32_t get_row_pixels(const wchar_t token, const uint8_t row_nth) {
        auto & col_data = font_res::enfont_16x8[MAX(token - ' ', 0)];
        uint32_t row_mask = 0;

        row_mask = col_data[row_nth];

        return row_mask;
	}


    constexpr Vec2u16 size() const {
        return Vec2u16{8,16};
    }
public:
};

template<typename Encoding, typename Font>
struct LineText{
    Vec2u16 left_top;
    uint16_t spacing;
    StringView str;
    // MonoFont7x7 & font;
    // MonoFont7x7 font;
    Font font;

    constexpr Rect2u16 bounding_box() const {
        const size_t str_len = str.length();
        const uint16_t width = (str_len * font.size().x) + (str_len - 1) * spacing;
        const uint16_t height = font.size().y;

        return Rect2u16{left_top, {width, height}};
    };
};

template<typename Encoding, typename Font>
struct is_placed_t<LineText<Encoding, Font>>:std::true_type{;};


}


using namespace ymd;




template<typename Iter>
static constexpr size_t count_iter(Iter && iter){
    size_t cnt = 0;
    while(true){
        if(iter.has_next()){
            (void)iter.next();
            cnt++;
        }else{
            break;
        }
    }

    return cnt;
}


static constexpr auto UART_BAUD = 576000u;

// static constexpr auto LCD_WIDTH = 32u;
// static constexpr auto LCD_HEIGHT = 18u;

// static constexpr auto LCD_WIDTH = 8u;
// static constexpr auto LCD_HEIGHT = 6u;

static constexpr auto LCD_WIDTH = 320u;
static constexpr auto LCD_HEIGHT = 170u;




template<typename Shape>
// auto make_DrawDispatchIterator
auto make_draw_dispatch_iterator(Shape && shape){
    return DrawDispatchIterator<std::decay_t<Shape>>(shape);
}


template<typename T>
requires (std::is_integral_v<T>)
struct DrawDispatchIterator<Rect2<T>> {
    using Shape = Rect2<T>;
    using Self = DrawDispatchIterator<Shape>;

    constexpr DrawDispatchIterator(const Shape& shape) : 
        x_range_(shape.x_range()),
        y_range_(shape.y_range()),
        y_(y_range_.start) {}  // 修复：使用 y_range_.start

    template<DrawTargetConcept Target, typename Color>
    Result<void, typename Target::Error> draw_filled(Target& target, const Color& color) {
        if (y_ >= y_range_.stop) return Ok();  // 安全检查
        
        auto res = target.fill_x_range(x_range_, color);  // 可能需要传入 y 坐标
        if (res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    template<DrawTargetConcept Target, typename Color>
    Result<void, typename Target::Error> draw_hollow(Target& target, const Color& color) {
        if (y_ >= y_range_.stop) return Ok();  // 安全检查
        
        // 如果是顶部或底部边界，绘制整行
        if (y_ == y_range_.start || y_ == y_range_.stop - 1) {
            auto res = target.fill_x_range(x_range_, color);
            if (res.is_err()) return Err(res.unwrap_err());
        } 
        // 如果是中间行，只绘制左右两端
        else {
            // 绘制左端点
            if (auto res = target.fill_x_range(Range2<T>::from_start_and_length(x_range_.start, 1), color);
                res.is_err()) return Err(res.unwrap_err());
            
            // 绘制右端点（注意：x_range_.stop 是 exclusive）
            if (auto res = target.fill_x_range(Range2<T>::from_start_and_length(x_range_.stop - 1, 1), color);
                res.is_err()) return Err(res.unwrap_err());
        }
        return Ok();
    }

    constexpr bool has_next() const {
        return y_ < y_range_.stop;
    }

    constexpr void forward() {
        if (y_ < y_range_.stop) {
            y_++;
        }
    }

private:
    Range2<T> x_range_;
    Range2<T> y_range_;
    T y_;
};



// DrawDispatchIterator 特化
template<std::integral T>
struct DrawDispatchIterator<Segment2<T>> {
    using Segment = Segment2<T>;
    // using Iterator = BresenhamIterator<T>;
    using Iterator = LineDDAIterator<T>;

    constexpr DrawDispatchIterator(const Segment& segment)
        : iter_(segment){}

    // 检查是否还有下一行
    constexpr bool has_next() const {
        return iter_.has_next();
    }

    // 推进到下一行
    constexpr void forward() {
        iter_.advance();
    }

    // 绘制当前行的所有点
    template<DrawTargetConcept Target, typename Color>
    Result<void, typename Target::Error> draw_filled(Target& target, const Color& color) {
        if (!has_next()) return Ok();
        
        // 绘制当前行的范围
        const auto x_range = iter_.x_range();
        auto res = target.fill_x_range(x_range, color);
        if (res.is_err()) return Err(res.unwrap_err());
        
        return Ok();
    }

    // 空心绘制（对于线段来说和填充一样）
    template<DrawTargetConcept Target, typename Color>
    Result<void, typename Target::Error> draw_hollow(Target& target, const Color& color) {
        return draw_filled(target, color);
    }


private:
    Iterator iter_;
};


// DrawDispatchIterator 特化
template<std::integral T>
struct DrawDispatchIterator<Circle2<T>> {
    using Shape = Circle2<T>;
    using Iterator = CircleBresenhamIterator<T>;

    constexpr DrawDispatchIterator(const Shape & shape)
        : iter_(shape){}

    // 检查是否还有下一行
    constexpr bool has_next() const {
        return iter_.has_next();
    }

    // 推进到下一行
    constexpr void forward() {
        iter_.advance();
    }

    // 绘制当前行的所有点
    template<DrawTargetConcept Target, typename Color>
    Result<void, typename Target::Error> draw_filled(Target& target, const Color& color) {
        // 绘制当前行的范围
        const auto x_range = iter_.x_range();
        auto res = target.fill_x_range(x_range, color);
        if (res.is_err()) return Err(res.unwrap_err());
        
        return Ok();
    }

    template<DrawTargetConcept Target, typename Color>
    Result<void, typename Target::Error> draw_hollow(Target& target, const Color& color) {
        // 绘制当前行的范围
        const auto [left_x_range, right_x_range] = iter_.left_and_right();
        if(const auto res = target.fill_x_range(left_x_range, color);
            res.is_err()) return res;
        if(const auto res = target.fill_x_range(right_x_range, color);
            res.is_err()) return res;
        return Ok();
    }


private:
    Iterator iter_;
};




// DrawDispatchIterator 特化
template<std::integral T, typename D>
struct DrawDispatchIterator<HorizonSpectrum<T, D>> {
    using Shape = HorizonSpectrum<T, D>;
    using Transformer = Rescaler<q16>;
    constexpr DrawDispatchIterator(const Shape & shape)
        : shape_(shape),
            transformer_(Transformer::from_input_and_inverted_output(
                shape_.sample_range,
                shape_.bounding_box().y_range()
            )),
            y_(shape.top_left.y)
        {}

    // 检查是否还有下一行
    constexpr bool has_next() const {
        return y_ < shape_.top_left.y + shape_.cell_size.y;
    }

    // 推进到下一行
    constexpr void forward() {
        y_++;
    }

    // 绘制当前行的所有点
    template<DrawTargetConcept Target, typename DestColor>
    Result<void, typename Target::Error> draw_filled(Target& target, const DestColor & dest_color) {
        
        T x = shape_.top_left.x;
        // const auto stop_x = .stop;
        
        const T count = MIN(
            shape_.samples.size(),
            target.bounding_box().x_range().length() / shape_.cell_size.x
        );

        // const T count = shape_.samples.size();

        const auto color = color_cast<RGB565>(dest_color);

        #pragma GCC unroll(4)
        for(size_t i = 0; i < count; i++){
            // const auto old_x = x;
            const T next_x = x + shape_.cell_size.x;
            const auto x_range = Range2u16::from_start_and_stop_unchecked(x, next_x);
            x = next_x + shape_.spacing;

            const auto data_y = transformer_(shape_.samples[i]);

            if((y_ >= data_y)){
                if(const auto res = target.fill_x_range(x_range, color);
                    res.is_err()) return Err(res.unwrap_err());
            }else{
                // if(const auto res = target.fill_x_range(x_range, color);
                //     res.is_err()) return res;
            }

            continue;
        }
        return Ok();
    }

    template<DrawTargetConcept Target, typename Color>
    Result<void, typename Target::Error> draw_hollow(Target& target, const Color& color) {
        return draw_filled(target, color);
    }


private:
    Shape shape_;
    Transformer transformer_;
    T y_;
};



namespace ymd{
template<typename Encoding, typename Font>
struct DrawDispatchIterator<LineText<Encoding, Font>> {
    using Shape = LineText<Encoding, Font>;
    using Self = DrawDispatchIterator<Shape>;

    constexpr DrawDispatchIterator(const Shape& shape) : 
        shape_(shape),
        x_range_(shape.bounding_box().x_range()),
        y_range_(shape.bounding_box().y_range()),
        y_(shape.bounding_box().y_range().start) {}  // 修复：使用 y_range_.start

    template<DrawTargetConcept Target, typename DestColor>
    Result<void, typename Target::Error> draw_filled(Target& target, const DestColor& dest_color) {
        size_t x_base = x_range_.start;
        const size_t font_w = shape_.font.size().x;

        const auto y_offset = y_ - y_range_.start;
        const auto color = static_cast<RGB565>(dest_color);


        // for(const auto gbk_token : StdRange(GBKIterator(shape_.str))){
        // for(const auto gbk_token : StdRange(GBKIterator("这个驱动已经完成"))){
        // for(const auto gbk_token : shape_.str){
        const auto str = shape_.str;
        // for(size_t i = 0; i < str.length() -5; i++){
        for(size_t i = 0; i < str.length(); i++){
            const auto gbk_token = str[i];
        // for(const auto gbk_token : "123456789abcdABCD"){
        // for(const auto gbk_token : "abcdefghijklmnopqrstuvwxyz"){
        // for(const auto gbk_token : "123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"){
        // for(const auto gbk_token : "SPINRATE"){

            const auto row_pixels = shape_.font.get_row_pixels(gbk_token, y_offset);

            if(unlikely(row_pixels == 0)) continue;

            // #pragma GCC unroll(4)
            // #pragma GCC unroll 4
            for(size_t j = 0; j < font_w; j++){
                if((row_pixels & (1 << j)) == 0) continue;
                // if(const auto res = target.draw_x_unchecked(
                if(const auto res = target.draw_x(
                    // x_base + (font_w - j), color); 
                    x_base + j, color); res.is_err()) return res;
                continue;
            }

            x_base += (font_w + shape_.spacing);
        }
        return Ok();
    }

    constexpr bool has_next() const {
        return y_ < y_range_.stop;
    }

    constexpr void forward() {
        if (y_ < y_range_.stop) {
            y_++;
        }
    }

private:
    Shape shape_;
    Range2<uint16_t> x_range_;
    Range2<uint16_t> y_range_;
    uint16_t y_;
};
}



void render_main(){


    auto & DBG_UART = DEBUGGER_INST;
    auto init_debugger = []{

        DBG_UART.init({
            .baudrate = UART_BAUD
        });

        DEBUGGER.retarget(&DBG_UART);
        DEBUGGER.set_eps(4);
        DEBUGGER.set_splitter(",");
        // DEBUGGER.no_brackets(EN);
    };



    init_debugger();

    #ifdef CH32V30X
    auto & spi = hal::spi2;
    #else
    auto & spi = hal::spi1;
    #endif

    spi.init({144_MHz});
    
    auto scl_gpio = hal::PB<3>();
    auto sda_gpio = hal::PB<5>();
    
    hal::I2cSw i2c{&scl_gpio, &sda_gpio};
    i2c.init({400_KHz});

    drivers::QMC5883L qmc{&i2c};
    retry(2, [&]{return qmc.init();}).examine();

    


    auto lcd_blk = hal::PD<0>();
    lcd_blk.outpp(HIGH);

    auto lcd_dc = hal::PD<7>();
    auto dev_rst = hal::PB<7>();
    auto lcd_cs = hal::PD<4>();

    const auto spi_rank = spi.allocate_cs_gpio(&lcd_cs).unwrap();

    drivers::ST7789 tft{
        drivers::ST7789_Phy{&spi, spi_rank, &lcd_dc, &dev_rst}, 
        {LCD_WIDTH, LCD_HEIGHT}
    };

    tft.init(drivers::st7789_preset::_320X170{}).examine();


    // std::array<RGB565, LCD_WIDTH * LCD_HEIGHT> render_buffer;
    std::array<RGB565, LCD_WIDTH> render_buffer;

    [[maybe_unused]] auto plot_rgb = [&](
        const std::span<RGB565> & src, 
        const ScanLine line
    ){
        // DEBUG_PRINTLN(line.bounding_box());
        tft.put_texture(
            line.bounding_box(),
            src.data()
        ).examine();
    };

    [[maybe_unused]] auto ch_font = MonoFont7x7{};
    [[maybe_unused]] auto en_font = MonoFont8x5{};
    [[maybe_unused]] auto en_font2 = MonoFont16x8{};

    while(true){


        const auto ctime = clock::time();
        // const auto dest_angle = Angle<q16>::from_turns(ctime * 0.3_r);
        const auto dest_angle = Angle<q16>::from_turns(ctime * 0.1_r);
        // [[maybe_unused]] const auto [s,c] = sincospu(ctime * 0.3_r);
        [[maybe_unused]] const auto [s, c] = dest_angle.sincos();
        [[maybe_unused]] const auto [shape_x, shape_y] = std::make_tuple(
            uint16_t(30 + 20 * c), uint16_t(30 + 20 * s));

        [[maybe_unused]] const auto samples = [&]{
            static constexpr auto LEN = 20;
            std::array<q16, LEN> ret;
            for(size_t i = 0; i < LEN; i++){
                // ret[i] = 0.8_q16 * sin(7 * ctime + i * 0.15_r);
                ret[i] = 0.8_q16 * sinpu(ctime + i * 0.1_r);
            }
            return ret;
        } ();

        // const auto shapes = std::to_array<Rect2u16>({
        //     Rect2u16{shape_x,shape_y,5,5},
        //     Rect2u16{shape_x,uint16_t(shape_y+ 10),5,5},
        //     Rect2u16{uint16_t(shape_x + 10),shape_y,5,5},
        //     Rect2u16{shape_x,uint16_t(shape_y- 10),5,5},
        //     Rect2u16{uint16_t(shape_x - 10),shape_y,5,5}
        // });
        // using Vec2u16 = Vec2<uint16_t>;
        // auto shape =  Rect2u16{shape_x,shape_y,120,60};

        #if 0
        auto shape = Segment2<uint16_t>{
            Vec2u16{uint16_t(50 + 20 * c), uint16_t(80 + 20 * s)},
            Vec2u16{50,80}
        };
        #endif 

        #if 0
        auto shape = VerticalOval2<uint16_t>::from_bounding_box(
            Rect2u16{
                Vec2u16{0,0},
                Vec2u16{120,170},
                // Vec2u16{14,60},
                // Vec2u16{12,60},
            }
        ).unwrap();
        // auto shape = Rect2u16{
        //         Vec2u16{20,20},
        //         // Vec2u16{14,60},
        //         Vec2u16{12,60},
        //     };
        #endif 

        #if 0

        // char str[2] = {
        //     static_cast<char>('0' + (clock::millis().count() / 200) % 16), '\0'};

        // auto shape = LineText<void, MonoFont16x8>{

        // FixedString<32> str;
        FixedStringStream<32> ss;
        qmc.update().examine();
        ss.print(qmc.read_mag().examine());
        ss.flush();
        DEBUG_PRINTLN(ss.c_str());
        auto shape = LineText<void, MonoFont8x5>{
            .left_top = {20,20},
            .spacing = 2,
            // .str = "A",
            // .str = "B",
            // .str = str,
            // .str = "明白了您只需要编码值而不是以下是修复后的代码",
            .str = "0123456789abcdef",
            // .str = "a",
            // .str = ss.c_str(),
            // .str = "(0.001, 0.040, -0.367)",
            // .str = "widget",
            .font = en_font
        };
        #endif


        #if 0
        auto shape =  Circle2<uint16_t>{
            Vec2u16{uint16_t(160 + 80 * sinpu(ctime * 0.2_r)), 80}, 6};
        #endif

        #if 0
        // auto shape = HorizonOval2<uint16_t>{
        //     .left_center = Vec2u16{uint16_t(130 + 80 * sinpu(ctime * 0.2_r)), 80}, 
        //     .radius = 20, 
        //     .length = 80
        // };
        auto shape = HorizonOval2<uint16_t>::from_bounding_box(
            // Rect2u16{Vec2u16{0,0}, Vec2u16{160, 40}}
            // Rect2u16{Vec2u16{0,0},tft.bounding_box().size/2}
            tft.bounding_box()
        ).unwrap();
        #endif
        
        #if 0
        auto shape =  RoundedRect2<uint16_t>{
            .bounding_rect = Rect2u{Vec2u16{uint16_t(115 + 80 * sinpu(ctime * 0.2_r)), 
                80}, Vec2u16{90, 30}}, 
            .radius = 8
        };
        #endif
        
        #if 0
        auto shape = HorizonSpectrum<uint16_t, q16>{
            .top_left = {20, 20},
            .cell_size = {8, 140},
            .spacing = 3,
            .samples = std::span(samples),
            .sample_range = {-1, 1}
        };
        #endif

        #if 0
        auto shape = RoundedRect2<uint16_t>{
            .bounding_rect = tft.bounding_box().shrink(0).unwrap(), 
            .radius = 40
        };
        #endif

        #if 0
        auto shape = RoundedRect2<uint16_t>{.bounding_rect = GridMap2<uint16_t>{
        // auto shape = GridMap2<uint16_t>{
            .top_left_cell = Rect2<uint16_t>::from_xywh(shape_x, shape_y, 15, 15),
            .padding = {2,2},
            .count = {15,7}
        // };
        }.bounding_box(), .radius = 15};
        // }.bounding_box();

        #endif

        // PANIC{shape, shape.bounding_box()};

        #if 1
        auto shape = Triangle2<uint16_t>{
            .points = {
                // Vec2u16{85,85} + Vec2u16::from_x_axis(50).rotated(dest_angle),
                // Vec2u16{85,85} + Vec2u16::from_x_axis(50).rotated(dest_angle + 120_deg),
                // Vec2u16{85,85} + Vec2u16::from_x_axis(50).rotated(dest_angle + 240_deg),

                // Vec2u16{85,85} + Vec2u16::from_x_axis(70).rotated(dest_angle),
                // Vec2u16{85,85} + Vec2u16::from_x_axis(70).rotated(dest_angle + 120_deg),
                // Vec2u16{85,85} + Vec2u16::from_x_axis(70).rotated(dest_angle + 240_deg),

                Vec2u16{85,85} + Vec2u16::from_ones(50).rotated(dest_angle),
                Vec2u16{85,85} + Vec2u16::from_ones(50).rotated(dest_angle + 120_deg),
                Vec2u16{85,85} + Vec2u16::from_ones(50).rotated(dest_angle + 240_deg)
            }
        }.to_sorted_by_y();
        #endif

        #if 0
        const auto shape = AnnularSector<uint16_t, q16>{
            .center = {uint16_t(160 + 79.2_r * sinpu(ctime * 0.2_r)), 80},
            .radius_range = {8, 12},
            // .radius_range = {10, 14},
            // .radius_range = {47, 53},
            // .radius_range = {40, 60},
            .angle_range = {
                Angle<q16>::from_degrees(60 + ctime * 120), 
                // Angle<q16>::from_degrees(123)
                Angle<q16>::from_degrees(LERP(
                    50, 310, 
                    sinpu(ctime * 0.4_r) * 0.5_q16 + 0.5_q16
                ))
            }
        };

        // const auto shape = AnnularSector<uint16_t, q16>{
        //     .center = {160, 160},
        //     // .radius_range = {8, 12},
        //     // .radius_range = {10, 14},
        //     // .radius_range = {47, 53},
        //     .radius_range = {130, 150},
        //     .angle_range = {
        //         Angle<q16>::from_degrees(0), 
        //         // Angle<q16>::from_degrees(123)
        //         Angle<q16>::from_degrees(180)
        //     }
        // };

        #endif

        #if 0
        const auto shape = Sector<uint16_t, q16>{
            .center = {uint16_t(160 + 80 * sinpu(ctime * 0.2_r)), 80},
            // .radius = 23,
            .radius = 12,
            // .radius = 53,
            .angle_range = {
                Angle<q16>::from_degrees(60 + ctime * 120), 
                // Angle<q16>::from_degrees(123)
                Angle<q16>::from_degrees(LERP(50, 310, sinpu(ctime * 0.4_r) * 0.5_q16 + 0.5_q16))
            }
        };

        #endif
        // using Shape = decltype(shape);
        auto shape_bb = shape.bounding_box();
        // const auto shape_bb = sector_to_bb(shape);
        // ASSERT(shape_bb.y_range().stop == 140, shape_bb);
        // DEBUG_PRINTLN(shape_bb);

        // PANIC{shape_bb};
        // Option<DrawDispatchIterator<Rect2u16>> render_iter = None;
        auto render_iter = make_draw_dispatch_iterator(shape);
        
        // PANIC{render_iter};

        Microseconds upload_us = 0us;
        Microseconds render_us = 0us;
        Microseconds clear_us = 0us;
        const auto total_us = measure_total_elapsed_us([&]{
            for(size_t i = 0; i < LCD_HEIGHT; i++){
                
                auto line_span = LineBufferSpan<RGB565>(std::span(render_buffer), i);
                auto guard = make_scope_guard([&]{
                    clear_us += measure_total_elapsed_us([&]{
                        line_span.fill(color_cast<RGB565>(ColorEnum::BLACK)).examine();
                    });
                });

                render_us += measure_total_elapsed_us([&]{
                    if(not shape_bb.has_y(i)) return;

                    if(i == shape_bb.y()){
                        render_iter = make_draw_dispatch_iterator(shape);
                    }

                    if(render_iter.has_next()){
                        // for(size_t j = 0; j < 2000; j++){
                        // for(size_t j = 0; j < 200; j++){
                        // for(size_t j = 0; j < 30; j++){
                        // for(size_t j = 0; j < 10; j++){
                        for(size_t j = 0; j < 1; j++){

                            // const auto color = color_cast<RGB565>(
                            //     render_iter.is_mid_at_right() ? ColorEnum::PINK : ColorEnum::BLUE);
                            static constexpr auto color = color_cast<RGB565>(ColorEnum::PINK);

                            render_iter.draw_filled(line_span, color).examine();
                            // render_iter.draw_hollow(line_span, RGB565::BLUE).examine();
                        }

                        // DEBUG_PRINTLN(
                        // render_iter.draw_hollow(line_span, RGB565::BRRED).examine();

                        render_iter.forward();
                    }{

                    }
                });

                upload_us += measure_total_elapsed_us([&]{
                    plot_rgb(line_span.iter(), line_span.to_scanline());
                });
            }
        });

        
        if(0) DEBUG_PRINTLN(
            
            // ,shape.points
            // ,render_iter.is_mid_at_right()
            // clear_us.count(), 
            // upload_us.count(), 
            // total_us.count(),
            // shape_bb
            
            // clock::micros().count()

            // render_iter
            // shape_bb
        );

    }

};

#if 0
[[maybe_unused]] static void static_test(){
    {
        static constexpr auto len = []{
            std::array<RGB565, LCD_WIDTH * LCD_HEIGHT> render_buffer;

            auto frame_span = FrameBufferSpan<RGB565>::from_ptr_and_size(
                render_buffer.data(), {LCD_WIDTH, LCD_HEIGHT}).unwrap();

            auto iter = frame_span.iter();

            return count_iter(iter);
        }();

        static_assert(len == LCD_HEIGHT);
    }
}

#endif