#pragma once

#include "../canopen_primitive/canopen_sdo_primitive.hpp"
#include "canopen_msg_serde.hpp"
#include "core/utils/bits/bits_caster.hpp"


namespace ymd::canopen::sdo_msgs{
using namespace canopen::primitive;


struct [[nodiscard]] ExpeditedRequest{
    NodeId client_nodeid;
    SdoExpeditedContext context;

    [[nodiscard]] constexpr CanFrame to_can_frame() const {
        return context.to_can_frame(client_nodeid.with_func_code(FunctionCode::ReqSdo));
    }
};

struct [[nodiscard]] ExpeditedResponse{
    NodeId server_nodeid;
    SdoExpeditedContext context;

    [[nodiscard]] constexpr CanFrame to_can_frame() const {
        return context.to_can_frame(server_nodeid.with_func_code(FunctionCode::RespSdo));
    }
};
}

namespace ymd::canopen::msg_serde{


template<>
struct MsgSerde<sdo_msgs::ExpeditedResponse>{
    using Self = sdo_msgs::ExpeditedResponse;
    [[nodiscard]] static constexpr CanFrame to_can_frame(const Self & self){
        return self.to_can_frame();
    }

    template<VerifyLevel verify_level>
    [[nodiscard]] static constexpr auto from_can_frame(const CanFrame& frame)
    -> FLEX_OPTION(Self){
        FLEX_EXTERNAL_ASSERT_NONE(frame.is_standard());
        FLEX_EXTERNAL_ASSERT_NONE(frame.length() == 8);

        const auto cobid = CobId(frame.identifier().to_stdid());
        
        // 验证这是 RxSDO (0x580 + NodeID)
        FLEX_EXTERNAL_ASSERT_NONE(cobid.func_code().is_resp_sdo());

        const auto self = Self{
            .server_nodeid = cobid.node_id(),
            .context = SdoExpeditedContext::from_u64(frame.payload_u64())
        };
        FLEX_RETURN_SOME(self);
    }
};

template<>
struct MsgSerde<sdo_msgs::ExpeditedRequest>{
    using Self = sdo_msgs::ExpeditedRequest;
    [[nodiscard]] static constexpr CanFrame to_can_frame(const Self & self){
        return self.to_can_frame();
    }

    template<VerifyLevel verify_level>
    [[nodiscard]] static constexpr auto from_can_frame(const CanFrame& frame)
    -> FLEX_OPTION(Self){
        FLEX_EXTERNAL_ASSERT_NONE(frame.is_standard());
        FLEX_EXTERNAL_ASSERT_NONE(frame.length() == 8);

        const auto cobid = CobId(frame.identifier().to_stdid());

        // 验证这是 TxSDO (0x600 + NodeID)
        FLEX_EXTERNAL_ASSERT_NONE(cobid.func_code().is_req_sdo());

        const auto self = Self{
            .client_nodeid = cobid.node_id(),
            .context = SdoExpeditedContext::from_u64(frame.payload_u64())
        };
        FLEX_RETURN_SOME(self);
    }
};

}