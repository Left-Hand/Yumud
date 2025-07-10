#pragma once

#include "core/math/real.hpp"
#include "core/stream/ostream.hpp"
#include "core/math/realmath.hpp"
#include "core/utils/stdrange.hpp"

#include "digipw/prelude/abdq.hpp"
#include "dsp/filter/butterworth/ButterBandFilter.hpp"
#include "dsp/filter/butterworth/ButterSideFilter.hpp"



namespace ymd{


struct CurrentSensor{

    static constexpr real_t CURRENT_CUTOFF_FREQ = 400;

    struct Config{
        uint32_t fs;
    };

    constexpr explicit CurrentSensor(const Config & cfg):
        mid_filter_({CURRENT_CUTOFF_FREQ, cfg.fs}){
    }

    constexpr void reconf(const Config & cfg){
        mid_filter_.reconf({CURRENT_CUTOFF_FREQ, cfg.fs});
    }



    constexpr void update(const real_t u, const real_t v, const real_t w){
        uvw_raw_ = {u,v,w};

        mid_filter_.update((uvw_raw_.u + uvw_raw_.v + uvw_raw_.w) * q16(1.0/3));

        mid_curr_ = mid_filter_.get();
        uvw_curr_[0] = (uvw_raw_.u - mid_curr_ - uvw_bias_.u);
        uvw_curr_[1] = (uvw_raw_.v - mid_curr_ - uvw_bias_.v);
        uvw_curr_[2] = (uvw_raw_.w - mid_curr_ - uvw_bias_.w);

        ab_curr_ = digipw::AbCurrent::from_uvw(uvw_curr_);
    }

    constexpr void reset(){
        uvw_curr_ = {0, 0, 0};
        uvw_bias_ = {0, 0, 0};
        ab_curr_ = {0, 0};
    }


    constexpr const auto &  raw()const {return uvw_raw_;}
    constexpr const auto &  mid() const {return mid_curr_;}
    constexpr const auto &  uvw()const{return uvw_curr_;}
    constexpr const auto & ab()const{return ab_curr_;}

private:

    digipw::UvwCurrent uvw_bias_;
    digipw::UvwCurrent uvw_raw_;
    digipw::UvwCurrent uvw_curr_;
    real_t mid_curr_;
    digipw::AbCurrent ab_curr_;
    dsp::ButterLowpassFilter<q16, 4> mid_filter_;
};


struct RawBytes;
struct ReadableAscii;


template<typename Protocol, typename T>
struct SerializeIter{};


enum class DeserializeError:uint8_t{
    BytesLengthShort
};


template<typename Protocol, typename T>
struct Deserializer{};


template<typename Protocol, typename T>
struct SerializeIterMaker{
    static constexpr auto make(const T & obj){
        return SerializeIter<Protocol, T>(obj);
    }
};


template<typename Protocol, typename T>
struct DeserializerMaker{
    static constexpr auto make(){
        return Deserializer<Protocol, T>{};
    }
};


template<typename Protocol, typename T>
struct serialize_iter_support_sso:std::false_type{};

template<typename Protocol, typename T>
static constexpr bool serialize_iter_sso_v = serialize_iter_support_sso<Protocol, T>::value;


template<typename Protocol, typename T>
static constexpr auto make_serialize_iter(const T & obj){
    return SerializeIterMaker<Protocol, T>::make(obj);
}



template<typename Protocol, typename T>
static constexpr auto make_deserializer() {
    return DeserializerMaker<Protocol, T>::make();
}


template<typename Protocol, typename T>
static constexpr auto make_deserialize(auto && pbuf) {
    return make_deserializer<Protocol, T>()
        .deserialize(std::forward<std::decay_t<decltype(pbuf)>>(pbuf));
}


template<size_t Q>
struct SerializeIter<RawBytes, iq_t<Q>>{
    constexpr explicit SerializeIter(const iq_t<Q> num):
        buf_(serialize(num)){;}
    constexpr bool has_next() const {
        return pos_ < N;
    }
    constexpr uint8_t next() {
        return buf_[pos_++];
    }

    static constexpr std::array<uint8_t, 4> serialize(const iq_t<Q> num){
        const auto inum = num.to_i32();
        return std::bit_cast<std::array<uint8_t, 4>>(inum);
    } 
private:
    static constexpr size_t N = sizeof(decltype(serialize(std::declval<iq_t<Q>>())));
    using Buf = std::array<uint8_t, N>;
    Buf buf_;
    size_t pos_ = 0;
};

template<typename Protocol, size_t Q>
struct serialize_iter_support_sso<Protocol, iq_t<Q>>:std::true_type{};


template<typename T>
requires (std::is_integral_v<T> || std::is_floating_point_v<T>)
struct SerializeIter<RawBytes, T>{
    static constexpr size_t N = sizeof(T);
    constexpr explicit SerializeIter(const T num):
        buf_(serialize(num)){;}

    constexpr bool has_next() const {
        return pos_ < N;
    }
    constexpr uint8_t next() {
        return buf_[pos_++];
    }

    static constexpr std::array<uint8_t, N> serialize(const T num){
        return std::bit_cast<std::array<uint8_t, N>>(num);
    } 
private:
    using Buf = std::array<uint8_t, N>;
    Buf buf_;
    size_t pos_ = 0;
};

template<typename Protocol, typename T>
requires (std::is_integral_v<T> || std::is_floating_point_v<T>)
struct serialize_iter_support_sso<Protocol, T>:std::true_type{;};


// 枚举类型特化
template<typename T>
requires std::is_enum_v<T>
struct SerializeIter<RawBytes, T> {
    using UnderlyingType = std::underlying_type_t<T>;
    static constexpr size_t N = sizeof(UnderlyingType);
    
    constexpr explicit SerializeIter(const T value)
        : buf_(serialize(value)) {}

    constexpr bool has_next() const { return pos_ < N; }
    
    constexpr uint8_t next() { return buf_[pos_++]; }

    static constexpr std::array<uint8_t, N> serialize(const T value) {
        return SerializeIter<RawBytes, UnderlyingType>::serialize(
            static_cast<UnderlyingType>(value));
    }

private:
    std::array<uint8_t, N> buf_;
    size_t pos_ = 0;
};

template<typename Protocol, typename T>
requires(std::is_enum_v<T>)

struct serialize_iter_support_sso<Protocol, T>:std::true_type{;};


template<typename Protocol, typename ... Ts>
struct SerializeIter<Protocol, std::tuple<Ts ... >> {
    static constexpr size_t N = sizeof...(Ts);
    
    constexpr explicit SerializeIter(const std::tuple<Ts...> & tup)
        : iters_(make_iter_tuple(tup, std::index_sequence_for<Ts...>{})) {}
        
    constexpr bool has_next() const {
        if (pos_ >= N) return false;
        
        return [this]<size_t ...Is>(std::index_sequence<Is...>) {
            bool has_next = false;
            // 使用折叠表达式检查当前迭代器是否有下一个元素
            ((Is == pos_ ? (has_next = std::get<Is>(iters_).has_next()) : false), ...);
            return has_next;
        }(std::make_index_sequence<N>{});
    }
    
    constexpr uint8_t next() {
        return [this]<size_t ...Is>(std::index_sequence<Is...>) -> uint8_t {
            uint8_t val = 0;
            // 使用折叠表达式调用当前迭代器的next()
            ((Is == pos_ ? (val = forward<Is>()) : 0), ...);
            return val;
        }(std::make_index_sequence<N>{});
    }

    template<size_t I>
    constexpr uint8_t forward() {
        auto& iter = std::get<I>(iters_);
        uint8_t val = iter.next();
        
        // 如果当前迭代器完成，移动到下一个迭代器
        if(!iter.has_next()) {
            pos_++;
        }
        
        return val;
    }

private:
    template<size_t... Is>
    static constexpr auto make_iter_tuple(const std::tuple<Ts...>& tup, std::index_sequence<Is...>) {
        return std::tuple{
            [&]<size_t I>{
                using RawType = std::tuple_element_t<I, std::tuple<Ts...>>;
                using ElemType = std::decay_t<RawType>;
                if constexpr (serialize_iter_sso_v<Protocol, ElemType>) {
                    return SerializeIter<Protocol, ElemType>{std::get<I>(tup)};
                } else {
                    return SerializeIter<Protocol, ElemType>{std::get<I>(tup)};
                }
            }.template operator()<Is>()...
        };
    }
    
    using Iters = decltype(make_iter_tuple(std::declval<std::tuple<Ts...>>(), std::index_sequence_for<Ts...>{}));
    Iters iters_;
    size_t pos_ = 0;
};


template<typename T>
requires std::is_aggregate_v<std::decay_t<T>>
OutputStream& print_struct(OutputStream& os, T&& value) {
    using AggregateType = std::decay_t<T>;
    
    os.scoped(reflect::type_name<AggregateType>())([&]<size_t... Is>(std::index_sequence<Is...>) -> OutputStream& {
        // Fold expression with conditional splitter insertion
        bool first = true;
        return(..., [&](const auto& field) -> OutputStream& {
            if (!first) os << os.splitter();
            first = false;
            return os << os.field(reflect::member_name<Is>(value))(os << reflect::get<Is>(value));
        }(reflect::get<Is>(value)));
    }(std::make_index_sequence<reflect::size<AggregateType>()>{}));
    
    return os;
}



template<size_t Q>
struct Deserializer<RawBytes, iq_t<Q>> {
    static constexpr size_t N = sizeof(iq_t<Q>);
    [[nodiscard]] static constexpr size_t size(std::span<const uint8_t>){
        return Q;
    }

    [[nodiscard]] __fast_inline static constexpr std::span<const uint8_t>
    take(std::span<const uint8_t> pbuf){
        return pbuf.subspan(size(pbuf));
    }

    [[nodiscard]] static constexpr Result<iq_t<Q>, DeserializeError> 
    deserialize(std::span<const uint8_t> pbuf) {
        if(pbuf.size() < Q) return Err(DeserializeError::BytesLengthShort);
        int32_t val = std::bit_cast<int32_t>(
            std::array<uint8_t, N>{pbuf[0], pbuf[1], pbuf[2], pbuf[3]});
        return Ok(iq_t<Q>::from_i32(val));
    }
};


template<typename T>
requires (std::is_integral_v<T> || std::is_floating_point_v<T>)
struct Deserializer<RawBytes, T> {
    static constexpr size_t N = sizeof(T);
    [[nodiscard]] static constexpr size_t size(std::span<const uint8_t>){
        return N;
    }

    [[nodiscard]] __fast_inline static constexpr std::span<const uint8_t>
    take(std::span<const uint8_t> pbuf){
        return pbuf.subspan(size(pbuf));
    }

    [[nodiscard]] static constexpr Result<T, DeserializeError> 
    deserialize(std::span<const uint8_t> pbuf) {
        if (pbuf.size() < N) {
            return Err(DeserializeError::BytesLengthShort);
        }

        std::array<uint8_t, N> bytes{};
        std::copy_n(pbuf.data(), N, bytes.begin());
        return Ok(std::bit_cast<T>(bytes));
    }
};


template<typename Protocol, typename... Ts>
struct Deserializer<Protocol, std::tuple<Ts...>> {
    static constexpr size_t N = (sizeof(Ts) + ...);

    [[nodiscard]] static constexpr size_t size() {
        return (Deserializer<Protocol, Ts>::size() + ...);
    }

    [[nodiscard]] __fast_inline static constexpr std::span<const uint8_t>
    take(std::span<const uint8_t> pbuf) {
        return pbuf.subspan(size());
    }

    [[nodiscard]] static constexpr Result<std::tuple<Ts...>, DeserializeError>
    deserialize(std::span<const uint8_t> data) {
        if (data.size() < size()) {
            return Err(DeserializeError::BytesLengthShort);
        }

        return deserialize_impl(data, std::index_sequence_for<Ts...>{});
    }

private:
    template<size_t... Is>
    [[nodiscard]] static constexpr Result<std::tuple<Ts...>, DeserializeError>
    deserialize_impl(std::span<const uint8_t> data, std::index_sequence<Is...>) {
        std::tuple<Ts...> result;
        std::span<const uint8_t> remaining = data;

        // Fold expression to deserialize each element sequentially
        auto deserialize_element = [&]<size_t I>(std::integral_constant<size_t, I>) {
            using ElementType = std::tuple_element_t<I, std::tuple<Ts...>>;
            auto elem_result = make_deserialize<Protocol, ElementType>(remaining);
            if (elem_result.is_err()) {
                return elem_result.unwrap_err();
            }
            std::get<I>(result) = elem_result.unwrap();
            remaining = Deserializer<Protocol, ElementType>::take(remaining);
            return DeserializeError::BytesLengthShort; // Dummy return, won't be used
        };

        // Execute deserialization for each element
        (..., deserialize_element(std::integral_constant<size_t, Is>{}));

        return Ok(result);
    }
};




#define DERIVE_SERIALIZE_AS_TUPLE(T)\
template<typename Protocol>\
struct SerializeIterMaker<Protocol, T>{\
    static constexpr auto make(T obj){\
        return make_serialize_iter<Protocol>(reflect::to<std::tuple>(obj));\
    }\
};


#define DERIVE_DEBUG_AS_DISPLAY(T)\
OutputStream & operator<<(OutputStream & os, const T & self){ \
    return print_struct(os, self);\
}


}


namespace ymd::dsp{
template<typename T>
struct ComplementaryFilter{
    struct Config{
        T kq;
        T ko;
        uint fs;
    };
    

    constexpr ComplementaryFilter(const Config & config){
        reconf(config);
        reset();
    }


    constexpr void reconf(const Config & cfg){
        kq_ = cfg.kq;
        kq_ = cfg.kq;
        dt_ = T(1) / cfg.fs;
    }

    constexpr T operator ()(const T rot, const T gyr){

        if(!inited_){
            rot_ = rot;
            rot_unfiltered_ = rot;
            inited_ = true;
        }else{
            rot_unfiltered_ += gyr * delta_t_;
            rot_unfiltered_ = kq_ * rot_ + (1-kq_) * rot;
            rot_ = ko_ * rot_ + (1-ko_) * rot_unfiltered_;
        }
    
        last_rot_ = rot;
        last_gyr_ = gyr;

        return rot_;
    }

    constexpr void reset(){
        rot_ = 0;
        rot_unfiltered_ = 0;
        last_rot_ = 0;
        last_gyr_ = 0;
        inited_ = false;
    }

    constexpr T get() const {
        return rot_;
    }

private:
    T kq_;
    T ko_;
    T dt_;
    T rot_;
    T rot_unfiltered_;
    T last_rot_;
    T last_gyr_;
    // T last_time;

    uint delta_t_;
    
    bool inited_;
};

}


// struct TurnSolver{
//     uint16_t ta = 0;
//     uint16_t tb = 0;
//     real_t pa = 0;
//     real_t pb = 0;
//     real_t va = 0;
//     real_t vb = 0;
// };

// [[maybe_unused]] static real_t demo(uint milliseconds){
//     // using Vector2<real_t> = CubicInterpolation::Vector2<real_t>;
//     static TurnSolver turnSolver;
    
//     uint32_t turnCnt = milliseconds % 2667;
//     uint32_t turns = milliseconds / 2667;
    
//     scexpr real_t velPoints[7] = {
//         real_t(20)/360, real_t(20)/360, real_t(62.4)/360, real_t(62.4)/360, real_t(20.0)/360, real_t(20.0)/360, real_t(20.0)/360
//     };
    
//     scexpr real_t posPoints[7] = {
//         real_t(1.0f)/360,real_t(106.1f)/360,real_t(108.1f)/360, real_t(126.65f)/360, real_t(233.35f)/360,real_t(359.0f)/360,real_t(361.0f)/360
//     };

//     scexpr uint tickPoints[7] = {
//         0, 300, 400, 500, 2210, 2567, 2667 
//     };

//     int8_t i = 6;

//     while((turnCnt < tickPoints[i]) && (i > -1))
//         i--;
    
//     turnSolver.ta = tickPoints[i];
//     turnSolver.tb = tickPoints[i + 1];
//     auto dt = turnSolver.tb - turnSolver.ta;

//     turnSolver.va = velPoints[i];
//     turnSolver.vb = velPoints[i + 1];
    
//     turnSolver.pa = posPoints[i];
//     turnSolver.pb = posPoints[i + 1];
//     real_t dp = turnSolver.pb - turnSolver.pa;

//     real_t _t = ((real_t)(turnCnt  - turnSolver.ta) / dt);
//     real_t temp = (real_t)dt / 1000 / dp; 

//     real_t yt = 0;

//     if((i == 0) || (i == 2) || (i == 4))
//         yt = CubicInterpolation::forward(
//             Vector2<real_t>{real_t(0.4f), real_t(0.4f) * turnSolver.va * temp}, 
//             Vector2<real_t>(real_t(0.6f), real_t(1.0f) - real_t(0.4f)  * turnSolver.vb * temp), _t);
//     else
//         yt = _t;

//     real_t new_pos =  real_t(turns) + turnSolver.pa + dp * yt;

//     return new_pos;
// }
