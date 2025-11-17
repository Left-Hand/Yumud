#include "src/testbench/tb.h"

#include "core/clock/time.hpp"
#include "core/utils/nth.hpp"
#include "core/utils/stdrange.hpp"
#include "core/utils/data_iter.hpp"
#include "primitive/arithmetic/rescaler.hpp"
#include "core/string/fixed_string.hpp"

#include "hal/gpio/gpio_port.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/bus/uart/uartsw.hpp"
#include "hal/gpio/gpio.hpp"
#include "hal/bus/uart/uarthw.hpp"

#include "primitive/image/painter/painter.hpp"
#include "primitive/colors/rgb/rgb.hpp"
#include "types/regions/rect2.hpp"

#include "types/vectors/quat.hpp"
#include "primitive/image/image.hpp"
#include "primitive/image/font/font.hpp"

#include "drivers/Display/Polychrome/ST7789/st7789.hpp"
#include "drivers/IMU/Axis6/MPU6050/mpu6050.hpp"
#include "drivers/IMU/Magnetometer/QMC5883L/qmc5883l.hpp"

#include "robots/rpc/rpc.hpp"
#include "robots/repl/repl_service.hpp"
#include "robots/mock/mock_burshed_motor.hpp"

#include "frame_buffer.hpp"
#include "core/string/utils/strconv2.hpp"


using namespace ymd;

// struct StringDeserializeSpawner{
//     explicit StringDeserializeSpawner(const StringView str):
//         remain_str_(str){;}
// private:
//     StringView remain_str_;
// };
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


namespace ymd::rpc{

struct ReplServer2 final{
public:
    ReplServer2(ReadCharProxy && is, WriteCharProxy && os) :
        is_(std::move(is)), 
        os_(std::move(os)){;}

    template<typename T>
    void invoke(T && obj){
        while(is_->available()){
            char chr;
            is_->read1(chr);
            if(not is_visible_char(chr)) continue;
            DEBUG_PRINTLN(chr);
        }
    }

    void set_outen(Enable outen){ outen_ = outen == EN; }   
private:
    ReadCharProxy is_;
    OutputStreamByRoute os_;
    // FixedString<32> temp_str_;

    bool outen_ = false;
    
    template<typename T>
    auto respond(T && obj, const std::span<const StringView> strs){
        const auto guard = os_.create_guard();
        if(outen_){
            os_.force_sync(EN);
            os_.prints("<<=", strs);
        }

        return [&]{
            if(!this->outen_){
                DummyOutputStream dos{};
                return rpc::visit(obj, dos, rpc::AccessProvider_ByStringViews(strs));
            }else{
                return rpc::visit(obj, os_, rpc::AccessProvider_ByStringViews(strs));
            }
        }();
    }

    [[nodiscard]] static constexpr bool is_visible_char(const char c){
        return (c >= 32) and (c <= 126);
    }
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
            .baudrate = UART_BAUD
        });

        DEBUGGER.retarget(&DBG_UART);
        DEBUGGER.set_eps(4);
        DEBUGGER.set_splitter(",");
        // DEBUGGER.no_brackets(EN);
    };



    init_debugger();


    while(true){
        [[maybe_unused]] auto repl_service_poller = [&]{
            static rpc::ReplServer2 repl_server{&DBG_UART, &DBG_UART};

            static const auto list = rpc::make_list(
                "list",

                rpc::make_function("errn", [&](int32_t a, int32_t b){ 
                    DEBUG_PRINTLN(a,b);
                }),
                rpc::make_function("errn2", [&](int32_t a, int32_t b){ 
                    DEBUG_PRINTLN(a,b);
                })

            );

            repl_server.invoke(list);
        };

        while(DBG_UART.available()){
            uint32_t chr;
            DBG_UART.read(chr);
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