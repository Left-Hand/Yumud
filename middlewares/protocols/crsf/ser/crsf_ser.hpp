#pragma once

#include "../crsf_primitive.hpp"
#include "core/utils/bytes/bytes_caster.hpp"
#include "core/utils/bits/bits_caster.hpp"

namespace ymd::crsf{


template<typename Policy>
struct SerialzeFunctions{

    using SerResult = Policy::SerResult;
    using Buffer = Policy::Buffer;


    template<size_t Extents>
    [[nodiscard]] static constexpr SerResult ser_zero_terminated_uchars(
        Buffer buffer, 
        const std::span<const uint8_t, Extents> obj
    ) noexcept {
        return ser_extra_terminated_uchars(buffer, obj, 0);
    }

    template<size_t Extents>
    [[nodiscard]] static constexpr SerResult ser_0xff_terminated_uchars(
        Buffer buffer, 
        const std::span<const uint8_t, Extents> obj
    ) noexcept {
        return ser_extra_terminated_uchars(buffer, obj, 0xff);
    }

    template<typename D>
    [[nodiscard]] static constexpr SerResult ser_be_int(
        Buffer buffer, 
        const auto obj
    ) noexcept {
        using UD = std::make_unsigned_t<D>;

        UD bits = std::bit_cast<UD>(obj);
        constexpr size_t required_length = sizeof(D);
        if(not is_compatible(buffer, required_length)) return make_oom_error();

        #pragma GCC unroll 8
        for(size_t i = 0; i < required_length; i++){
            buffer[i] = static_cast<uint8_t>((bits >> ((required_length - 1 - i) * 8)) & 0xff);
        }

        return make_feed_length(required_length);
    }

    template<typename T>
    [[nodiscard]] static constexpr SerResult ser_bits_intoable(
        Buffer buffer, 
        const T obj
    ) noexcept {
        using D = to_bits_t<T>;
        return ser_be_int<D>(buffer, obj_to_bits<std::decay_t<T>>(obj));
    }
private:
    template<size_t Extents>
    [[nodiscard]] static constexpr SerResult ser_extra_terminated_uchars(
        Buffer buffer, 

        const std::span<const uint8_t, Extents> obj, 
        uint8_t terminator
    ) noexcept {
        const auto required_length = obj.size() + 1;
        if(not is_compatible(buffer, required_length)) return make_oom_error();
        uint8_t * ptr = get_buffer_begin(buffer);
        std::copy_n(obj.data(), obj.size(), ptr);
        buffer[obj.size()] = terminator;
        return make_feed_length(required_length);
    }

    template<size_t Extents>
    [[nodiscard]] static constexpr SerResult ser_uchars(
        Buffer buffer, 

        const std::span<const uint8_t, Extents> obj, 
        uint8_t terminator
    ) noexcept {
        const auto required_length = obj.size();
        if(not is_compatible(buffer, required_length)) return make_oom_error();
        uint8_t * ptr = get_buffer_begin(buffer);
        std::copy_n(obj.data(), obj.size(), ptr);
        return make_feed_length(required_length);
    }

    [[nodiscard]] static constexpr bool is_compatible(Buffer buffer, size_t required_length) noexcept {
        return Policy::is_compatible(buffer, required_length);
    }

    [[nodiscard]] static constexpr uint8_t * get_buffer_begin(Buffer buffer) noexcept {
        return Policy::get_buffer_begin(buffer);
    }

    [[nodiscard]] static constexpr auto make_oom_error() noexcept {
        return Policy::make_oom_error();
    }

    [[nodiscard]] static constexpr auto make_feed_length(const size_t length) noexcept {
        return Policy::make_feed_length(length);
    }
};



#if 1
enum class SerError:uint8_t{
    OutOfMemory
};
#else
using SerError = Infallible;
#endif

template<typename E>
using EitherSize = Result<size_t, E>;

using SerSize = EitherSize<SerError>;


struct CheckedPolicy{
    using SerResult = Result<size_t, SerError>;
    using Buffer = std::span<uint8_t>;

    [[nodiscard]] static constexpr bool is_compatible(Buffer buffer, size_t required_length) noexcept {
        return buffer.size() >= required_length;
    }

    [[nodiscard]] static constexpr uint8_t * get_buffer_begin(Buffer buffer) noexcept {
        return buffer.data();
    }

    [[nodiscard]] static constexpr SerResult make_oom_error() noexcept {
        return Err(SerError::OutOfMemory);
    }

    [[nodiscard]] static constexpr SerResult make_feed_length(const size_t length) noexcept {
        return Ok(length);
    }
};

struct NoCheckPolicy{
    using SerResult = size_t;
    using Buffer = uint8_t *;


    [[nodiscard]] static constexpr bool is_compatible(Buffer buffer, size_t required_length) noexcept {
        (void)buffer;
        (void)required_length;
        return true;
    }
    static constexpr uint8_t * get_buffer_begin(Buffer buffer) noexcept {
        return buffer;
    }

    [[noreturn]]
    static constexpr SerResult make_oom_error() noexcept {
        __builtin_unreachable();
    }

    static constexpr SerResult make_feed_length(const size_t length) noexcept {
        return length;
    }
};




struct [[nodiscard]] SerializeReceiver final{
public:
    using NoCheckSerialzeFunc = SerialzeFunctions<NoCheckPolicy>;
    std::span<uint8_t> uchars;
    size_t idx;

    using Error = SerError;
    using SerResult = Result<void, Error>;


    template<size_t Extents>
    constexpr Result<void, SerError> recv_zero_terminated_uchars(const std::span<const uint8_t, Extents> obj) noexcept {
        if(const auto res = check_input_length(obj.size() + 1);
            res.is_err()) return res;
        const auto feed_len = NoCheckSerialzeFunc::ser_zero_terminated_uchars(uchars.data() + idx, obj);
        idx += feed_len;
        return Ok();
    }

    template<size_t Extents>
    constexpr Result<void, SerError> recv_0xff_terminated_uchars(const std::span<const uint8_t, Extents> obj) noexcept {
        if(const auto res = check_input_length(obj.size() + 1);
            res.is_err()) return res;
        const auto feed_len = NoCheckSerialzeFunc::ser_0xff_terminated_uchars(uchars.data() + idx, obj);
        idx += feed_len;
        return Ok();
    }

    template<typename D>
    constexpr Result<void, SerError> recv_be_int(const auto int_val) noexcept {
        if constexpr(std::is_same_v<D, uint24_t> || std::is_same_v<D, int24_t>){
            if(const auto res = check_input_length(3);
                res.is_err()) return res;
            const auto feed_len = NoCheckSerialzeFunc::ser_be_int<D>(uchars.data() + idx, static_cast<D>(int_val));
            idx += feed_len;
            return Ok();
        }else{
            if(const auto res = check_input_length(sizeof(D));
                res.is_err()) return res;
            const auto feed_len = NoCheckSerialzeFunc::ser_be_int<D>(uchars.data() + idx, static_cast<D>(int_val));
            idx += feed_len;
            return Ok();
        }

    }

    template<typename T>
    constexpr Result<void, SerError> recv_bits_intoable(const T obj) noexcept {
        return recv_be_int<to_bits_t<T>>(uchars.data(), obj_to_bits<std::decay_t<T>>(obj));
    }

    template<typename E>
    requires (std::is_enum_v<E>)
    constexpr Result<void, SerError> recv_enum(const E obj) noexcept {
        static_assert(sizeof(E) == 1);
        return recv_be_int<uint8_t>(static_cast<uint8_t>(obj));
    }

    constexpr Result<void, SerError> recv_fp32(const math::fp32 obj) noexcept{
        return recv_be_int<uint32_t>(obj.to_bits());
    }
private:
    constexpr Result<void, SerError> check_input_length(size_t length) noexcept {
        if(idx + length > uchars.size()){
            return Err(SerError::OutOfMemory);
        }
        return Ok();
    }

    constexpr void recv_bytes_unchecked(std::span<const uint8_t> bytes) noexcept {
        std::copy_n(bytes.data(), bytes.size(), uchars.begin() + idx);
        idx += bytes.size();
    }
};


// [[nodiscard]] static constexpr std::tuple<uint8_t *, std::span<uint8_t>> 
// split_bytes(std::span<uint8_t> bytes, size_t n) noexcept { 
//     return std::make_tuple(bytes.data(), bytes.subspan(n));
// }


enum class [[nodiscard]] DeError:uint8_t{

};
}