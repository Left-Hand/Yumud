#pragma once

#include "core/utils/Result.hpp"
#include "core/container/heapless_vector.hpp"
#include "primitive/arithmetic/angular.hpp"
#include "tamagawa_utils.hpp"

//多摩川编码器

// https://blog.csdn.net/qq_28149763/article/details/132718177


// 单圈绝对位置的清除
//     单圈绝对角度清零请求CF=DataID8，即上位机发送0xC2，连续发送10 次，并正常接收到编码器的返回信息时，角度清零

// 多圈圈数的清零
//     多圈圈数清零请求CF=DataIDC，即上位机发送0x62，连续发送10 次，并正常接收到编码器的返回信息时，多圈圈数清零。
// 注：对于多圈编码器，用户如果要把单圈角度和多圈圈数同时reset，必须连续发10 次0xC2，
// 然后再连续发10 次0x62。


namespace ymd::drivers::tamagawa{


namespace primitive{

static constexpr size_t MAX_CONTEXT_SIZE = 8;
enum class [[nodiscard]] Command:uint8_t{
    GetAbsoluteLap = 0x02,
    GetMultiTurns = 0x8a,
    GetEncoderId = 0x92,
    GetFullInfo = 0x1a,
    WriteEEprom = 0x32,
    ReadEEprom = 0xea,
    ClearError = 0xba,
    ResetTurns = 0xc2,
    ClearAll = 0x62
};

static constexpr auto CF_DATA_ID_0 = Command::GetAbsoluteLap;
static constexpr auto CF_DATA_ID_1 = Command::GetMultiTurns;
static constexpr auto CF_DATA_ID_2 = Command::GetEncoderId;
static constexpr auto CF_DATA_ID_3 = Command::GetFullInfo;
static constexpr auto CF_DATA_ID_6 = Command::WriteEEprom;
static constexpr auto CF_DATA_ID_D = Command::ReadEEprom;
static constexpr auto CF_DATA_ID_7 = Command::ClearError;
static constexpr auto CF_DATA_ID_8 = Command::ResetTurns;
static constexpr auto CF_DATA_ID_C = Command::ClearAll;




struct [[nodiscard]] StatusField final{
    using Self = StatusField;
    //delimitier error in request frame
    uint8_t ca1:1;

    //paraity error in request frame
    uint8_t ca2:1;

    //overheat / multiturn err / battery err / batter alarm
    uint8_t ea1:1;

    //counting err
    uint8_t ea2:1;
    uint8_t :4;

    static constexpr Self from_u8(const uint8_t b){
        return std::bit_cast<Self>(b);
    }
    constexpr bool is_none() const {
        return std::bit_cast<uint8_t>(*this) == 0;
    }
};

struct [[nodiscard]] Almc final{
    uint8_t over_speed:1;
    uint8_t full_absolute_status:1;
    uint8_t counting_error:1;
    uint8_t counter_overflow:1;
    uint8_t over_heat:1;
    uint8_t multiturn_error:1;
    uint8_t battery_error:1;
    uint8_t battery_alarm:1;
};

struct [[nodiscard]] AbsolutionData final{
    using Self = AbsolutionData;
    std::array<uint8_t, 3> bytes;

    constexpr uint32_t b24() const {
        uint32_t bits = 0;
        bits |= static_cast<uint32_t>(bytes[0]);
        bits |= static_cast<uint32_t>(bytes[1]) << 8;
        bits |= static_cast<uint32_t>(bytes[2]) << 16;
        return bits;
    }


    constexpr Angular<uq32> to_angle(size_t q) const {
        const size_t shift_cnt = static_cast<size_t>(32u - q);
        const uint32_t bits = static_cast<uint32_t>(static_cast<uint32_t>(b24()) << shift_cnt);
        return Angular<uq32>::from_turns(uq32::from_bits(bits));
    }

    constexpr void fill_bytes(std::span<uint8_t> other_bytes){
        other_bytes[0] = bytes[0];
        other_bytes[1] = bytes[1];
        other_bytes[2] = bytes[2];
    }

    static constexpr Self from_bytes(std::span<const uint8_t> bytes){
        return Self{
            .bytes = {bytes[0], bytes[1], bytes[2]}
        };
    }
};

struct [[nodiscard]] MultiTurnData final{
    using Self = MultiTurnData;
    std::array<uint8_t, 3> bytes;

    constexpr uint32_t to_turns() const {
        uint32_t bits;
        bits |= static_cast<uint32_t>(bytes[0]);
        bits |= static_cast<uint32_t>(bytes[1]) << 8;
        bits |= static_cast<uint32_t>(bytes[2]) << 16;

        return bits;
    }

    constexpr void fill_bytes(std::span<uint8_t> other_bytes){
        other_bytes[0] = bytes[0];
        other_bytes[1] = bytes[1];
        other_bytes[2] = bytes[2];
    }

    static constexpr Self from_bytes(std::span<const uint8_t> bytes){
        return Self{
            .bytes = {bytes[0], bytes[1], bytes[2]}
        };
    }
};

// struct [[nodiscard]] FlatPacket final{
//     Command command;
//     StatusField sf;
//     std::array<uint8_t, MAX_CONTEXT_SIZE> context;
    
//     std::span<uint8_t> as_bytes(){

//     }
//     uint8_t calc_crc(){
//         return utils::calc_crc8()
//     }
// };
}



namespace resp_msgs{
using namespace primitive;
template<Command CMD>
struct ResponseContext;

[[nodiscard]] static constexpr size_t command_to_context_length(const Command cmd){
    switch(cmd){
        case CF_DATA_ID_0: return 3;
        case CF_DATA_ID_1: return 3;
        case CF_DATA_ID_2: return 1;
        case CF_DATA_ID_3: return 8;
        case CF_DATA_ID_6: return 3;
        case CF_DATA_ID_D: return 3;
        case CF_DATA_ID_7: return 3;
        case CF_DATA_ID_8: return 3;
        case CF_DATA_ID_C: return 3;
    }
    __builtin_unreachable();
}

template<>
struct ResponseContext<CF_DATA_ID_0> final{
    AbsolutionData abs_data;

    std::span<uint8_t> as_bytes(){
        return std::span(abs_data.bytes);
    }
};

template<>
struct ResponseContext<CF_DATA_ID_1> final{
    MultiTurnData abm_data;
};

template<>
struct ResponseContext<CF_DATA_ID_2> final{
    uint8_t enc_id;
};

template<>
struct ResponseContext<CF_DATA_ID_3> final{
    AbsolutionData abs_data;
    uint8_t enc_id;
    MultiTurnData abm_data;
    Almc almc;
};

template<>
struct ResponseContext<CF_DATA_ID_7> final{
    AbsolutionData abs_data;
};

template<>
struct ResponseContext<CF_DATA_ID_8> final{
    AbsolutionData abs_data;
};

template<>
struct ResponseContext<CF_DATA_ID_C> final{
    AbsolutionData abs_data;
};

struct Response{
    Command cf;
    StatusField sf;
    union{
        ResponseContext<CF_DATA_ID_0> _0;
        ResponseContext<CF_DATA_ID_1> _1;
        ResponseContext<CF_DATA_ID_2> _2;
        ResponseContext<CF_DATA_ID_3> _3;
        ResponseContext<CF_DATA_ID_7> _7;
        ResponseContext<CF_DATA_ID_8> _8;
        ResponseContext<CF_DATA_ID_C> _c;
        std::array<uint8_t, 10> bytes;
    }context;

    [[nodiscard]] std::span<const uint8_t> as_bytes() const {
        return std::span{reinterpret_cast<const uint8_t *>(this), 13};
    }

    HeaplessVector<uint8_t, 13> serialize_to_bytes() const {
        HeaplessVector<uint8_t, 13> ret;
        ret.push_back(std::bit_cast<uint8_t>(cf));
        ret.push_back(std::bit_cast<uint8_t>(sf));
        const auto length = command_to_context_length(cf);
        for (size_t i = 0; i < length; i++) {
            ret.push_back(context.bytes[i]);
        }
        const auto crc8 = utils::calc_crc8(as_bytes());
        ret.push_back(crc8);
        return ret;
    };
};
}
}