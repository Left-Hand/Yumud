#include "../canopen_sdo_msg.hpp"
#include "../flex_assert.hpp"

using namespace ymd;
using namespace ymd::canopen;
using namespace ymd::canopen::primitive;
using namespace ymd::canopen::sdo_msg;

static_assert(sizeof(CobId) == sizeof(uint16_t));
static_assert(PdoOnlyFunctionCode::from_bits(0).is_none());
static_assert(PdoOnlyFunctionCode::from_bits(4).is_some());

[[maybe_unused]] static void test1(){
    static constexpr auto layout1 = ExpeditedPayload::from_exception(
        OdIndex::from_parts(
            OdPreIndex::from_bits(0),
            OdSubIndex::from_bits(0)
        ),
        SdoAbortCode::DataTransferOrStorageFailed
    );

    static_assert(layout1.header.to_bits() == 0x80'0000'00);
}



[[maybe_unused]] static void canopen_sdo_expedited_request_static_test() {
    using Msg = ExpeditedRequest;
    
    // 测试写请求
    static constexpr auto write_request = Msg{
        .client_nodeid = NodeId::from_u7(5),
        .payload = ExpeditedPayload::from_write_req<uint32_t>(
            OdIndex{OdPreIndex::from_bits(0x1000), OdSubIndex::from_bits(0)},
            uint32_t(0x12345678u)
        )
    };
    
    static constexpr auto write_canmsg = msg_serde::to_canmsg(write_request);
    static_assert(write_canmsg.is_standard());
    static_assert(write_canmsg.length() == 8);
    static_assert(write_canmsg.id_u32() == 0x605); // 0x600 + 5 (TxSDO for node 5)
    static_assert(CobId(write_canmsg.identifier().to_stdid()).func_code().is_req_sdo());
    static constexpr auto de_write_request = 
        msg_serde::from_canmsg<Msg, VerifyLevel::NoCheck>(write_canmsg);
    static_assert(de_write_request.client_nodeid.to_u7() == 5);
    static constexpr auto payload = ExpeditedPayload::from_read_req(
        OdIndex{OdPreIndex::from_bits(0x1001), OdSubIndex::from_bits(1)});
    // 测试读请求
    static constexpr auto read_request = Msg{
        .client_nodeid = NodeId::from_u7(3),
        .payload = payload
    };
    
    static constexpr auto read_canmsg = msg_serde::to_canmsg(read_request);
    static_assert(read_canmsg.is_standard());
    static_assert(read_canmsg.length() == 8);
    static_assert(read_canmsg.id_u32() == 0x603); // 0x600 + 3 (TxSDO for node 3)
    
    static constexpr auto de_read_req = msg_serde::from_canmsg<Msg, VerifyLevel::NoCheck>(read_canmsg);
    static_assert(de_read_req.client_nodeid.to_u7() == 3);
}




[[maybe_unused]] static void canopen_sdo_expedited_response_static_test() {
    using Msg = ExpeditedResponse;
    
    // 测试写响应
    static constexpr auto write_response = Msg{
        .server_nodeid = NodeId::from_u7(5),
        .payload = ExpeditedPayload::from_write_succeed(
            OdIndex{OdPreIndex::from_bits(0x1000), OdSubIndex::from_bits(0)}
        )
    };
    
    static constexpr auto write_resp_canmsg = msg_serde::to_canmsg(write_response);
    static_assert(write_resp_canmsg.is_standard());
    static_assert(write_resp_canmsg.length() == 8);
    static_assert(write_resp_canmsg.id_u32() == 0x585); // 0x580 + 5 (RxSDO for node 5)
    
    static_assert(msg_serde::from_canmsg<Msg, VerifyLevel::NoCheck>(write_resp_canmsg)
        .server_nodeid == NodeId::from_u7(5));
        
    // 测试读响应
    static constexpr auto read_response = Msg{
        .server_nodeid = NodeId::from_u7(3),
        .payload = ExpeditedPayload::from_read_req(
            OdIndex{OdPreIndex::from_bits(0x1001), OdSubIndex::from_bits(1)}
        )
    };
    
    static constexpr auto read_resp_canmsg = msg_serde::to_canmsg(read_response);
    static_assert(read_resp_canmsg.is_standard());
    static_assert(read_resp_canmsg.length() == 8);
    static_assert(read_resp_canmsg.id_u32() == 0x583); // 0x580 + 3 (RxSDO for node 3)
    static_assert(read_resp_canmsg.payload_u64() != 0); // 确保有效载荷不为0
    
    static_assert(msg_serde::from_canmsg<Msg, VerifyLevel::NoCheck>(read_resp_canmsg)
        .server_nodeid == NodeId::from_u7(3));
}
#if 0


[[maybe_unused]] static void canopen_sdo_payload_static_test() {
    // 测试 ExpeditedPayload 的各种构建方法
    
    // 测试写请求构建
    static constexpr auto write_payload = ExpeditedPayload::from_write_req<uint16_t>(
        OdIndex{OdPreIndex::from_bits(0x1002), OdSubIndex::from_bits(0)},
        0xABCDu
    );
    
    // 测试读请求构建
    static constexpr auto read_payload = ExpeditedPayload::from_read_req<uint8_t>(
        OdIndex{OdPreIndex::from_bits(0x1003), OdSubIndex::from_bits(2)},
        0x00u
    );
    
    // 测试响应构建
    static constexpr auto response_payload = ExpeditedPayload::from_read_succeed(
        OdIndex{OdPreIndex::from_bits(0x1004), OdSubIndex::from_bits(0)}
    );
    
    // 验证大小
    static_assert(sizeof(ExpeditedPayload) == 8);
    
    // 验证转换
    static constexpr auto u64_value = write_payload.to_u64();
    static constexpr auto restored_payload = ExpeditedPayload::from_u64(u64_value);
    static_assert(restored_payload.to_u64() == u64_value);
}


#endif