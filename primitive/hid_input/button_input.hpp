#pragma once

#include "keycode.hpp"
#include "core/string/string_view.hpp"

namespace ymd::hid{

namespace details{

struct [[nodiscard]] SingleKeyCodeStorage{
    [[nodiscard]] constexpr bool has(const KeyCode code) const {
        return kind_ == code.kind();
    }

    [[nodiscard]] constexpr bool has_any(const std::initializer_list<KeyCode> codes){
        for(const auto& c : codes){
            if(kind_ == c.kind()) return true;
        }
        return false;
    }

    constexpr void add(const KeyCode code){
        kind_ = code.kind();
    }

    constexpr void remove(const KeyCode code){
        //remove is not supported in single kind button
        reset();
    }

    constexpr void emplace(const KeyCode code){
        kind_ = code.kind();
    }

    constexpr void emplace(const SingleKeyCodeStorage & other){
        kind_ = other.kind_;
    }
    constexpr void reset(){
        kind_ = std::bit_cast<KeyCode_Kind>(uint8_t(INVALID_CODE));
    }

    constexpr Option<KeyCode> first_code() const{
        if(kind_ == std::bit_cast<KeyCode_Kind>(uint8_t(INVALID_CODE)))
            return None;
        else return Some(KeyCode(kind_));
    }

    friend OutputStream & operator << (
            OutputStream & os, const SingleKeyCodeStorage & self){
        return os << self.first_code();
    }
private:
    KeyCode_Kind kind_;

    static constexpr uint8_t INVALID_CODE = 0xff;
};

}


template<typename T>
struct ButtonInput;


// https://docs.rs/bevy/latest/bevy/input/struct.ButtonInput.html
template<>
struct [[nodiscard]] ButtonInput<KeyCode>{
public:

    using KeyCodes = details::SingleKeyCodeStorage;
    constexpr void reflash(const Option<KeyCode> may_keycode){
        if(may_keycode.is_none()){
            release_all();
            return;
        }

        const auto code = may_keycode.unwrap();
        const auto & previous_pressed = pressed_;
        if(not previous_pressed.has(code)){
            just_pressed_.add(code);
            just_released_.reset();
        }else{
            just_pressed_.reset();
            just_released_.reset();
        }
        pressed_.emplace(code);
    }

    constexpr void press(const KeyCode code){

        if(pressed_.has(code))
            just_pressed_.remove(code);
        else
            just_pressed_.add(code);
        just_released_.reset();
        pressed_.emplace(code);

    }

    constexpr void release(const KeyCode code){
        // const auto & previous_pressed = pressed_;
        if(pressed_.has(code))
            just_released_.add(code);
        else
            just_released_.remove(code);
        just_pressed_.reset();
        pressed_.emplace(code);
    }

    constexpr void release_all(){
        just_released_.emplace(pressed_);
        just_pressed_.reset();
        pressed_.reset();
    }

    [[nodiscard]] constexpr KeyCodes pressed() const {
        return pressed_;
    }

    [[nodiscard]] constexpr KeyCodes just_pressed() const {
        return just_pressed_;
    }

    [[nodiscard]] constexpr KeyCodes just_released() const {
        return just_released_;
    }

    friend OutputStream & operator << (
        OutputStream & os, const ButtonInput<KeyCode> & self
    ){
        auto print_item = [&os](const StringView str, const KeyCodes & codes){
            os << str << codes;
        };

        os << os.brackets<'{'>();
        print_item("~:", self.pressed_); os << os.splitter();
        print_item("+:", self.just_pressed_); os << os.splitter();
        print_item("-:", self.just_released_);
        os << os.brackets<'}'>();

        return os;
    }
private:
    KeyCodes pressed_;
    KeyCodes just_pressed_;
    KeyCodes just_released_;
};

}