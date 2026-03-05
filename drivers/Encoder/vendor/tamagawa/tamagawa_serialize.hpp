#pragma once

#include "tamagawa_primitive.hpp"
#include "tamagawa_utils.hpp"

namespace ymd::drivers::tamagawa{

using namespace primitive;

template<typename Receiver, typename Msg>
static constexpr Result<void, typename Receiver::Error> serialize_msg(
    Receiver & receiver,
    const Msg & msg
){

    constexpr CfCode CF_CODE = Msg::CF_CODE;
    constexpr size_t CONTEXT_LENGTH = Msg::CONTEXT_LENGTH;

    //header
    {
        const std::array<uint8_t, 1> buffer = {
            static_cast<uint8_t>(CF_CODE),
        };

        if(const auto res = receiver.push_bytes(buffer); 
            res.is_err()) return Err(res.unwrap_err());
    }

    if constexpr(CONTEXT_LENGTH != 0){
        //context
        {
            if(const auto res = msg.serialize_context(receiver); 
                res.is_err()) return Err(res.unwrap_err());
        }

        //tail crc
    
        {
            //crc字段为小端序
            const uint8_t crc =  Crc8XorAccumulator{}
                .push_bytes(receiver.collected_bytes())
                .finalize()
            ;
    
            const std::array<uint8_t, 1> buffer = {
                static_cast<uint8_t>(crc),
            };
    
            if(const auto res = receiver.push_bytes(buffer); 
                res.is_err()) return Err(res.unwrap_err());
        }
    }

    return Ok();
}

template<typename Receiver, typename Request>
static constexpr Result<void, typename Receiver::Error> serialize_request(
    Receiver & receiver,
    const Request & request
){
    return serialize_msg(receiver, request);
}

template<typename Receiver, typename Response>
static constexpr Result<void, typename Receiver::Error> serialize_response(
    Receiver & receiver,
    const Response & response
){
    return serialize_msg(receiver, response);
}


}