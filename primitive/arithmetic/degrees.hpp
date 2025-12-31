#pragma once

namespace ymd{

template<typename T>
struct Degrees final{
public:
    static constexpr Degrees<T> from_radians(T rad){
        Degrees<T> self;
        self.value_ = (rad * T(180.0 / PI));
        return self;
    }
    constexpr T to_radians() const { return value_ * T(PI / 180.0); }

    friend OutputStream & operator<<(OutputStream & os, const Degrees<T>& self){
        // return os << self.value_ << "'C";
        // return os << self.value_ << char(248);
        return os << self.value_ << '\'';
    }
private:
    T value_;
};


template<typename T>
static constexpr Degrees<T> to_degrees(const T rad){
    return Degrees<T>::from_radians(rad);
}
}