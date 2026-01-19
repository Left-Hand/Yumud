#pragma once

#include "crsf_primitive.hpp"

namespace ymd::crsf{

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


struct SerialzeFunctions{

    template<size_t Extents>
    [[nodiscard]] static constexpr OptionalUCharPtr ser_zero_terminated_uchars(
        std::span<uint8_t> bytes, 
        const UCharsNullTerminated<Extents> obj
    ){
        return ser_flexible_uchars(bytes, obj, 0);
    }

    [[nodiscard]] static constexpr OptionalUCharPtr ser_0xff_terminated_uchars(
        std::span<uint8_t> bytes, 
        const std::span<const uint8_t> obj
    ){
        return ser_flexible_uchars(bytes, obj, 0);
    }
private:
    [[nodiscard]] static constexpr OptionalUCharPtr ser_flexible_uchars(
        std::span<uint8_t> bytes, 

        const std::span<const uint8_t> obj, 
        uint8_t terminator
    ){
        const auto required_length = obj.size() + 1;
        if(bytes.size() < required_length) return OptionalUCharPtr::from_null();
        auto * ptr = bytes.data();
        std::copy_n(obj.begin(), obj.size(), ptr);
        ptr += obj.size();
        *ptr = terminator;
        ptr++;
        return OptionalUCharPtr::from_valid(ptr);
    }
};


struct [[nodiscard]] SerializeReceiver final{
    std::span<uint8_t> storage;
    size_t idx;

    // enum class Error{

    // };

    using Error = Infallible;


    template<size_t Extents>
    Result<void, Error> push_zero_terminated_uchars(const UCharsNullTerminated<Extents> obj){

    }


private:
    constexpr void check_input_length(size_t length){
        if(idx + length > storage.size()) 
            __builtin_trap();
    }
};


[[nodiscard]] static constexpr std::tuple<uint8_t *, std::span<uint8_t>> 
split_bytes(std::span<uint8_t> bytes, size_t n){ 
    return std::make_tuple(bytes.data(), bytes.subspan(n));
}


enum class DeserError:uint8_t{

};
}