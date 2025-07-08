#include "core/debug/debug.hpp"
#include "core/sync/spinlock.hpp"
#include "core/sync/barrier.hpp"
#include "core/string/fixed_string.hpp"

#include "hal/bus/uart/uarthw.hpp"
#include "hal/bus/i2c/i2csw.hpp"
#include "hal/gpio/gpio_port.hpp"

#include "src/testbench/tb.h"

#include "drivers/HID/TM1637/TM1637.hpp"
#include "drivers/HID/HT16K33/HT16K33.hpp"
#include "drivers/HID/prelude/keycode.hpp"
#include "drivers/HID/prelude/axis_input.hpp"
#include "drivers/HID/prelude/segcode.hpp"
#include "drivers/HID/prelude/button_input.hpp"

#include <atomic>
#include <barrier>


using namespace ymd;
using namespace ymd::drivers;

#define UART hal::uart2
#define SCL_GPIO hal::PB<0>()
#define SDA_GPIO hal::PB<1>()


namespace ymd::hid{

template<typename T>
class KeyBoardComponent{
};


template<typename T>
struct KeyBoardLayout{
};

template<>
struct KeyBoardLayout<HT16K33> final{
static constexpr char map_place_to_char(const uint8_t x, const uint8_t y){
    switch(y){
        case 0: switch(x){
            case 0: return 'E';
            case 1: return 'W';
            case 2: return 'Q';
            case 3: return '-';
            case 4: return '3';
            case 5: return '2';
            case 6: return '1';
            default: break;
        }

        case 1: switch(x){
            case 0: return 'D';
            case 1: return 'S';
            case 2: return 'A';
            case 3: return '.';
            case 4: return '6';
            case 5: return '5';
            case 6: return '4';
            default: break;
        }

        case 2: switch(x){
            case 0: return 'C';
            case 1: return 'X';
            case 2: return 'Z';
            case 3: return '0';
            case 4: return '9';
            case 5: return '8';
            case 6: return '7';
            default: break;
        } 
        default: break;
    }
    return 0;
}

static constexpr KeyCode map_wasd_to_arrow(const char chr){
    switch(chr){
        case 'W': return KeyCode::ArrowUp;
        case 'A': return KeyCode::ArrowLeft;
        case 'S': return KeyCode::ArrowDown;
        case 'D': return KeyCode::ArrowRight;
        default: __builtin_unreachable();
    }
}
constexpr Option<KeyCode> map(const uint8_t x, const uint8_t y) const {
    const auto chr = map_place_to_char(x,y);


    switch(chr){
        case 0: return None;
        case 'W':
        case 'A':
        case 'S':
        case 'D':
            return Some(map_wasd_to_arrow(chr));
        case 'Q':
            return Some(KeyCode::NumpadBackspace);
        case 'E':
            return Some(KeyCode::NumpadEnter);
        case 'C':
            return Some(KeyCode::NumpadClear);
        default:
            return KeyCode::from_char(chr);
    }
} 
};

template<>
class KeyBoardComponent<drivers::HT16K33>{
public:
    using Error = drivers::HT16K33::Error;

    template<typename T = void>
    using IResult = Result<T, Error>;

    KeyBoardComponent(
        HT16K33 & inst):
        inst_(inst)
    {;}

    IResult<> update(){
        if(const auto res = inst_.is_any_key_pressed();
            res.is_err()) return Err(res.unwrap_err());
        else if(res.unwrap() == false){
            input_.update(None);
            return Ok();
        }
        // const auto beg = clock::micros();

        const auto next_key_data = ({
            const auto res = inst_.get_key_data();
            if(res.is_err()) return Err(res.unwrap_err());
            res.unwrap();
        });

        // const auto beg = clock::micros();

        const auto may_keycode = [&] -> Option<KeyCode>{
            const auto may_xy = next_key_data.to_first_xy();;
            if(may_xy.is_none()) return None;
            const auto [x,y] = may_xy.unwrap();
            const auto may_code = layouter_.map(x,y);
            // DEBUG_PRINTLN("mc", may_code);
            return may_code;
        }();

        // DEBUG_PRINTLN(may_keycode.map([](const KeyCode code){
        //     return code.to_char();
        // }).flatten());

        
        input_.update(may_keycode);
        // DEBUG_PRINTLN(clock::micros() - beg);
        return Ok();
    }
    constexpr const auto & input() const noexcept { return input_; }
private:
    HT16K33 & inst_;
    KeyBoardLayout<HT16K33> layouter_;
    ButtonInput<KeyCode> input_;
};

template<typename T>
class SegDisplayerComponent{

};

template<>
class SegDisplayerComponent<HT16K33> final{
public:
    SegDisplayerComponent(HT16K33 & inst) : inst_(inst){}
private:
    HT16K33 & inst_;
};



class LineEdit final{
private:

    struct Cursor{
        // constexpr Cursor(
        constexpr Cursor(
            const uint8_t position,
            const uint8_t max_limit 
        ):
            position_(position),
            max_limit_(max_limit){;}

        [[nodiscard]] constexpr Result<Cursor, void> shift(int delta, size_t _limit = UINT32_MAX) const{
            const int next_position = int(position_) + delta;
            const int limit = int(MIN(_limit, max_limit_));
            // ASSERT(next_position >= 0);
            if(next_position < 0) return Err();
            if(next_position > limit) return Err();
            // ASSERT(next_position < limit); 
            return Ok(copy(CLAMP(next_position, 0, limit)));
        }

        [[nodiscard]] constexpr uint8_t position() const {
            return position_;
        }

        [[nodiscard]] constexpr size_t max_limit() const {
            return max_limit_;
        }

        [[nodiscard]] constexpr Cursor copy(const uint8_t position) const {
            return Cursor(position, max_limit_);
        }

        constexpr void set_position(const uint8_t position){
            position_ = position;
        }
    private:
        uint8_t position_;
        uint8_t max_limit_;


        static void static_test(){
            constexpr auto cursor = Cursor(0, 10);
            static_assert(cursor.position() == 0);
            static_assert(cursor.max_limit() == 10);
            static_assert(cursor.copy(1).position() == 1);
            static_assert(cursor.shift(1).unwrap().position() == 1);
            static_assert(cursor.shift(3).unwrap().position() == 3);

            static_assert(Cursor{1,MAX_LENGTH}.position() == 1);
        }
    };

public:
    constexpr LineEdit():
        str_(),
        cursor_(0, MAX_LENGTH){;}
    constexpr Result<void, void> handle_key_input(const KeyCode code){
        #define DEF_INSERT_CHAR(_code) {\
            const auto may_chr = _code.to_char();\
            if(may_chr.is_none()) return Err();\
            handle_insert_char(cursor_.position(), may_chr.unwrap());\
        }\

        if(code.is_digit()){
            DEF_INSERT_CHAR(code);
        }else if(code.is_alpha()){
            DEF_INSERT_CHAR(code);
        } else if(code.is_arrow()){
            handle_arrow_input(code);
        }else{
            switch(code.kind()){
                case KeyCode::NumpadComma:
                case KeyCode::NumpadSubtract:
                    DEF_INSERT_CHAR(code);
                    break;
                case KeyCode::NumpadBackspace:
                    handle_backspace();
                    break;
                case KeyCode::NumpadEnter:
                    handle_enter([](const StringView sv){
                        DEBUG_PRINTLN("entered", sv);
                    });
                    break;
                case KeyCode::NumpadClear:
                    handle_clear();
                    break;
                default:
                    DEBUG_PRINTLN("not handled", code);
            }
        }

        return Ok();
        #undef DEF_INSERT_CHAR
    }




    constexpr void handle_clear(){
        cursor_.set_position(0);
        str_.clear();
    }

    template<typename Fn>
    constexpr void handle_enter(Fn && fn){
        std::forward<Fn>(fn)(str_.as_view());
    }

    [[nodiscard]] constexpr StringView str() const{
        return str_.as_view();
    }

    [[nodiscard]] constexpr auto cursor() const{
        return cursor_;
    }



private:
    static constexpr size_t MAX_LENGTH = 7;
    FixedString<MAX_LENGTH> str_;

    Cursor cursor_;


    constexpr void handle_arrow_input(const KeyCode code){
        switch(code.kind()){
            default: __builtin_unreachable();
            case KeyCode::ArrowLeft:
                cursor_ = cursor_
                    .shift(-1, str_.length())
                    .unwrap_or(cursor_);

                return;
            case KeyCode::ArrowRight:
                cursor_ = cursor_
                    .shift(1, str_.length())
                    .unwrap_or(cursor_);

            case KeyCode::ArrowUp:
            case KeyCode::ArrowDown:{
                TODO();
                break;
            }
        }
    }

    constexpr void handle_insert_char(const size_t position, const char chr){
        const auto res = str_.try_insert(position, chr);
        if(res.is_ok()){
            // cursor_ = cursor_.try_shift(1, str_.length());
            // cursor_ = cursor_.try_shift(1, MAX_LENGTH);
            // cursor_ = Cursor{1,MAX_LENGTH};
            // cursor_ = Cursor{static_cast<uint8_t>(position + 1), MAX_LENGTH};
            cursor_ = cursor_
                .shift(1, str_.length())
                .unwrap_or(cursor_);
            // return next_cursor;
            // __builtin_abort();
        }else{
            // return cursor_;
            // __builtin_abort();
        }
    }

    constexpr void handle_backspace(){
        const auto position = cursor_.position();
        const auto res = str_.try_erase(position);
        if(res.is_ok()){
            cursor_ = cursor_
                .shift(-1, str_.length())
                .unwrap_or(cursor_);
        }else{
            DEBUG_PRINTLN("can't erase");
        }
    }

    #if 0
    static void static_test(){
        constexpr auto line = []{
            auto _line = LineEdit();

            _line.handle_key_input(KeyCode::Digit0);
            // _line.handle_insert_char(0, '0');
            _line.handle_key_input(KeyCode::Digit1);
            // _line.handle_key_input(KeyCode::Digit2);

            return _line;
        }();

        static_assert(line.str().length() == 2);
        static_assert(line.cursor().position() == 2);
        // static_assert(line.handle_ == 1);
    }
    #endif

    friend OutputStream & operator <<(OutputStream & os, const LineEdit & self){

        const auto str = self.str();
        const auto cp = self.cursor().position();
        #if 1
        const auto head = str.substr_by_range(0, cp);
        const auto tail = str.substr_by_range(cp, str.length());
        os << os.brackets<'['>() << cp << os.brackets<']'>() << ' ';
        if(head.length()) os << head;
        os << '|';
        if(tail.length()) os << tail;
        #else 
        const auto head = str.substr(0, cp);
        const auto body = str.substr(cp, cp + 1);
        const auto tail = str.substr(cp + 1, str.length());
        if(head.length()) os << head;
        os << '[';
        if(body.length()) os << body;
        os << ']';
        if(tail.length()) os << tail;
        #endif
        return os;
    }

};

}





static void HT16K33_tb(HT16K33 & ht16){
    ht16.init({.pulse_duty = HT16K33::PulseDuty::_8_16}).examine();


    uint8_t cnt = 0;
    // sync::SpinLock locker;
    // locker.lock();
    // locker.unlock();
    // locker.lock();

    // std::atomic_bool a{false};

    hid::KeyBoardComponent<drivers::HT16K33> comp{ht16};
    hid::LineEdit line_edit;

    while(true){
        cnt++;

        // const auto keydata = ht16.get_key_data().examine();
        // const auto may_xy = keydata
        //     .to_first_xy();

        // const auto may_token = may_xy 
        //     .map([](std::tuple<uint8_t, uint8_t> xy){
        //         const auto [x,y] = xy;
        //         return hid::KeyBoardLayout<HT16K33>::map(x,y).to_char();
        //     })
        //     .flatten()
        //     ;
        comp.update().examine();

        using Pattern = HT16K33::GcRam;
        auto test_pattern = Pattern{};
        // test_pattern.fill(cnt);

        auto correct_display_xy = [](const uint8_t _x, const uint8_t _y)
            -> std::tuple<uint8_t, uint8_t>{
            
            const auto x = [&](){
                switch(_x){
                    case 0: return 3;
                    case 1: return 6;
                    case 2: return 2;
                    case 3: return 5;
                    case 4: return 4;
                    case 5: return 1;
                    case 6: return 0;
                    default: return 0;
                }
            }();

            const auto y = [&](){
                switch(_y){
                    case 0: return 5;
                    case 1: return 4;
                    case 2: return 2;
                    case 3: return 3;
                    case 4: return 0;
                    case 5: return 6;
                    case 6: return 7;
                    case 7: return 1;
                    default: return 0;
                }
            }();

            return {y,x};
        };

        auto display_segcode = [&](const uint8_t i, const hid::SegCode code){
            const auto raw = std::bit_cast<uint8_t>(code);

            for(size_t j = 0; j < 8; j++){
                if(not (raw & (1 << j))) continue;
                const auto [x,y] = correct_display_xy(i,j);
                test_pattern.write_pixel(x, y, true).examine();
            }
        };

        // for(int i = 0; i < 7; i++){
        // display_segcode(0, SegCode::_0);
        // display_segcode(1, SegCode::_1);
        // display_segcode(2, SegCode::_2);
        // display_segcode(3, SegCode::_3);
        // display_segcode(4, SegCode::_4);
        // display_segcode(5, SegCode::_5);
        // display_segcode(6, SegCode::_6);

        auto display_str = [&](const StringView& str) {
            for(size_t i = 0; i < str.size(); i++){
                display_segcode(i, hid::SegCode::from_char(str[i]).examine());
            }
            ht16.update_displayer(test_pattern).examine();
        };

        const auto & input = comp.input();
        const auto may_key = input.just_pressed().first_code();
        may_key.inspect([&](const hid::KeyCode code){
            line_edit.handle_key_input(code).examine(); 
            display_str(line_edit.str());
            DEBUG_PRINTLN(line_edit);
        });

        clock::delay(40ms);
    }
}



void ht16k33_main(){
    UART.init({576000});
    DEBUGGER.retarget(&UART);
    DEBUGGER.set_eps(4);

    hal::I2cSw i2c = {&SCL_GPIO, &SDA_GPIO};
    i2c.init(400_KHz);


    HT16K33 ht16{
        HT16K33::Settings::SOP20Settings{},
        hal::I2cDrv{&i2c, HT16K33::DEFAULT_I2C_ADDR}};

    HT16K33_tb(ht16);
}