#include "core/debug/debug.hpp"

#include "hal/bus/uart/uarthw.hpp"
#include "hal/bus/i2c/i2csw.hpp"
#include "hal/gpio/gpio_port.hpp"

#include "src/testbench/tb.h"

#include "drivers/HID/TM1637/TM1637.hpp"
#include "drivers/HID/HT16K33/HT16K33.hpp"


#include "drivers/HID/keycode.hpp"
#include "drivers/HID/axis_input.hpp"
#include "drivers/HID/button_input.hpp"

#include <atomic>
#include "core/sync/spinlock.hpp"
#include "core/sync/barrier.hpp"
#include <barrier>


using namespace ymd;
using namespace ymd::drivers;

#define UART hal::uart2
#define SCL_GPIO hal::PB<0>()
#define SDA_GPIO hal::PB<1>()





// static constexpr auto a = ThisKeyBoardLayout::map(0,0).to_char().unwrap();

namespace ymd::hid{

template<typename T>
class KeyBoardComponent{
};


template<typename T>
struct KeyBoardLayout{
};

template<>
struct KeyBoardLayout<HT16K33>{
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
        if(const auto res = inst_.any_key_pressed();
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



template<size_t N>
class FixedString{
public:
    constexpr FixedString(const StringView str):
        len_(MIN(str.length(), N))
    {
        // memcpy(buf_, str.data(), len_);
        for(size_t i = 0; i < len_; i++){
            buf_[i] = str.data()[i];
        }
    }

    constexpr FixedString():
        len_(0){;}

    // constexpr operator StringView() const {
    //     return StringView(buf_, len);
    // }

    constexpr StringView as_view() const {
        return StringView(buf_, len_);
    }

    [[nodiscard]] constexpr Result<void, void> push_back(const char chr){
        if(len_ >= N) return Err();
        buf_[len_++] = chr;
        return Ok();
    }

    [[nodiscard]] constexpr Result<char, void> pop_back(){
        if(len_ == 0) return Err();
        return Ok(buf_[--len_]);
    }

    constexpr void try_push_back(const char chr){
        if(len_ >= N) return;
        buf_[len_++] = chr;
    }

    [[nodiscard]] constexpr char try_pop_back(){
        if(len_ == 0);
        return buf_[--len_];
    }

    [[nodiscard]] constexpr size_t length() const{
        return len_;
    }

    constexpr void try_insert(const size_t idx, const char chr){
        if (len_ >= N) return;
        // Move characters from the end to make space for the new character
        for (size_t i = len_; i > idx; i--) {
            buf_[i] = buf_[i - 1];
        }
        // Insert the new character at the specified index
        buf_[idx] = chr;
        // Increase the length of the string
        len_++;
    }

    [[nodiscard]] constexpr Result<void, void> insert(const size_t idx, const char chr){
        if (len_ >= N) return Err();
        // Move characters from the end to make space for the new character
        for (size_t i = len_; i > idx; i--) {
            buf_[i] = buf_[i - 1];
        }
        // Insert the new character at the specified index
        buf_[idx] = chr;
        // Increase the length of the string
        len_++;

        return Ok();
    }

    [[nodiscard]] constexpr Result<void, void> erase(const size_t idx){
        // if(idx > len_) return Err();
        // if(idx == 0) return Err();
        // for(size_t i = idx; i < len_; i++){
        //     buf_[i] = buf_[i + 1];
        // }
        // return Ok();
        if (idx > len_) return Err();  // 正确索引检查
        if (len_ == 0) return Err();    // 防止空字符串操作

        // 前移字符（注意循环终止条件）
        for (size_t i = idx; i < len_ - 1; i++) {
            buf_[i] = buf_[i + 1];
        }

        len_--;  // 必须更新长度
        return Ok();
    }

    constexpr void clear(){
        len_ = 0;
    }

    [[nodiscard]]constexpr bool operator==(const StringView & other) const {
        return len_ == other.length() && std::memcmp(buf_, other.data(), len_) == 0;
    } 

    // constexpr Result<void, void> 
    // constexpr char & operator 
private:

    char buf_[N] = {};
    size_t len_;

    // static consteval void static_test(){
    //     constexpr auto str = FixedString<10>("Hello");
    //     constexpr auto str2 = []{
    //         auto _str = FixedString<10>("Hello");
    //         _str.try_push_back('!');
    //         return _str;
    //     }();

    //     constexpr auto str3 = []{
    //         auto _str = FixedString<10>("Hello");
    //         _str.insert(0, '!');
    //         return _str;
    //     }();

    //     constexpr auto str4 = []{
    //         auto _str = FixedString<10>("Hello");
    //         _str.erase(4);
    //         return _str;
    //     }();

    //     static_assert(str.length() == 5);
    //     static_assert(str2.length() == 6);
    //     static_assert(str3.length() == 6);
    //     static_assert(str2 == StringView("Hello!"));
    //     static_assert(str3 == StringView("!Hello"));
    //     static_assert(str4 == StringView("Hell"));
    // }
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

        // constexpr Cursor(const Cursor& other): 
        //     position_(other.position_),
        //     max_limit_(other.max_limit_) 
            
        //     {
        //         DEBUG_PRINTLN("copy", position_, other.position_);
        //     }

        // constexpr Cursor operator = (const Cursor& other){
        //     return Cursor(other);
        // }
        // [[nodiscard]] constexpr Cursor try_shift(int delta, const size_t limit = UINT32_MAX) const{
        //     const auto raw_next_position = int(position_) + delta;
        //     const auto next_position = CLAMP(raw_next_position, 0, int(MIN(limit, max_limit_)));
        //     return copy(next_position);
        // }

        [[nodiscard]] constexpr Cursor try_shift(int delta, size_t _limit = UINT32_MAX) const{
            const int raw_next_position = int(position_) + delta;
            const int limit = int(MIN(_limit, max_limit_));
            // ASSERT(raw_next_position >= 0);
            // ASSERT(raw_next_position < limit); 
            const auto next_position = CLAMP(raw_next_position, 0, limit);
            return copy(next_position);
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
    private:
        uint8_t position_;
        uint8_t max_limit_;


        static void static_test(){
            constexpr auto cursor = Cursor(0, 10);
            static_assert(cursor.position() == 0);
            static_assert(cursor.max_limit() == 10);
            static_assert(cursor.copy(1).position() == 1);
            static_assert(cursor.try_shift(1).position() == 1);
            static_assert(cursor.try_shift(3).position() == 3);

            static_assert(Cursor{1,MAX_LENGTH}.position() == 1);
        }
    };

public:
    constexpr LineEdit():
        str_(),
        cursor_(0, MAX_LENGTH){;}
    constexpr void handle_key_input(const KeyCode code){

        auto insert_char = [this](const KeyCode code){
            handle_insert_char(cursor_.position(), code.to_char().unwrap());
        };

        if(code.is_digit()){
            insert_char(code);
        }else if(code.is_alpha()){
            insert_char(code);
        } else if(code.is_arrow()){
            handle_arrow_input(code);
        }else{
            // __builtin_unreachable
            switch(code.kind()){
                case KeyCode::NumpadComma:
                case KeyCode::NumpadSubtract:
                    insert_char(code);
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
    }

    constexpr void handle_clear(){
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
    static constexpr size_t MAX_LENGTH = 16;
    FixedString<MAX_LENGTH> str_;

    Cursor cursor_;


    constexpr void handle_arrow_input(const KeyCode code){
        switch(code.kind()){
            default: __builtin_unreachable();
            case KeyCode::ArrowLeft:
                cursor_ =  cursor_.try_shift(-1, str_.length());
                return;
            case KeyCode::ArrowRight:
                cursor_ = cursor_.try_shift(1, str_.length());
                return;

            case KeyCode::ArrowUp:
            case KeyCode::ArrowDown:{
                TODO();
                break;
            }
        }
    }

    constexpr void handle_insert_char(const size_t position, const char chr){
        const auto res = str_.insert(position, chr);
        if(res.is_ok()){
            // cursor_ = cursor_.try_shift(1, str_.length());
            // cursor_ = cursor_.try_shift(1, MAX_LENGTH);
            // cursor_ = Cursor{1,MAX_LENGTH};
            // cursor_ = Cursor{static_cast<uint8_t>(position + 1), MAX_LENGTH};
            cursor_ = cursor_.try_shift(1, str_.length());
            // return next_cursor;
            // __builtin_abort();
        }else{
            // return cursor_;
            // __builtin_abort();
        }
    }

    constexpr void handle_backspace(){
        const auto position = cursor_.position();
        const auto res = str_.erase(position);
        if(res.is_ok()){
            cursor_ = cursor_.try_shift(-1, str_.length());
        }else{
            DEBUG_PRINTLN("erase failed");
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
        const auto head = str.substr(0, cp);
        const auto tail = str.substr(cp, str.length());
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

        {
            auto test_pattern = std::array<uint8_t, 16>{};
            test_pattern.fill(cnt);
            ht16.update_displayer(0, std::span(test_pattern)).examine();
        }


        // DEBUG_PRINTLN(may_xy, may_token);
        // DEBUG_PRINTLN(comp.input().pressed().first_code());
        const auto & input = comp.input();
        const auto may_key = input.just_pressed().first_code();
        may_key.inspect([&](const hid::KeyCode code){
            line_edit.handle_key_input(code); 
            // DEBUG_PRINTLN(line_edit.str());
            DEBUG_PRINTLN(line_edit);
        });

        clock::delay(40ms);
    }
}



void ht16k33_main(){
    UART.init(576000);
    DEBUGGER.retarget(&UART);
    DEBUGGER.set_eps(4);

    hal::I2cSw i2c = {SCL_GPIO, SDA_GPIO};
    i2c.init(400_KHz);


    HT16K33 ht16{
        HT16K33::Settings::SOP20Settings{},
        hal::I2cDrv{i2c, HT16K33::DEFAULT_I2C_ADDR}};

    HT16K33_tb(ht16);
}