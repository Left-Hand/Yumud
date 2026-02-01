
#include "core/string/view/string_view.hpp"
#include "primitive/arithmetic/rescaler.hpp"
#include "primitive/image/image.hpp"


#include "algebra/regions/rect2.hpp"
#include "algebra/vectors/quat.hpp"
#include "algebra/shapes/bresenham_iter.hpp"
#include "algebra/shapes/rotated_rect.hpp"
#include "algebra/shapes/box_rect.hpp"
#include "algebra/regions/segment2.hpp"
#include "algebra/shapes/triangle2.hpp"  
#include "algebra/shapes/line_iter.hpp"
#include "algebra/shapes/annular_sector.hpp"
#include "algebra/shapes/horizon_spectrum.hpp"
#include "algebra/shapes/triangle2.hpp"
#include "algebra/shapes/oval2.hpp"
#include "algebra/shapes/gridmap2.hpp"
#include "algebra/shapes/rounded_rect2.hpp"


#include "frame_buffer.hpp"



namespace ymd{

template<typename T>
struct [[nodiscard]] Sprite final{
    Image<T> image;
    math::Vec2u position;

    math::Rect2u bounding_box() const{
        // return image.bounding_box() + position;
        return math::Rect2u16::from_xywh(
            static_cast<uint16_t>(position.x),
            static_cast<uint16_t>(position.y),
            static_cast<uint16_t>(image.size().x),
            static_cast<uint16_t>(image.size().y)
        );
    }

    Sprite copy() {
        return Sprite<T>(image.copy(), position);
    }
};

template<typename T>
struct is_placed_t<Sprite<T>>:std::true_type{;};

template<typename T>
struct is_placed_t<math::Segment2<T>>:std::true_type{};




template<typename Encoding, typename Font>
struct LineText{
    math::Vec2u16 left_top;
    uint16_t spacing;
    StringView str;
    // MonoFont7x7 & font;
    // MonoFont7x7 font;
    Font font;

    constexpr math::Rect2u16 bounding_box() const {
        const size_t str_len = str.length();
        const uint16_t width = (str_len * font.size().x) + (str_len - 1) * spacing;
        const uint16_t height = font.size().y;

        return math::Rect2u16{left_top, {width, height}};
    };
};

template<typename Encoding, typename Font>
struct is_placed_t<LineText<Encoding, Font>>:std::true_type{;};




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

template<typename Shape>
// auto make_RenderIterator
auto make_draw_dispatch_iterator(Shape && shape){
    return RenderIterator<std::decay_t<Shape>>(std::move<Shape>(shape));
}


template<typename T>
requires (std::is_integral_v<T>)
struct RenderIterator<math::Rect2<T>> {
    using Shape = math::Rect2<T>;
    using Self = RenderIterator<Shape>;

    constexpr explicit RenderIterator(const Shape& shape) : 
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
            if (auto res = target.fill_x_range(math::Range2<T>::from_start_and_length(x_range_.start, 1), color);
                res.is_err()) return Err(res.unwrap_err());
            
            // 绘制右端点（注意：x_range_.stop 是 exclusive）
            if (auto res = target.fill_x_range(math::Range2<T>::from_start_and_length(x_range_.stop - 1, 1), color);
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
    math::Range2<T> x_range_;
    math::Range2<T> y_range_;
    T y_;
};



// RenderIterator 特化
template<std::integral T>
struct RenderIterator<math::Segment2<T>> {
    using Segment = math::Segment2<T>;
    // using Iterator = BresenhamIterator<T>;
    using Iterator = LineDDAIterator<T>;

    constexpr explicit RenderIterator(const Segment& segment)
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


// RenderIterator 特化
template<std::integral T>
struct RenderIterator<math::Circle2<T>> {
    using Shape = math::Circle2<T>;
    using Iterator = CircleBresenhamIterator<T>;

    constexpr explicit RenderIterator(const Shape & shape)
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




// RenderIterator 特化
template<typename T>
struct RenderIterator<Sprite<T>> {
    using Shape = Sprite<T>;
    constexpr explicit RenderIterator(Shape && shape)
        : shape_(shape.copy()),
            y_stop_(shape_.image.size().y + shape_.position.y),
            y_(shape_.position.y)
        {}

    // 检查是否还有下一行
    constexpr bool has_next() const {
        return y_ < y_stop_;
    }

    // 推进到下一行
    constexpr void seek_next() {
        y_++;
    }

    template<DrawTargetConcept Target>
    Result<void, typename Target::Error> draw_texture(Target & target) {
        const size_t x_start = shape_.position.x;
        const size_t width = shape_.image.size().x;

        const T * p_texture = shape_.image.head_ptr() + (y_ - shape_.position.y) * width;
        if(const auto res = target.fill_texture(ScanLine{.x_range = {x_start, x_start + width}, .y = y_}, p_texture);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

private:
    Shape shape_;
    uint16_t y_stop_;
    uint16_t y_;
};


// RenderIterator 特化
template<std::integral T, typename D>
struct RenderIterator<HorizonSpectrum<T, D>> {
    using Shape = HorizonSpectrum<T, D>;
    using Transformer = Rescaler<iq16>;
    constexpr explicit RenderIterator(const Shape & shape)
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
            const auto x_range = math::Range2u16::from_start_and_stop_unchecked(x, next_x);
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


template<typename Encoding, typename Font>
struct RenderIterator<LineText<Encoding, Font>> {
    using Shape = LineText<Encoding, Font>;
    using Self = RenderIterator<Shape>;

    constexpr explicit RenderIterator(const Shape& shape) : 
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
    math::Range2<uint16_t> x_range_;
    math::Range2<uint16_t> y_range_;
    uint16_t y_;
};


struct DemoShapeFactory{
    uq16 now_secs;
    math::Rect2u tft_bounding_box;
    
    // Font en_font_;
    // Font ch_font_;

    auto make_segment2() const {
        auto shape = math::Segment2<uint16_t>{
            math::Vec2u16{
                uint16_t(50 + 20 * iq16(math::cospu(now_secs * 0.2_r))), 
                uint16_t(80 + 20 * iq16(math::sinpu(now_secs * 0.2_r)))},
            math::Vec2u16{50,80}
        };
        return shape;
    }

    auto make_vertical_oval2() const {
        auto shape = VerticalOval2<uint16_t>::from_bounding_box(
            math::Rect2u16{
                math::Vec2u16{0,0},
                math::Vec2u16{120,170},
            }
        ).unwrap();
        return shape;
    }

    auto make_rect2() const {
        auto shape = math::Rect2u16{
                math::Vec2u16{20,20},
                math::Vec2u16{12,60},
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
        auto shape = math::Circle2<uint16_t>{
            math::Vec2u16{uint16_t(160 + 80 * iq16(math::sinpu(now_secs * 0.2_r))), 80}, 6};
        return shape;
    }

    auto make_horizon_oval2(const math::Rect2<int16_t> rect) const {
        auto shape = HorizonOval2<int16_t>::try_from_bounding_box(
            rect
        ).unwrap();
        return shape;
    }

    auto make_rounded_rect2_moving() const {
        auto shape = RoundedRect2<uint16_t>{
            .bounding_rect = math::Rect2u{
                math::Vec2u16{uint16_t(115 + 80 * iq16(math::sinpu(now_secs * 0.2_r))), 80}, 
                math::Vec2u16{90, 30}
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
            .top_left_cell = math::Rect2<uint16_t>::from_xywh(shape_x, shape_y, 15, 15),
            .padding = {2,2},
            .count = {15,7}
            // .count = {2,2}
        };
    }

    auto make_triangle2(Angular<iq16> dest_angle) const {
        auto shape = Triangle2<iq16>{
            .points = {
                math::Vec2<iq16>{185,85} + math::Vec2<iq16>::from_ones(50).rotated(dest_angle),
                math::Vec2<iq16>{185,85} + math::Vec2<iq16>::from_ones(50).rotated(dest_angle + 120_deg),
                math::Vec2<iq16>{185,85} + math::Vec2<iq16>::from_ones(50).rotated(dest_angle + 240_deg)
            }
        }.to_sorted_by_y();
        
        const auto shape_pixeded = Triangle2<uint16_t>{
            .points = {
                math::Vec2<uint16_t>(shape.points[0]),
                math::Vec2<uint16_t>(shape.points[1]),
                math::Vec2<uint16_t>(shape.points[2])
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

