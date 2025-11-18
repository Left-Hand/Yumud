#pragma once

#include <type_traits>
#include <span>
#include <bit>

#include "core/constants/enums.hpp"


namespace ymd{

template<typename T, typename Fn1, typename Fn2, typename Fn3>
[[nodiscard]] __fast_inline
auto iterate_bytes(std::span<T> pbuf, std::endian endian, Fn1 && do_fn, Fn2 && check_fn, Fn3 && ok_fn){
    using Byte = std::conditional_t<std::is_const_v<T>, const uint8_t, uint8_t>;
    constexpr size_t dsize = sizeof(T);
    
    const auto bytes = std::span<Byte>(
        reinterpret_cast<Byte *>(std::addressof(pbuf[0])), 
        pbuf.size() * dsize
    );

    for(size_t i = 0; i < bytes.size(); i += dsize){
        if(endian == std::endian::big){
            for(size_t j = dsize; j > 0; j--){
                const bool is_end = ((j == 1) && (i == bytes.size() - dsize));
                const auto err = std::forward<Fn1>(do_fn)(bytes[i + j - 1], is_end);
                if(std::forward<Fn2>(check_fn)(err)) return err;
            }
        }else{
            for(size_t j = 0; j < dsize; j++){
                const bool is_end = (i + j == bytes.size() - 1);
                const auto err = std::forward<Fn1>(do_fn)(bytes[i + j], is_end);
                if(std::forward<Fn2>(check_fn)(err)) return err;
            }
        }
    }

    return std::forward<Fn3>(ok_fn)();
}

template<typename T, typename Fn1, typename Fn2, typename Fn3>
[[nodiscard]] __fast_inline
auto iterate_bytes(T & data, size_t len, std::endian endian, Fn1 && do_fn, Fn2 && check_fn, Fn3 && ok_fn){
    using Byte = std::conditional_t<std::is_const_v<T>, const uint8_t, uint8_t>;
    constexpr size_t dsize = sizeof(T);
    
    const auto bytes = std::span<Byte>(
        reinterpret_cast<Byte *>(std::addressof(data)), 
        dsize
    );

    for(size_t i = 0; i < len * dsize; i += dsize){
        if(endian == std::endian::big){
            for(size_t j = dsize; j > 0; j--){
                const bool is_end = ((j == 1) && (i == dsize * len - dsize));
                const auto err = std::forward<Fn1>(do_fn)(bytes[j - 1], is_end);
                if(std::forward<Fn2>(check_fn)(err)) return err;
            }
        }else{
            for(size_t j = 0; j < dsize; j++){
                const bool is_end = (i + j == dsize * len - 1);
                const auto err = std::forward<Fn1>(do_fn)(bytes[j], is_end);
                if(std::forward<Fn2>(check_fn)(err)) return err;
            }
        }
    }

    return std::forward<Fn3>(ok_fn)();
}

};