#pragma once

#include <variant>
#include <type_traits>

#include "Option.hpp"
#include "typetraits/args_traits.hpp"
// #include "hal/bus/bus_error.hpp"

namespace ymd{

template<typename ... Ts>
class SumtypeError{
public:
    using Self = SumtypeError<Ts...>;

    template<typename Raw, typename T = magic::first_convertible_arg_t<Raw, Ts...>>
    requires (!std::is_void_v<T>)
    constexpr SumtypeError(Raw && val):
        value_(std::in_place_type<T>, static_cast<T>(val)) { // Add std::in_place_type<Raw> to specify variant type
    }

    
    template<typename Raw, typename T = magic::first_convertible_arg_t<Raw, Ts...>>
    requires (!std::is_void_v<T>)

    constexpr bool is() const{
        return std::holds_alternative<T>(value_);
    }

    template<typename T>
    constexpr Option<T &> as() {
        if(! this->is<T>()) return None;
        return Some<T *>(&std::get<T>(value_)); 
    }

    template<typename T>
    constexpr Option<const T &> as() const {
        if(! this->is<T>()) return None;
        return Some<const T *>(&std::get<T>(value_)); 
    }


    constexpr bool operator ==(const Self & other) const {
        // if(this->var_index() != other.var_index()) return false;
        // return 
        return this->value_ == other.value_;
    }

    template<typename Raw, typename T = magic::first_convertible_arg_t<Raw, Ts...>>
    requires (!std::is_void_v<T>)
    constexpr bool operator ==(const Raw &rhs) const {
        if(not this->is<T>()) return false;
        return this->as<T>().unwrap() == static_cast<T>(rhs);
    }

    template<typename Raw, typename T = magic::first_convertible_arg_t<Raw, Ts...>>
    requires (!std::is_void_v<T>)
    constexpr bool operator !=(const Raw &rhs) const {
        return !(this->operator ==(rhs));
    }

    friend OutputStream & operator <<(OutputStream & os,const SumtypeError & self){
        // 使用 std::visit 遍历 std::variant
        std::visit([&os](const auto& value) {
            using T = std::decay_t<decltype(value)>;

            // 检查类型是否可被 OutputStream 打印
            if constexpr (requires(OutputStream& os, const T& value) {os << value;}) {
                os << value; // 如果可打印，则直接打印
            } else {
                os << "[Unprintable type]"; // 否则打印提示信息
            }
        }, self.value_);

        return os;
    }
private:
    std::variant<Ts...> value_;

    constexpr size_t var_index() const {
        return value_.index();
    }

};

namespace details{
    enum class MyError_Kind:uint8_t{
        I2cError,
        Unspecified = 0xff,
    };
}

#define DEF_ERROR_WITH_KIND(name, kind)\
class name:public SumtypeError<kind>{\
public:\
    using Kind = kind;\
    using Super = SumtypeError<Kind>;\
    using Super::Super;\
    using enum Kind;\
};\


#define DEF_ERROR_SUMWITH_HALERROR(name, kind)\
class name:public SumtypeError<kind, hal::HalError>{\
public:\
    using Kind = kind;\
    using Super = SumtypeError<Kind, hal::HalError>;\
    using Super::Super;\
    using enum Kind;\
    static constexpr name HalError(const hal::HalError & err){\
        return name(err);\
    }\
};\

// DEF_ERROR_SUMWITH_HALERROR(MyError, details::MyError_Kind)

// __inline void test_err(){
//     // static constexpr MyError err{MyError::I2cError};
//     {
//         // using t1 = err_enum_t<typename hal::HalResult::Kind>;
//         // using t2 = err_enum_t<hal::HalResult>;
//         // using t = magic::first_convertible_arg_t<typename hal::HalResult::Kind, hal::HalResult>;
//         // static constexpr bool ok = std::is_convertible_v<typename hal::HalResult::Kind, hal::HalResult>;
//         static constexpr SumtypeError<hal::HalResult> err0{hal::HalResult::Ok()};
//         static constexpr SumtypeError<hal::HalResult> err{hal::HalResult::AckTimeout};
//         // static constexpr MyError err2(hal::HalResult::AckTimeout);
//         // static constexpr MyError err2(MyError::I2cError);

//         static constexpr MyError err2 = MyError::I2cError;
//         static constexpr MyError err3(hal::HalResult::AckTimeout);
//         static constexpr auto err4 = err2.as<MyError::Kind>().unwrap();

//         // static constexpr auto err5 = err2.as<hal::HalResult>().unwrap();
//     }
// }
}