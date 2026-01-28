

#include "src/testbench/tb.h"

#include "core/clock/time.hpp"
#include "core/utils/nth.hpp"
#include "core/utils/stdrange.hpp"
#include "core/utils/data_iter.hpp"
#include "core/string/conv/strconv2.hpp"

#include "primitive/arithmetic/rescaler.hpp"
#include "primitive/image/painter/painter.hpp"
#include "primitive/image/image.hpp"
#include "primitive/image/font/font.hpp"
#include "primitive/colors/rgb/rgb.hpp"

#include "middlewares/repl/repl.hpp"
#include "middlewares/repl/repl_server.hpp"


#include "algebra/regions/rect2.hpp"
#include "algebra/vectors/quat.hpp"
#include "algebra/shapes/bresenham_iter.hpp"
#include "algebra/shapes/rotated_rect.hpp"
#include "algebra/shapes/box_rect.hpp"
#include "algebra/regions/Segment2.hpp"
#include "algebra/shapes/triangle2.hpp"  
#include "algebra/shapes/line_iter.hpp"
#include "algebra/shapes/annular_sector.hpp"
#include "algebra/shapes/horizon_spectrum.hpp"
#include "algebra/shapes/triangle2.hpp"
#include "algebra/shapes/oval2.hpp"
#include "algebra/shapes/gridmap2.hpp"
#include "algebra/shapes/rounded_rect2.hpp"

#include "hal/gpio/gpio_port.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/timer/timer.hpp"
#include "hal/analog/adc/hw_singleton.hpp"
#include "hal/bus/uart/uartsw.hpp"
#include "hal/gpio/gpio.hpp"
#include "hal/bus/spi/spihw.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/bus/i2c/i2csw.hpp"


#include "drivers/Display/Polychrome/ST7789/st7789.hpp"
// #include "drivers/IMU/Axis6/MPU6050/mpu6050.hpp"
// #include "drivers/IMU/Magnetometer/QMC5883L/qmc5883l.hpp"

#include "frame_buffer.hpp"



namespace ymd{
template<typename T>
struct is_placed_t<Segment2<T>>:std::true_type{};




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

    constexpr void seek_next() {
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
    constexpr void seek_next() {
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
    constexpr void seek_next() {
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
    using Transformer = Rescaler<iq16>;
    constexpr DrawDispatchIterator(const Shape & shape)
        : shape_(shape),
            transformer_(Transformer::from_input_and_output(
                shape_.sample_range,
                shape_.bounding_box().y_range().swap()
            )),
            y_(shape.top_left.y)
        {}

    // 检查是否还有下一行
    constexpr bool has_next() const {
        return y_ < shape_.top_left.y + shape_.cell_size.y;
    }

    // 推进到下一行
    constexpr void seek_next() {
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

            if(row_pixels == 0) [[unlikely]] continue;

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

    constexpr void seek_next() {
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


struct DemoShapeFactory{
    uq16 now_secs;
    Rect2u tft_bounding_box;
    
    // Font en_font_;
    // Font ch_font_;

    auto make_segment2() const {
        auto shape = Segment2<uint16_t>{
            Vec2u16{
                uint16_t(50 + 20 * iq16(math::cospu(now_secs * 0.2_r))), 
                uint16_t(80 + 20 * iq16(math::sinpu(now_secs * 0.2_r)))},
            Vec2u16{50,80}
        };
        return shape;
    }

    auto make_vertical_oval2() const {
        auto shape = VerticalOval2<uint16_t>::from_bounding_box(
            Rect2u16{
                Vec2u16{0,0},
                Vec2u16{120,170},
            }
        ).unwrap();
        return shape;
    }

    auto make_rect2() const {
        auto shape = Rect2u16{
                Vec2u16{20,20},
                Vec2u16{12,60},
        };
        return shape;
    }


    template<typename Font>
    auto make_line_text(const Font & font) const {
        auto shape = LineText<void, Font>{
            .left_top = {20,20},
            .spacing = 2,
            // .str = "0123456789ABCDEF",
            .str = "0123456789abcd",
            .font = font
        };
        return shape;
    }

    auto make_circle2() const {
        auto shape = Circle2<uint16_t>{
            Vec2u16{uint16_t(160 + 80 * iq16(math::sinpu(now_secs * 0.2_r))), 80}, 6};
        return shape;
    }

    auto make_horizon_oval2() const {
        auto shape = HorizonOval2<uint16_t>::try_from_bounding_box(
            tft_bounding_box
        ).unwrap();
        return shape;
    }

    auto make_rounded_rect2_moving() const {
        auto shape = RoundedRect2<uint16_t>{
            .bounding_rect = Rect2u{
                Vec2u16{uint16_t(115 + 80 * iq16(math::sinpu(now_secs * 0.2_r))), 80}, 
                Vec2u16{90, 30}
            }, 
            .radius = 8
        };
        return shape;
    }

    auto make_horizon_spectrum(std::span<const iq16> samples) const {
        auto shape = HorizonSpectrum<uint16_t, iq16>{
            .top_left = {20, 20},
            .cell_size = {8, 140},
            .spacing = 3,
            .samples = samples,
            .sample_range = {-1, 1}
        };
        return shape;
    }

    auto make_full_screen_rounded_rect() const {
        auto shape = RoundedRect2<uint16_t>{
            .bounding_rect = tft_bounding_box.shrink(0).unwrap(), 
            .radius = 40
        };
        return shape;
    }

    auto make_grid_map(uint16_t shape_x, uint16_t shape_y) const {
        return GridMap2<uint16_t>{
            .top_left_cell = Rect2<uint16_t>::from_xywh(shape_x, shape_y, 15, 15),
            .padding = {2,2},
            .count = {15,7}
            // .count = {2,2}
        };
    }

    auto make_triangle2(Angular<iq16> dest_angle) const {
        auto shape = Triangle2<iq16>{
            .points = {
                Vec2<iq16>{185,85} + Vec2<iq16>::from_ones(50).rotated(dest_angle),
                Vec2<iq16>{185,85} + Vec2<iq16>::from_ones(50).rotated(dest_angle + 120_deg),
                Vec2<iq16>{185,85} + Vec2<iq16>::from_ones(50).rotated(dest_angle + 240_deg)
            }
        }.to_sorted_by_y();
        
        const auto shape_pixeded = Triangle2<uint16_t>{
            .points = {
                Vec2<uint16_t>(shape.points[0]),
                Vec2<uint16_t>(shape.points[1]),
                Vec2<uint16_t>(shape.points[2])
            }
        };

        return shape_pixeded;
    }

    auto make_annular_sector() const {
        const auto shape = AnnularSector<uint16_t, iq16>{
            .center = {uint16_t(160 + 79.2_r * iq16(math::sinpu(now_secs * 0.2_r))), 80},
            .radius_range = {8, 12},
            .angle_range = AngularRange<iq16>::from_start_and_interval(
                Angular<iq16>::from_degrees(60 + now_secs * 120), 
                Angular<iq16>::from_degrees(LERP(
                    50, 310, 
                    iq16(math::sinpu(now_secs * 0.4_r)) * 0.5_iq16 + 0.5_iq16
                ))
            )
        };
        return shape;
    }

    // auto make_sector() const {
    //     const auto shape = Sector<uint16_t, iq16>{
    //         .center = {uint16_t(160 + 80 * math::sinpu(now_secs * 0.2_r)), 80},
    //         .radius = 12,
    //         .angle_range = {
    //             Angular<iq16>::from_degrees(60 + now_secs * 120), 
    //             Angular<iq16>::from_degrees(LERP(50, 310, math::sinpu(now_secs * 0.4_r) * 0.5_iq16 + 0.5_iq16))
    //         }
    //     };
    //     return shape;
    // }

};
}

