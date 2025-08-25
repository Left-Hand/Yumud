

#include "src/testbench/tb.h"

#include "robots/rpc/rpc.hpp"
#include "robots/repl/repl_service.hpp"
#include "robots/mock/mock_burshed_motor.hpp"

#include "hal/gpio/gpio_port.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/timer/timer.hpp"
#include "hal/analog/adc/adcs/adc1.hpp"
#include "hal/bus/uart/uartsw.hpp"

#include "types/image/painter/painter.hpp"
#include "types/colors/rgb/rgb.hpp"
#include "types/regions/rect2.hpp"
#include "core/utils/stdrange.hpp"
#include "core/utils/data_iter.hpp"

#include "drivers/Display/Polychrome/ST7789/st7789.hpp"


#include "core/clock/time.hpp"


#include "hal/gpio/gpio.hpp"
#include "hal/bus/spi/spihw.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/bus/i2c/i2csw.hpp"

#include "types/vectors/quat.hpp"
#include "types/image/image.hpp"
#include "types/image/font/font.hpp"

#include "types/shapes/bresenham_iter.hpp"

#include "types/shapes/rotated_rect.hpp"
#include "types/shapes/box_rect.hpp"

#include "types/regions/Segment2.hpp"
#include "types/shapes/triangle2.hpp"  
#include "types/shapes/line_iter.hpp"

namespace ymd{
template<typename T>
struct is_placed_t<Segment2<T>>:std::true_type{};
}


using namespace ymd;

struct Infallible{};

OutputStream & operator << (OutputStream & os, const Infallible){
    return os << "Infallible";
}

template<typename Color>
struct LineBufferSpan{
public:
    using Error = Infallible;

    constexpr explicit LineBufferSpan(
        const std::span<Color> buf,
        size_t y
    ):
        buf_(buf), y_(y){;}

    constexpr std::span<const Color> iter() const {
        return std::span<const Color>(buf_);
    }

    constexpr std::span<Color> iter(){
        return std::span<Color>(buf_);
    }

    constexpr auto data() const {return buf_.data();}

    constexpr auto data() {return buf_.data();}

    constexpr auto begin() const {return buf_.begin();}

    constexpr auto begin() {return buf_.begin();}

    constexpr auto end() const {return buf_.end();}

    constexpr auto end() {return buf_.end();}

    constexpr size_t size() const {return buf_.size();}

    constexpr Color operator[](const size_t index) const {return buf_[index];}

    constexpr Color & operator[](const size_t index) {return buf_[index];}

    constexpr Rect2u16 bounding_box() const {return Rect2u16(0, y_, buf_.size(), 1);}

    constexpr ScanLine to_scanline() const {
        return ScanLine{
            .x_range = {0, static_cast<uint16_t>(buf_.size())}, 
            .y = static_cast<uint16_t>(y_)
        };
    }

    template<typename PixelsIter>
    constexpr Result<void, Error> draw_iter(PixelsIter && iter){
        for(const auto pixel : StdRange(iter)){
            const auto [x, y] = pixel.position;
            if(y != y_) continue;
            buf_[x] = static_cast<Color>(pixel.color);
        }

        return Ok();
    };

    template<typename DestColor>
    constexpr Result<void, Error> fill_scanline(
        const ScanLine line,
        const DestColor color
    ){
        if(likely(line.y != y_)) return Ok();
        return fill_x_range(line.x_range, color);
    }

    template<typename DestColor>
    constexpr Result<void, Error> fill_x_range(
        const Range2<uint16_t> x_range,
        const DestColor color
    ){
        if(x_range.stop > buf_.size()) return Ok();

        const auto dest_x = MIN(buf_.size(), x_range.stop);

        for(size_t x = x_range.start; x < dest_x; ++x){
            buf_[x] = static_cast<Color>(color);
        }
        return Ok();
    }

    template<typename ColorsIter>
    constexpr Result<void, Error> fill_contiguous(
        const Rect2u16 area,
        ColorsIter && iter
    ){
        return Ok();
    }

    template<typename DestColor>
    constexpr Result<void, Error> fill_solid(
        const Rect2u16 area,
        const DestColor color
    ){
        if(not area.has_y(y_)) return Ok();
        if(area.x() > buf_.size()) return Ok();

        const auto dest_x = MIN(buf_.size(), area.x() + area.w());

        for(size_t x = area.x(); x < dest_x; ++x){
            buf_[x] = static_cast<Color>(color);
        }
        return Ok();
    }

    template<typename DestColor>
    constexpr Result<void, Error> draw_x(
        const size_t x,
        const DestColor color
    ){
        if(x >= buf_.size()) return Ok();
        buf_[x] = static_cast<Color>(color);
        return Ok();
    }



    template<typename DestColor>
    constexpr Result<void, Error> fill(
        const DestColor color
    ){ 
        for(size_t i = 0; i < buf_.size(); ++i){
            buf_[i] = static_cast<Color>(color);
        }
        return Ok();
    }

private:
    std::span<Color> buf_;
    size_t y_;
};

OutputStream & operator << (OutputStream & os, const LineBufferSpan<Binary> & line){
    const size_t size = line.size();

    static constexpr char BLACK_CHAR = '_';
    static constexpr char WHITE_CHAR = '#';

    for(size_t i = 0; i < size; ++i){
        os << (line[i].is_white() ? WHITE_CHAR : BLACK_CHAR);
    }

    return os;
}


template<typename Color>
struct FrameBufferSpan{
    using Error = Infallible;

    static constexpr Option<FrameBufferSpan> from_ptr_and_size(
        Color * ptr, Vec2u size
    ){
        if(ptr == nullptr) return None;
        FrameBufferSpan ret;

        ret.buf_ = std::span<Color>(ptr, size.x * size.y);
        ret.size_ = size;

        return Some(ret);
    }

    static constexpr Option<FrameBufferSpan> from_slice_and_width(
        std::span<Color> slice, size_t width
    ){ 
        if(slice.size() % width != 0) return None;
        const size_t height = slice.size() / width;

        FrameBufferSpan ret;

        ret.buf_ = slice;
        ret.size_ = Vec2u{width, height};

        return Some(ret);
    }

    constexpr auto data() const {
        return buf_.data();
    }

    constexpr auto data() {
        return buf_.data();
    }

    constexpr auto iter() {
        return ToLineSpanIter(buf_.data(), {0, size_.y}, size_.x);
    }

    // Add a new iter method for partial iteration
    constexpr auto iter(Range2u y_range) {

        return ToLineSpanIter(buf_.data(), y_range, size_.x);
    }

    constexpr LineBufferSpan<Color> operator [](const size_t y){
        const auto pdata = buf_.data();
        const auto width = size_.x;
        return LineBufferSpan<Color>(std::span<Color>(pdata + y * width, width), y);
    }

    constexpr Rect2u16 bounding_box() const {
        return Rect2u16::from_size(size_);
    }

    template<typename PixelsIter>
    constexpr Result<void, Error> draw_iter(PixelsIter && iter){
        auto & self = *this;
        while(true){
            if(iter.has_next() == false) return Ok();
            const auto pixel = iter.next();
            const auto [x, y] = pixel.position;
            const auto offset = x * self.size_.x + y;
            if(offset >= buf_.size()) continue;
            buf_[offset] = static_cast<Color>(pixel.color);
        }
        return Ok();
    };

    template<typename ColorsIter>
    constexpr Result<void, Error> fill_contiguous(
        const Rect2u16 area,
        ColorsIter && iter
    ){
        auto & self = *this;
        const auto x_range = area.get_x_range();
        const auto y_range = area.get_y_range();

        for(size_t y = y_range.start; y < y_range.stop; y++){
            const auto offset_base = y * self.size_.x;
            for(
                size_t x = x_range.start; 
                x < x_range.stop; 
                x++
            ){
                const size_t offset = offset_base + x;
                
                // Check if iterator has more elements
                if (!iter.has_next()) {
                    return Ok(); // or return an error if preferred
                }
                
                // Get next color start iterator
                auto color = iter.next();
                buf_[offset] = static_cast<Color>(color);
            }
        }
        return Ok();
    }

    template<typename DestColor>
    constexpr Result<void, Error> fill_solid(
        const Rect2u16 area,
        const DestColor color
    ){
        auto & self = *this;
        const auto x_range = area.get_x_range();
        const auto y_range = area.get_y_range();

        for(size_t y = y_range.start; y < y_range.stop; y++){
            const auto offset_base = y * self.size_.x;
            for(
                size_t offset = offset_base + x_range.start; 
                offset < offset_base + x_range.stop; 
                offset++
            ){
                buf_[offset] = static_cast<Color>(color);
            }
        }
        return Ok();
    }
private:
    std::span<Color> buf_;
    Vec2u size_;

    struct ToLineSpanIter{
        constexpr ToLineSpanIter(Color * pbuf, Range2u y_range, size_t width):
            pbuf_(pbuf),
            y_(y_range.start),
            y_stop_(y_range.stop),
            width_(width)
        {}

        constexpr LineBufferSpan<Color> next(){
            const auto offset = y_ * width_;
            const auto ret = LineBufferSpan<Color>(
                std::span<Color>(pbuf_ + offset, pbuf_ + offset + width_),
                y_
            );

            ++y_;

            return ret;
        }

        constexpr bool has_next() const{
            return (y_ < y_stop_);
        }

    private:
        Color * pbuf_;
        size_t y_;
        size_t y_stop_;
        size_t width_;
    };

};


template<typename T, typename Iter = LineBufferSpan<T>::ToLineSpanIter>
requires requires(OutputStream& os, const LineBufferSpan<T>& line_span) {
    os << line_span;
}
OutputStream & operator << (OutputStream & os, Iter && iter){
    for(auto line_span : StdRange(iter)){
        os.println(line_span);
    }
    return os;
}


template<typename T>
OutputStream & operator << (OutputStream & os, FrameBufferSpan<T> & frame_span){
    return os << frame_span.iter();
}



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
struct RoundedRect2{
    Rect2<T> rect;
    T radius;
};


template<typename T>
constexpr bool is_points_clockwise(const std::span<const Vec2<T>> points) {
    if (points.size() < 3) return false;
    
    T cross_sum = T{0};
    for (size_t i = 0; i < points.size(); ++i) {
        const size_t j = (i + 1) % points.size();
        cross_sum += (points[j].x - points[i].x) * (points[j].y + points[i].y);
        // 或者如果 Vec2<T> 有 cross() 方法：cross_sum += points[i].cross(points[j]);
    }
    
    // 正和：逆时针，负和：顺时针
    return cross_sum > 0;  // 修正符号
}



template<typename T>
requires (std::is_integral_v<T>)
struct DrawDispatchIterator<Rect2<T>> {
    using Shape = Rect2<T>;
    using Self = DrawDispatchIterator<Shape>;

    constexpr DrawDispatchIterator(const Shape& shape) : 
        x_range_(shape.get_x_range()),
        y_range_(shape.get_y_range()),
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


template<std::integral T>
struct DrawDispatchIterator<HorizonOval2<T>> {
    using Shape = HorizonOval2<T>;
    using Iterator = CircleBresenhamIterator<T>;

    constexpr DrawDispatchIterator(const Shape & shape)
        : iter_(Circle2<T>{.center = shape.left_center, .radius = shape.radius}),
            length_(shape.length){}

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
        if(auto res = target.fill_x_range({x_range.start, x_range.stop + length_}, color);
            res.is_err()) return res;
        
        return Ok();
    }

    template<DrawTargetConcept Target, typename Color>
    Result<void, typename Target::Error> draw_hollow(Target& target, const Color& color) {
        // 绘制当前行的范围
        if(iter_.is_y_at_edge()){
            const auto x_range = iter_.x_range();
            if(const auto res = target.fill_x_range({x_range.start, x_range.stop + length_}, color);
                res.is_err()) return res;
            return Ok();
        }else{
            const auto [left_x_range, right_x_range] = iter_.left_and_right();
            if(const auto res = target.fill_x_range(left_x_range, color);
                res.is_err()) return res;
            if(const auto res = target.fill_x_range(right_x_range.shift(length_), color);
                res.is_err()) return res;
            return Ok();
        }

    }


private:
    Iterator iter_;
    T length_;
};

// DrawDispatchIterator 特化
template<std::integral T>
struct DrawDispatchIterator<Triangle2<T>> {
    using Triangle = Triangle2<T>;
    using Iterator = TriangleIterator<T>;

    constexpr DrawDispatchIterator(const Triangle & triangle):
        iter_(triangle.to_sorted_by_y()){;}

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
        const auto x_range = iter_.current_filled();
        auto res = target.fill_x_range(x_range, color);
        if (res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    // 空心绘制（对于线段来说和填充一样）
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
    TriangleIterator<T> iter_;
};

void render_main(){


    auto init_debugger = []{
        auto & DBG_UART = DEBUGGER_INST;

        DBG_UART.init({
            .baudrate = UART_BAUD
        });

        DEBUGGER.retarget(&DBG_UART);
        DEBUGGER.set_eps(4);
        DEBUGGER.set_splitter(",");
        DEBUGGER.no_brackets();
    };



    init_debugger();

    
    auto & spi = hal::spi2;

    spi.init({144_MHz});
    
    auto & lcd_blk = hal::portD[0];
    lcd_blk.outpp(HIGH);

    auto & lcd_dc = hal::portD[7];
    auto & dev_rst = hal::portB[7];

    const auto spi_fd = spi.allocate_cs_gpio(&hal::PD<4>()).unwrap();

    drivers::ST7789 tft{
        drivers::ST7789_Phy{&spi, spi_fd, &lcd_dc, &dev_rst}, 
        {LCD_WIDTH, LCD_HEIGHT}
    };

    tft.init().examine();
    drivers::st7789_preset::init(tft, drivers::st7789_preset::_320X170{}).examine();


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



    while(true){
        const auto ctime = clock::time();
        // [[maybe_unused]] const auto [s,c] = sincospu(ctime * 0.3_r);
        [[maybe_unused]] const auto [s,c] = sincospu(ctime);
        const auto [shape_x,shape_y] = std::make_tuple(uint16_t(50 + 20 * c), uint16_t(80 + 20 * s));


        // const auto shapes = std::to_array<Rect2u16>({
        //     Rect2u16{shape_x,shape_y,5,5},
        //     Rect2u16{shape_x,uint16_t(shape_y+ 10),5,5},
        //     Rect2u16{uint16_t(shape_x + 10),shape_y,5,5},
        //     Rect2u16{shape_x,uint16_t(shape_y- 10),5,5},
        //     Rect2u16{uint16_t(shape_x - 10),shape_y,5,5}
        // });
        using Vec2u16 = Vec2<uint16_t>;
        // auto shape =  Rect2u16{shape_x,shape_y,20,20};
        // auto shape =  Segment2<uint16_t>{Vec2u16{shape_x,shape_y},Vec2u16{50,80}};
        // auto shape =  Circle2<uint16_t>{Vec2u16{shape_x,shape_y},30};
        auto shape =  HorizonOval2<uint16_t>{
            .left_center = Vec2u16{shape_x,shape_y}, .radius = 15, .length = 60};
        
        // auto shape =  Triangle2<uint16_t>{
        //     .points = {
        //         Vec2u16{shape_x,shape_y},
        //         Vec2u16{160,130},
        //         Vec2u16{20,150}
        //         // Vec2u16{static_cast<uint16_t>(shape_x + 20),static_cast<uint16_t>(shape_y + 29)},
        //         // Vec2u16{static_cast<uint16_t>(shape_x - 20),static_cast<uint16_t>(shape_y + 50)}
        //     }
        // };
        // using Shape = decltype(shape);
        auto shape_bb = shape.bounding_box();
        // Option<DrawDispatchIterator<Rect2u16>> render_iter = None;
        auto render_iter = make_draw_dispatch_iterator(shape);

        Microseconds upload_us = 0us;
        Microseconds render_us = 0us;
        const auto total_us = measure_total_elapsed_us([&]{
            for(size_t i = 0; i < LCD_HEIGHT; i++){
                
                auto line_span = LineBufferSpan<RGB565>(std::span(render_buffer), i);
                auto guard = make_scope_guard([&]{
                    line_span.fill(RGB565::BLACK).examine();
                });

                render_us += measure_total_elapsed_us([&]{
                    if(not shape_bb.has_y(i)) return;

                    if(i == shape_bb.y()){
                        render_iter = make_draw_dispatch_iterator(shape);
                        // DEBUG_PRINTLN(render_iter.y_, render_iter.y_stop_);
                        // ASSERT{i > 20, render_iter.y_, render_iter.y_range_, "why"};
                    }

                    if(render_iter.has_next()){
                        // ASSERT{i > 20, render_iter.y_, render_iter.y_range_};
                        // for(size_t j = 0; j < 20; j++){
                        for(size_t j = 0; j < 1; j++){

                            render_iter.draw_filled(line_span, RGB565::RED).examine();
                            render_iter.draw_hollow(line_span, RGB565::BLUE).examine();
                        }
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

        DEBUG_PRINTLN(render_us.count(), upload_us.count());
    }

};


void a(){



    [[maybe_unused]] auto test_render = [&]{
    
        [[maybe_unused]]const auto t = clock::time();

    };

    while(true){
        // test_fill();
        test_render();
        // test_paint();
        // test_paint();
        // qmc.update().examine();
        // painter.set_color(HSV888{0, int(100 + 100 * sinpu(clock::time())), 255});
        // painter.draw_pixel(Vec2u(0, 0));
        // painter.draw_filled_rect(Rect2u(0, 0, 20, 40)).examine();

    }

}

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