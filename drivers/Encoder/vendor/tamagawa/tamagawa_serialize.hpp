#pragma once

#include "tamagawa_primitive.hpp"
#include "tamagawa_utils.hpp"

namespace ymd::drivers::tamagawa{

template<typename Serializer, typename Msg>
static constexpr Result<void, typename Serializer::Error> 
serialize_msg(
    Serializer & srz,
    const Msg & msg
){

    constexpr CfCode CF_CODE = Msg::CF_CODE;
    constexpr size_t CONTEXT_LENGTH = Msg::CONTEXT_LENGTH;

    //header
    {
        const std::array<uint8_t, 1> buffer = {
            static_cast<uint8_t>(CF_CODE),
        };

        if(const auto res = srz.push_bytes(buffer); 
            res.is_err()) return Err(res.unwrap_err());
    }

    if constexpr(CONTEXT_LENGTH != 0){
        //context
        {
            if(const auto res = msg.serialize_context(srz); 
                res.is_err()) return Err(res.unwrap_err());
        }

        //tail crc
    
        {
            //crc字段为小端序
            const uint8_t checksum = ChecksumBuilder::from_default()
                .push_bytes(srz.collected_bytes())
                .finalize()
            ;
    
            const std::array<uint8_t, 1> buffer = {
                static_cast<uint8_t>(checksum),
            };
    
            if(const auto res = srz.push_bytes(buffer); 
                res.is_err()) return Err(res.unwrap_err());
        }
    }

    return Ok();
}

template<typename Serializer, typename Request>
static constexpr Result<void, typename Serializer::Error> serialize_request(
    Serializer & srz,
    const Request & request
){
    return serialize_msg(srz, request);
}

template<typename Serializer, typename Response>
static constexpr Result<void, typename Serializer::Error> serialize_response(
    Serializer & srz,
    const Response & response
){
    return serialize_msg(srz, response);
}


static constexpr Result<void, void>
verify_checksum(std::span<const uint8_t> in){
    if(in.size() < 2){
        #ifdef _NDEBUG
        return Err();
        #else
        __builtin_trap();
        #endif
    } 

    const auto bytes = std::span<const uint8_t>(in.begin(), std::prev(in.end()));
    const auto recv_checksum = *std::prev(in.end());
    const auto calc_checksum = ChecksumBuilder::from_default()
        .push_bytes(bytes).finalize();
    if(calc_checksum != recv_checksum) return Err();
    return Ok();
}


}