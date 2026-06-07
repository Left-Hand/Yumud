#pragma once

#include "../canopen_primitive/canopen_sdo_primitive.hpp"
#include "canopen_msg_serde.hpp"


namespace ymd::canopen::sdo_msgs{


using namespace canopen::primitive;


struct [[nodiscard]] ExpeditedRequest final{
    using Self = ExpeditedRequest;

    NodeId client_node_id;
    SdoExpeditedContext context;

    [[nodiscard]] constexpr CanFrame to_can_frame() const noexcept {
        return context.to_can_frame(client_node_id.to_cobid(FunctionCode::ReqSdo));
    }

    template<VerifyLevel VERIFY_LEVEL>
    [[nodiscard]] static constexpr auto from_can_frame(const CanFrame& frame)
    -> CANOPEN_FLEX_OPTION(Self){
        CANOPEN_EXTERNAL_RETURN_NONE_IFFALSE(frame.is_standard());
        CANOPEN_EXTERNAL_RETURN_NONE_IFFALSE(frame.length() == 8);

        const auto cobid = CobId(frame.identifier().to_stdid());

        // 验证这是 TxSDO (0x600 + NodeID)
        CANOPEN_EXTERNAL_RETURN_NONE_IFFALSE(cobid.func_code().is_sdo_request());

        const auto self = Self{
            .client_node_id = cobid.node_id(),
            .context = SdoExpeditedContext::from_u64(frame.payload_u64())
        };
        CANOPEN_RETURN_SOME(self);
    }
};

struct [[nodiscard]] ExpeditedResponse final{
    using Self = ExpeditedResponse;

    NodeId server_node_id;
    SdoExpeditedContext context;

    [[nodiscard]] constexpr CanFrame to_can_frame() const noexcept {
        return context.to_can_frame(server_node_id.to_cobid(FunctionCode::RespSdo));
    }


    template<VerifyLevel VERIFY_LEVEL>
    [[nodiscard]] static constexpr auto from_can_frame(const CanFrame& frame)
    -> CANOPEN_FLEX_OPTION(Self){
        CANOPEN_EXTERNAL_RETURN_NONE_IFFALSE(frame.is_standard());
        CANOPEN_EXTERNAL_RETURN_NONE_IFFALSE(frame.length() == 8);

        const auto cobid = CobId(frame.identifier().to_stdid());
        
        // 验证这是 RxSDO (0x580 + NodeID)
        CANOPEN_EXTERNAL_RETURN_NONE_IFFALSE(cobid.func_code().is_sdo_response());

        const auto self = Self{
            .server_node_id = cobid.node_id(),
            .context = SdoExpeditedContext::from_u64(frame.payload_u64())
        };
        CANOPEN_RETURN_SOME(self);
    }
};
}