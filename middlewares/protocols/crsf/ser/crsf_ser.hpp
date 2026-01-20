#pragma once

#include "../crsf_primitive.hpp"

namespace ymd::crsf{

struct SerialzeFunctions{

    template<size_t Extents>
    [[nodiscard]] static constexpr OptionalUCharPtr ser_zero_terminated_uchars(
        std::span<uint8_t> bytes, 
        const std::span<const uint8_t, Extents> obj
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
    std::span<uint8_t> uchars;
    size_t idx;

    using Error = Infallible;


    template<size_t Extents>
    constexpr Result<void, Error> push_zero_terminated_uchars(const std::span<const uint8_t, Extents> obj){
        if(const auto res = check_input_length<Error>(obj.length() + 1); 
            res.is_err()) return Err(res.unwrap_err());

        push_bytes_unchecked(obj.uchars());
        return Ok();
    }


private:
    template<typename E>
    constexpr Result<void, E> check_input_length(size_t length){
        if(idx + length > uchars.size()){
            if constexpr(std::is_same_v<E, Infallible>){
                __builtin_trap();
            }else{
                return Err(E());
            }
        }
        return Ok();
    }

    constexpr void push_bytes_unchecked(std::span<const uint8_t> bytes){
        std::copy_n(bytes.data(), bytes.size(), uchars.begin() + idx);
        idx += bytes.size();
    }
};


[[nodiscard]] static constexpr std::tuple<uint8_t *, std::span<uint8_t>> 
split_bytes(std::span<uint8_t> bytes, size_t n){ 
    return std::make_tuple(bytes.data(), bytes.subspan(n));
}


enum class [[nodiscard]] DeError:uint8_t{

};
}