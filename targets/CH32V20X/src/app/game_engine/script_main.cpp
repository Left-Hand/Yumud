#include "src/testbench/tb.h"

#include "core/clock/time.hpp"
#include "core/utils/nth.hpp"
#include "core/utils/stdrange.hpp"
#include "core/utils/data_iter.hpp"
#include "core/string/conv/strconv2.hpp"
#include "core/string/owned/heapless_string.hpp"

#include "primitive/arithmetic/rescaler.hpp"
#include "primitive/image/painter/painter.hpp"
#include "primitive/image/image.hpp"
#include "primitive/image/font/font.hpp"
#include "primitive/colors/rgb/rgb.hpp"
#include "algebra/regions/rect2.hpp"
#include "algebra/vectors/quat.hpp"

#include "hal/gpio/gpio_port.hpp"
#include "hal/conn/uart/hw_singleton.hpp"
#include "hal/gpio/gpio.hpp"
#include "hal/conn/uart/hw_singleton.hpp"


#include "drivers/Display/Polychrome/ST7789/st7789.hpp"

#include "middlewares/raster/frame_buffer.hpp"
#include "middlewares/repl/repl.hpp"
#include "middlewares/repl/repl_server.hpp"



using namespace ymd;

namespace ymd::strconv2{

struct StringSplitSeeker{
    explicit constexpr StringSplitSeeker(const char delimiter):
        delimiter_(delimiter){}

    Option<size_t> find_intro(const StringView str){
        for(size_t i = 0; i < str.size(); i++){
            if(not is_delimiter(str[i])) return Some(i);
        }
        return None;
    }
private:
    char delimiter_;

    constexpr bool is_delimiter(const char c) const{
        return (c == delimiter_) || (c == '\0');
    }
};

template<typename T>
using DestringResult = strconv2::DestringResult<T>;


#if 0

template<typename T>
struct StringEntitySeeker{

    template<typename SpBeginner, typename SpTerminator>
    static constexpr DestringResult<std::tuple<size_t, size_t>> match(
        const StringView str,
        const SpBeginner && beginner,
        const SpBeginner && terminator
    ){
        const size_t left = ({
            const auto may_left = beginner.find_intro(str);
            if(may_left.is_none()) return Err(DestringError::BeginnerNotFounded);
            may_left.unwrap();
        });

        const size_t right = ({
            const auto may_right = terminator.find_intro(str);
            if(may_right.is_none()) return Err(DestringError::TerminatorNotFounded);
            may_right.unwrap();
        });

        return Ok(std::make_tuple(left, right));
    }
};

template<>
struct StringEntitySeeker<StringView>{
    template<typename SpBeginner, typename SpTerminator>
    static constexpr DestringResult<std::tuple<size_t, size_t>> match(
        const StringView str,
        const SpBeginner && beginner,
        const SpBeginner && terminator
    ){
        const size_t left = ({
            const auto may_left = beginner.find_intro(str);
            if(may_left.is_none()) return Err(DestringError::BeginnerNotFounded);
            may_left.unwrap();
        });

        const size_t right = ({
            const auto may_right = terminator.find_intro(str);
            if(may_right.is_none()) return Err(DestringError::TerminatorNotFounded);
            may_right.unwrap();
        });

        return Ok(std::make_tuple(left, right));
    }
};

#endif

// struct StringDeformatIterator{
//     template<typename T>
//     using DestringResult = strconv2::DestringResult;

//     explicit constexpr StringDeformatIterator(StringView str):str_(str){}

//     template<typename T, typename SpSeeker>
//     constexpr DestringResult<T> next(SpSeeker && sperator_Seeker){
//     }
// private:
//     StringView str_;
// };



template<typename SpBeginner, typename SpTerminator>
struct StringEntitySpawner{
    explicit constexpr StringEntitySpawner(
        const SpBeginner && beginner,
        const SpBeginner && terminator
    ):
        beginner_(std::move(beginner)),
        terminator_(std::move(terminator)){}


    // 输入待序列化对象的可变引用, 以及包含待序列化的对象及后续子句字符串
    // 返回后续子句字符串或错误
    template<typename T>
    DestringResult<StringView> spawn(T & obj, const StringView str){

        return Ok(str);
    }
private:
    const SpBeginner beginner_;
    const SpTerminator terminator_;
};
}


// pub enum JsonVariant {
//     Object(Vec<(String, JsonVariant)>),
//     Array(Vec<JsonVariant>),
//     String(String),
//     Number(f64),
//     Bool(bool),
//     Null,
// }

struct JsonVariant{
    enum class Kind{
        Object,
        Array,
        String,
        Number,
        Bool,
        Null,
    };
    constexpr JsonVariant(const Kind kind,const StringView content):
        kind_(kind), content_(content){}
private:
    Kind kind_;
    StringView content_;
};

void script_main(){

    auto & DBG_UART = DEBUGGER_INST;
    static constexpr uint32_t UART_BAUD = 576000;

    auto init_debugger = []{

        DBG_UART.init({
            .remap = hal::USART2_REMAP_PA2_PA3,
            .baudrate = hal::NearestFreq(UART_BAUD),
        });

        DEBUGGER.retarget(&DBG_UART);

        DEBUGGER.build_config()
            .set_eps(4)
            .set_splitter(",")
            .no_brackets(EN)
            .no_fieldname(EN)
            .force_sync(EN)
            .finalize();
        // DEBUGGER.no_brackets(EN);
    };



    init_debugger();


    while(true){
        [[maybe_unused]] auto repl_service_poller = [&]{
            static repl::ReplServer repl_server{&DBG_UART, &DBG_UART};

            static const auto list = script::make_list(
                "list",

                script::make_function("errn", [&](int32_t a, int32_t b){
                    DEBUG_PRINTLN(a,b);
                }),
                script::make_function("errn2", [&](int32_t a, int32_t b){
                    DEBUG_PRINTLN(a,b);
                })

            );

            repl_server.invoke(list);
        };

        while(DBG_UART.available()){
            uint8_t chr;
            const auto len = DBG_UART.try_read_byte(chr);
            if(len == 0) break;
            DEBUG_PRINTLN(static_cast<char>(chr));
        }
        if(0) DEBUG_PRINTLN(
            // DBG_UART.available(),
            // DBG_UART.rx_dma_buf_index_,
            // DBG_UART.rx_fifo().write_idx(),
            // DBG_UART.rx_fifo().read_idx(),
            // strconv2::defmt_str<bool>("1")

            // strconv2::defmt_str<uint8_t>("256")

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

        repl_service_poller();
    }
}
