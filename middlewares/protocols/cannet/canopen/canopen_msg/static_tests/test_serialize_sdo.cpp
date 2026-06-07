#include "../canopen_nmt_msg.hpp"
#include "../canopen_sdo_msg.hpp"
#include "../flex_assert.hpp"


using namespace ymd;
using namespace ymd::canopen;
using namespace ymd::canopen::nmt_msgs;
using namespace ymd::canopen::sdo_msgs;

namespace {

[[maybe_unused]] static void test_canopen_sdo_expedited_request() {
    using Msg = ExpeditedRequest;
    {
        static constexpr auto err_code = SdoAbortCode(SdoAbortCode::DataTransferOrStorageFailed);
        static constexpr auto context = SdoExpeditedContextFactory::sdo_exception(
            OdIndex::from_num(0, 0),
            err_code
        );

        static_assert(context.header.to_bits() == 0x80'0000'00);
        static_assert(context.bytes_u32() == err_code.to_u32());

        static_assert(SdoExpeditedContextFactory::read_response(OdIndex::from_num(0, 0), std::to_array<uint8_t>({0}))
            .header.cmd_spec().to_u8() == 0x4f);
        static_assert(SdoExpeditedContextFactory::read_response(OdIndex::from_num(0, 0), std::to_array<uint8_t>({0, 0}))
            .header.cmd_spec().to_u8() == 0x4b);
        static_assert(SdoExpeditedContextFactory::read_response(OdIndex::from_num(0, 0), std::to_array<uint8_t>({0, 0, 0}))
            .header.cmd_spec().to_u8() == 0x47);
        static_assert(SdoExpeditedContextFactory::read_response(OdIndex::from_num(0, 0), std::to_array<uint8_t>({0, 0, 0, 0}))
            .header.cmd_spec().to_u8() == 0x43);

        static_assert(SdoExpeditedContextFactory::write_request(OdIndex::from_num(0, 0), std::to_array<uint8_t>({0}))
            .header.cmd_spec().to_u8() == 0x2f);
        static_assert(SdoExpeditedContextFactory::write_request(OdIndex::from_num(0, 0), std::to_array<uint8_t>({0, 0}))
            .header.cmd_spec().to_u8() == 0x2b);
        static_assert(SdoExpeditedContextFactory::write_request(OdIndex::from_num(0, 0), std::to_array<uint8_t>({0, 0, 0}))
            .header.cmd_spec().to_u8() == 0x27);
        static_assert(SdoExpeditedContextFactory::write_request(OdIndex::from_num(0, 0), std::to_array<uint8_t>({0, 0, 0, 0}))
            .header.cmd_spec().to_u8() == 0x23);
        // static_assert()
    }
    // #region
    // 测试写请求
    {
        static constexpr auto write_request = Msg{
            .client_node_id = NodeId::from_u7(5),
            .context = SdoExpeditedContextFactory::write_request<uint32_t>(
                OdIndex{0x1000, 0},
                uint32_t(0x12345678u)
            )
        };
        
        static constexpr auto can_frame = msg_serde::to_can_frame(write_request);
        static_assert(can_frame.is_standard());
        static_assert(can_frame.length() == 8);
        static_assert(can_frame.id_u32() == 0x605); // 0x600 + 5 (TxSDO for node 5)
        static_assert(CobId(can_frame.identifier().to_stdid()).func_code().is_sdo_request());
        static constexpr auto de_write_request = 
            msg_serde::from_can_frame<Msg, VerifyLevel::NoCheck>(can_frame);
        static_assert(de_write_request.client_node_id.to_u7() == 5);
    }
    //#endregion

    //#region
    // 测试读请求
    {
        static constexpr auto context = SdoExpeditedContextFactory::read_request(
            OdIndex{0x1001, 1});

        static constexpr auto read_request = Msg{
            .client_node_id = NodeId::from_u7(3),
            .context = context
        };
        
        static constexpr auto read_can_frame = msg_serde::to_can_frame(read_request);
        static_assert(read_can_frame.is_standard());
        static_assert(read_can_frame.length() == 8);
        static_assert(read_can_frame.id_u32() == 0x603); // 0x600 + 3 (TxSDO for node 3)
        
        static constexpr auto de_read_req = msg_serde::from_can_frame<Msg, VerifyLevel::NoCheck>(read_can_frame);
        static_assert(de_read_req.client_node_id.to_u7() == 3);
    }


    //#endregion
}



[[maybe_unused]] static void test_canopen_sdo_expedited_response() {
    using Msg = ExpeditedResponse;
    
    // 测试写响应
    static constexpr auto write_response = Msg{
        .server_node_id = NodeId::from_u7(5),
        .context = SdoExpeditedContextFactory::write_succeed(
            OdIndex{0x1000, 0}
        )
    };
    
    static constexpr auto write_resp_can_frame = msg_serde::to_can_frame(write_response);
    static_assert(write_resp_can_frame.is_standard());
    static_assert(write_resp_can_frame.length() == 8);
    static_assert(write_resp_can_frame.id_u32() == 0x585); // 0x580 + 5 (RxSDO for node 5)
    
    static_assert(msg_serde::from_can_frame<Msg, VerifyLevel::NoCheck>(write_resp_can_frame)
        .server_node_id == NodeId::from_u7(5));
        
    // 测试读响应
    static constexpr auto read_response = Msg{
        .server_node_id = NodeId::from_u7(3),
        .context = SdoExpeditedContextFactory::read_response<uint32_t>(
            OdIndex{0x1001, 1},
            uint32_t(0x11223344)
        )
    };
    
    static constexpr auto read_resp_can_frame = msg_serde::to_can_frame(read_response);
    static_assert(read_resp_can_frame.is_standard());
    static_assert(read_resp_can_frame.length() == 8);
    static_assert(read_resp_can_frame.id_u32() == 0x583); // 0x580 + 3 (RxSDO for node 3)
    static_assert(read_resp_can_frame.payload_u64() != 0); // 确保有效载荷不为0
    
    static_assert(msg_serde::from_can_frame<Msg, VerifyLevel::NoCheck>(read_resp_can_frame)
        .server_node_id == NodeId::from_u7(3));

    static_assert(msg_serde::from_can_frame<Msg, VerifyLevel::NoCheck>(read_resp_can_frame)
        .context.bytes_u32() == 0x11223344);
}
#if 0


[[maybe_unused]] static void test_canopen_sdo_payload() {
    // 测试 SdoExpeditedContext 的各种构建方法
    
    // 测试写请求构建
    static constexpr auto write_payload = SdoExpeditedContext::from_write_request<uint16_t>(
        OdIndex{OdMajorIndex::from_bits(0x1002), OdMinorIndex::from_bits(0)},
        0xABCDu
    );
    
    // 测试读请求构建
    static constexpr auto read_payload = SdoExpeditedContext::from_read_request<uint8_t>(
        OdIndex{OdMajorIndex::from_bits(0x1003), OdMinorIndex::from_bits(2)},
        0x00u
    );
    
    // 测试响应构建
    static constexpr auto response_payload = SdoExpeditedContext::from_read_succeed(
        OdIndex{OdMajorIndex::from_bits(0x1004), OdMinorIndex::from_bits(0)}
    );
    
    // 验证大小
    static_assert(sizeof(SdoExpeditedContext) == 8);
    
    // 验证转换
    static constexpr auto u64_value = write_payload.to_u64();
    static constexpr auto restored_payload = SdoExpeditedContext::from_u64(u64_value);
    static_assert(restored_payload.to_u64() == u64_value);
}


#endif

}