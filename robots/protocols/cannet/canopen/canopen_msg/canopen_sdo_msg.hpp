#pragma once

#include "../canopen_primitive/canopen_sdo_primitive.hpp"
#include "canopen_msg_serde.hpp"
#include "core/utils/bits/bits_caster.hpp"


namespace ymd::canopen::sdo_msg{
using namespace canopen::primitive;


struct [[nodiscard]] ExpeditedRequest{
    NodeId client_nodeid;
    ExpeditedPayload payload;

    [[nodiscard]] constexpr CanMsg to_canmsg() const {
        return payload.to_canmsg(client_nodeid.with_func_code(FunctionCode::ReqSdo));
    }
};

struct [[nodiscard]] ExpeditedResponse{
    NodeId server_nodeid;
    ExpeditedPayload payload;

    [[nodiscard]] constexpr CanMsg to_canmsg() const {
        return payload.to_canmsg(server_nodeid.with_func_code(FunctionCode::RespSdo));
    }
};
}

namespace ymd::canopen::msg_serde{


template<>
struct MsgSerde<sdo_msg::ExpeditedResponse>{
    using Self = sdo_msg::ExpeditedResponse;
    [[nodiscard]] static constexpr CanMsg to_canmsg(const Self & self){
        return self.to_canmsg();
    }

    template<VerifyLevel verify_level>
    [[nodiscard]] static constexpr auto from_canmsg(const CanMsg& msg)
    -> FLEX_OPTION(Self){
        FLEX_EXTERNAL_ASSERT_NONE(msg.is_standard());
        FLEX_EXTERNAL_ASSERT_NONE(msg.length() == 8);

        const auto canid_u32 = msg.id_u32();
        const auto cobid = CobId::from_bits(canid_u32);
        
        // 验证这是 RxSDO (0x580 + NodeID)
        FLEX_EXTERNAL_ASSERT_NONE(cobid.func_code().is_resp_sdo());

        const auto self = Self{
            .server_nodeid = cobid.node_id(),
            .payload = ExpeditedPayload::from_u64(msg.payload_u64())
        };
        FLEX_RETURN_SOME(self);
    }
};

template<>
struct MsgSerde<sdo_msg::ExpeditedRequest>{
    using Self = sdo_msg::ExpeditedRequest;
    [[nodiscard]] static constexpr CanMsg to_canmsg(const Self & self){
        return self.to_canmsg();
    }

    template<VerifyLevel verify_level>
    [[nodiscard]] static constexpr auto from_canmsg(const CanMsg& msg)
    -> FLEX_OPTION(Self){
        FLEX_EXTERNAL_ASSERT_NONE(msg.is_standard());
        FLEX_EXTERNAL_ASSERT_NONE(msg.length() == 8);

        const auto canid_u32 = msg.id_u32();
        const auto cobid = CobId::from_bits(canid_u32);

        // 验证这是 TxSDO (0x600 + NodeID)
        FLEX_EXTERNAL_ASSERT_NONE(cobid.func_code().is_req_sdo());

        const auto self = Self{
            .client_nodeid = cobid.node_id(),
            .payload = ExpeditedPayload::from_u64(msg.payload_u64())
        };
        FLEX_RETURN_SOME(self);
    }
};

}