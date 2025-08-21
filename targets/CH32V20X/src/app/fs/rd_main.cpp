#include "types/shapes/bresenham_iter.hpp"


#include "src/testbench/tb.h"
#include "dsp/controller/adrc/leso.hpp"
#include "dsp/controller/adrc/command_shaper.hpp"


#include "robots/rpc/rpc.hpp"
#include "robots/repl/repl_service.hpp"
#include "robots/mock/mock_burshed_motor.hpp"

#include "hal/gpio/gpio_port.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/timer/timer.hpp"
#include "hal/adc/adcs/adc1.hpp"
#include "hal/bus/uart/uartsw.hpp"

#include "types/image/painter/painter.hpp"
#include "types/colors/rgb/rgb.hpp"
#include "types/regions/rect2.hpp"
#include "core/utils/stdrange.hpp"

using namespace ymd;

template<typename Color>
struct LineSpan{
public:
    constexpr explicit LineSpan(
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

    constexpr Rect2u to_bounding_box() const {return Rect2u(0, y_, buf_.size(), 1);}
private:
    std::span<Color> buf_;
    size_t y_;
};

OutputStream & operator << (OutputStream & os, const LineSpan<Binary> & line){
    const size_t size = line.size();

    static constexpr char BLACK_CHAR = '_';
    static constexpr char WHITE_CHAR = '#';

    for(size_t i = 0; i < size; ++i){
        os << (line[i].is_white() ? WHITE_CHAR : BLACK_CHAR);
    }

    return os;
}

template<typename Color>
struct FrameSpan{
    static constexpr Option<FrameSpan> from_ptr_and_size(
        Color * ptr, Vec2u size
    ){
        if(ptr == nullptr) return None;
        FrameSpan ret;

        ret.buf_ = std::span<Color>(ptr, size.x * size.y);
        ret.size_ = size;

        return Some(ret);
    }

    static constexpr Option<FrameSpan> from_slice_and_width(
        std::span<Color> slice, size_t width
    ){ 
        if(slice.size() % width != 0) return None;
        const size_t height = slice.size() / width;

        FrameSpan ret;

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
        // auto y_start = y_range.start;
        // auto y_stop = y_range.start;

        // if (y_stop > size_.y) y_stop = size_.y;
        // if (y_start >= y_stop) y_start = y_stop;
        // return ToLineSpanIter(buf_.data(), {y_start, y_stop}, size_.x);

        return ToLineSpanIter(buf_.data(), y_range, size_.x);
    }

    constexpr LineSpan<Color> operator [](const size_t y){
        const auto pdata = buf_.data();
        const auto width = size_.x;
        return LineSpan<Color>(std::span<Color>(pdata + y * width, width), y);
    }

    constexpr Rect2u to_bounding_box() const {
        return Rect2u::from_size(size_);
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

        constexpr LineSpan<Color> next(){
            const auto offset = y_ * width_;
            const auto ret = LineSpan<Color>(
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


template<typename T, typename Iter = LineSpan<T>::ToLineSpanIter>
requires requires(OutputStream& os, const LineSpan<T>& line_span) {
    os << line_span;
}
OutputStream & operator << (OutputStream & os, Iter && iter){
    for(auto line_span : StdRange(iter)){
        os.println(line_span);
    }
    return os;
}


template<typename T>
OutputStream & operator << (OutputStream & os, FrameSpan<T> & frame_span){
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

// static constexpr auto IMG_WIDTH = 32u;
// static constexpr auto IMG_HEIGHT = 18u;

// static constexpr auto IMG_WIDTH = 8u;
// static constexpr auto IMG_HEIGHT = 6u;

static constexpr auto IMG_WIDTH = 6u;
static constexpr auto IMG_HEIGHT = 4u;

void render_main(){


    auto init_debugger = []{
        auto & DBG_UART = hal::uart2;

        DBG_UART.init({
            .baudrate = UART_BAUD
        });

        DEBUGGER.retarget(&DBG_UART);
        DEBUGGER.set_eps(4);
        DEBUGGER.set_splitter(",");
        DEBUGGER.no_brackets();
    };

    init_debugger();

    std::array<Binary, IMG_WIDTH * IMG_HEIGHT> buffer;

    auto frame_span = FrameSpan<Binary>::from_ptr_and_size(
        buffer.data(), {IMG_WIDTH, IMG_HEIGHT}).unwrap();

    auto draw = [&]{
        frame_span[1][1] = Binary::WHITE;
        frame_span[2][1] = Binary::WHITE;
        frame_span[3][3] = Binary::WHITE;
    };

    draw();

    while(true){
        // DEBUG_PRINTLN(frame_span.iter({0, 2}));
        // DEBUG_PRINTLN(count_iter(frame_span.iter({0, 2})));
        // DEBUG_PRINTLN(count_iter(frame_span.iter()));
        DEBUG_PRINTLN(frame_span.iter());
        // DEBUG_PRINTLN(frame_span);
        clock::delay(5ms);
    }
};




#if 0
[[maybe_unused]] static void static_test(){
    {
        static constexpr auto len = []{
            std::array<Binary, IMG_WIDTH * IMG_HEIGHT> buffer;

            auto frame_span = FrameSpan<Binary>::from_ptr_and_size(
                buffer.data(), {IMG_WIDTH, IMG_HEIGHT}).unwrap();

            auto iter = frame_span.iter();

            return count_iter(iter);
        }();

        static_assert(len == IMG_HEIGHT);
    }
}

#endif