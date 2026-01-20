#pragma once

#include <cstddef>

// namespace ymd::str{
namespace ymd{


struct [[nodiscard]] OptionalUCharPtr final{
    using Self = OptionalUCharPtr;  
    static constexpr Self from_valid(const unsigned char* ptr){
        return Self(ptr);
    }

    static constexpr Self from_null(){
        return Self(nullptr);
    }

    [[nodiscard]] bool is_null() const{
        return ptr_ == nullptr;
    }

    [[nodiscard]] const unsigned char * unwrap() const{
        if(is_null()) __builtin_trap();
        return ptr_;
    }
private:
    const unsigned char* ptr_;

    explicit constexpr OptionalUCharPtr(const unsigned char* ptr): ptr_(ptr){;}
    explicit constexpr OptionalUCharPtr(const std::nullptr_t ): ptr_(nullptr){;}
};
}