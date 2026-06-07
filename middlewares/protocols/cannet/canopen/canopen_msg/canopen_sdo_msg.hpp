#pragma once

#include "../canopen_primitive/canopen_sdo_primitive.hpp"
#include "canopen_msg_serde.hpp"
#include "core/utils/bits/bits_caster.hpp"


namespace ymd::canopen::sdo_msgs{
using namespace canopen::primitive;


struct [[nodiscard]] ExpeditedRequest final{
    NodeId client_nodeid;
    SdoExpeditedContext context;

    [[nodiscard]] constexpr CanFrame to_can_frame() const noexcept {
        return context.to_can_frame(client_nodeid.to_cobid(FunctionCode::ReqSdo));
    }
};

struct [[nodiscard]] ExpeditedResponse final{
    NodeId server_nodeid;
    SdoExpeditedContext context;

    [[nodiscard]] constexpr CanFrame to_can_frame() const noexcept {
        return context.to_can_frame(server_nodeid.to_cobid(FunctionCode::RespSdo));
    }
};
}

namespace ymd::canopen::msg_serde{


template<>
struct MsgSerde<sdo_msgs::ExpeditedResponse>{
    using Self = sdo_msgs::ExpeditedResponse;
    [[nodiscard]] static constexpr CanFrame to_can_frame(const Self & self) noexcept {
        return self.to_can_frame();
    }

    template<VerifyLevel VERIFY_LEVEL>
    [[nodiscard]] static constexpr auto from_can_frame(const CanFrame& frame)
    -> CANOPEN_FLEX_OPTION(Self){
        CANOPEN_FLEX_EXTERNAL_ASSERT_NONE(frame.is_standard());
        CANOPEN_FLEX_EXTERNAL_ASSERT_NONE(frame.length() == 8);

        const auto cobid = CobId(frame.identifier().to_stdid());
        
        // 验证这是 RxSDO (0x580 + NodeID)
        CANOPEN_FLEX_EXTERNAL_ASSERT_NONE(cobid.func_code().is_sdo_response());

        const auto self = Self{
            .server_nodeid = cobid.node_id(),
            .context = SdoExpeditedContext::from_u64(frame.payload_u64())
        };
        CANOPEN_FLEX_RETURN_SOME(self);
    }
};

template<>
struct MsgSerde<sdo_msgs::ExpeditedRequest>{
    using Self = sdo_msgs::ExpeditedRequest;
    [[nodiscard]] static constexpr CanFrame to_can_frame(const Self & self) noexcept {
        return self.to_can_frame();
    }

    template<VerifyLevel VERIFY_LEVEL>
    [[nodiscard]] static constexpr auto from_can_frame(const CanFrame& frame)
    -> CANOPEN_FLEX_OPTION(Self){
        CANOPEN_FLEX_EXTERNAL_ASSERT_NONE(frame.is_standard());
        CANOPEN_FLEX_EXTERNAL_ASSERT_NONE(frame.length() == 8);

        const auto cobid = CobId(frame.identifier().to_stdid());

        // 验证这是 TxSDO (0x600 + NodeID)
        CANOPEN_FLEX_EXTERNAL_ASSERT_NONE(cobid.func_code().is_sdo_request());

        const auto self = Self{
            .client_nodeid = cobid.node_id(),
            .context = SdoExpeditedContext::from_u64(frame.payload_u64())
        };
        CANOPEN_FLEX_RETURN_SOME(self);
    }
};

}