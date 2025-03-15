#pragma once

#include <type_traits>
#include <span>
#include "sys/constants/enums.hpp"


namespace ymd{


// template<typename T>
// requires std::is_standard_layout_v<T>
// class BytesIterator{
// private:
//     const uint8_t * data_;
//     const bool mutes_;
//     const Endian endian_;
//     bool ends_ = false;
//     size_t len_;

//     T temp;

//     BytesIterator(const uint8_t * data, const bool mutes, const Endian endian, const size_t len):
//         data_(data),
//         mutes_(mutes),
//         endian_(endian),
//         len_(len){;}
// public:

//     template<typename U = T>
//     requires std::is_standard_layout_v<U>
//     BytesIterator(const U * data_, const size_t len, const Endian endian):
//         BytesIterator(reinterpret_cast<const uint8_t *>(data_), true, endian, len),

//     template<typename U = T>
//     requires std::is_standard_layout_v<U>
//     BytesIterator(const U data_, const size_t len, const Endian endian):
//         BytesIterator(reinterpret_cast<const uint8_t *>(false), true, endian, len),

//     uint8_t next(){
        
//     }
// };


template<typename T, typename Fn1, typename Fn2, typename Fn3>
[[nodiscard]] __fast_inline
auto iterate_bytes(std::span<T> pdata, Endian endian, Fn1 && do_fn, Fn2 && check_fn, Fn3 && ok_fn){
    using Byte = std::conditional_t<std::is_const_v<T>, const std::byte, std::byte>;
    constexpr size_t dsize = sizeof(T);
    
    const auto bytes = std::span<Byte>(
        reinterpret_cast<Byte *>(std::addressof(pdata[0])), 
        pdata.size() * dsize
    );

    for(size_t i = 0; i < bytes.size(); i += dsize){
        if(endian == MSB){
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
auto iterate_bytes(T & data, size_t len, Endian endian, Fn1 && do_fn, Fn2 && check_fn, Fn3 && ok_fn){
    using Byte = std::conditional_t<std::is_const_v<T>, const std::byte, std::byte>;
    constexpr size_t dsize = sizeof(T);
    
    const auto bytes = std::span<Byte>(
        reinterpret_cast<Byte *>(std::addressof(data)), 
        dsize
    );

    for(size_t i = 0; i < len * dsize; i += dsize){
        if(endian == MSB){
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