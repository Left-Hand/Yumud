#include "../st3215_api_facade.hpp"

using namespace ymd;
using namespace ymd::robots::waveshare::st3215;


namespace{

struct [[nodiscard]] Serializer final{
    using Error = Infallible;

    std::array<uint8_t, 40> bytes;
    size_t ind = 0;

    constexpr Result<void, Error> push_bytes(std::span<const uint8_t> input_bytes){
        for(size_t i = 0; i < input_bytes.size(); i++){
            bytes[ind + i] = input_bytes[i];
        }
        ind += input_bytes.size();
        return Ok();
    }

    constexpr std::span<const uint8_t> collected_bytes() const {
        return std::span(bytes.data(), ind);
    }

    constexpr Result<void, Error> compatible_with_length(size_t n) const {
        return Ok();
    }

    constexpr uint8_t * take_cursor_and_inc(const size_t n) {
        const size_t next_ind = ind + n;
        auto ptr = bytes.data() + ind;
        ind = next_ind;
        return ptr;
    }

    constexpr size_t length() const {return ind;}
};

[[maybe_unused]] static void test_frame_serialize(){
    static constexpr auto factory = FrameFactory{.state = {.id = {1}}};

    // constexpr auto payload = make_write_data_payload(addr, std::span(paras));
    {
        constexpr auto srz = [&]{
            auto ret = Serializer{};
            factory.ping().serialize(ret).unwrap();
            return ret;
        }();

        // 预期: FF FF 01 02 01 FB
        static_assert(srz.length() == 6);

        static_assert(srz.bytes[0]  == 0xff);
        static_assert(srz.bytes[1]  == 0xff);
        static_assert(srz.bytes[2]  == 0x01);
        static_assert(srz.bytes[3]  == 0x02);
        static_assert(srz.bytes[4]  == 0x01);
        static_assert(srz.bytes[5]  == 0xfb);
    }

    {
        constexpr auto srz = [&]{
            auto ret = Serializer{};
            factory.read_data({.base_addr = 0x38, .len = 0x02}).serialize(ret).unwrap();
            return ret;
        }();

        // 预期: FF FF 01 04 02 38 02 BE
        static_assert(srz.length() == 8);

        static_assert(srz.bytes[0]  == 0xff);
        static_assert(srz.bytes[1]  == 0xff);
        static_assert(srz.bytes[2]  == 0x01);
        static_assert(srz.bytes[3]  == 0x04);
        static_assert(srz.bytes[4]  == 0x02);
        static_assert(srz.bytes[5]  == 0x38);
        static_assert(srz.bytes[6]  == 0x02);
        static_assert(srz.bytes[7]  == 0xbe);
    }

    {
        constexpr auto srz = [&]{
            auto ret = Serializer{};
            factory.read_data({.base_addr = 0x38, .len = 0x02}).serialize(ret).unwrap();
            return ret;
        }();

        // 预期: FF FF 01 04 02 38 02 BE
        static_assert(srz.length() == 8);

        static_assert(srz.bytes[0]  == 0xff);
        static_assert(srz.bytes[1]  == 0xff);
        static_assert(srz.bytes[2]  == 0x01);
        static_assert(srz.bytes[3]  == 0x04);
        static_assert(srz.bytes[4]  == 0x02);
        static_assert(srz.bytes[5]  == 0x38);
        static_assert(srz.bytes[6]  == 0x02);
        static_assert(srz.bytes[7]  == 0xbe);
    }

    {
        static constexpr uint8_t data[] = {0x01};
        constexpr auto srz = [&]{
            auto ret = Serializer{};
            factory.write_data({.base_addr = 0x5, .data = std::span(data)}).serialize(ret).unwrap();
            return ret;
        }();

        static_assert(srz.length() == 8);

        static_assert(srz.bytes[0]  == 0xff);
        static_assert(srz.bytes[1]  == 0xff);
        static_assert(srz.bytes[2]  == 0xfe);
        static_assert(srz.bytes[3]  == 0x04);
        static_assert(srz.bytes[4]  == 0x03);
        static_assert(srz.bytes[5]  == 0x05);
        static_assert(srz.bytes[6]  == 0x01);
        static_assert(srz.bytes[7]  == 0xf4);
    }

    // 测试新的指令类型
    {
        // 测试RegWrite指令
        static constexpr uint8_t data[] = {0x00, 0x08, 0x00, 0x00, 0xE8, 0x03};  // 目标位置2048，速度1000
        constexpr auto srz = [&]{
            auto ret = Serializer{};
            factory.reg_write({.base_addr = 0x2A, .data = std::span(data)}).serialize(ret).unwrap();
            return ret;
        }();

        static_assert(srz.length() == 13);

        static_assert(srz.bytes[0]  == 0xff);
        static_assert(srz.bytes[1]  == 0xff);
        static_assert(srz.bytes[2]  == 0xfe);  // 广播ID
        static_assert(srz.bytes[3]  == 0x09); 
        static_assert(srz.bytes[4]  == 0x04);  // RegWrite指令码
        static_assert(srz.bytes[5]  == 0x2A);  // 地址

        // 数据部分
        static_assert(srz.bytes[6]  == 0x00);
        static_assert(srz.bytes[7]  == 0x08);
        static_assert(srz.bytes[8]  == 0x00);
        static_assert(srz.bytes[9]  == 0x00);

        static_assert(srz.bytes[10]  == 0xe8);
        static_assert(srz.bytes[11]  == 0x03);
        static_assert(srz.bytes[12]  == 0xd7);
    }

    {
        // 测试Action指令
        constexpr auto srz = [&]{
            auto ret = Serializer{};
            factory.action().serialize(ret).unwrap();
            return ret;
        }();

        // 预期: FF FF FE 02 05 FA (广播ID，长度2，指令05，校验和FA)
        static_assert(srz.length() == 6);

        static_assert(srz.bytes[0]  == 0xff);
        static_assert(srz.bytes[1]  == 0xff);
        static_assert(srz.bytes[2]  == 0xfe);  // 广播ID
        static_assert(srz.bytes[3]  == 0x02);  // 长度
        static_assert(srz.bytes[4]  == 0x05);  // Action指令码
        static_assert(srz.bytes[5]  == 0xfa);  // 校验和
    }

    {
        // 测试Reset指令
        constexpr auto srz = [&]{
            auto ret = Serializer{};
            factory.reset().serialize(ret).unwrap();
            return ret;
        }();

        static_assert(srz.length() == 6);

        static_assert(srz.bytes[0]  == 0xff);
        static_assert(srz.bytes[1]  == 0xff);
        static_assert(srz.bytes[2]  == 0x01);  // ID=0
        static_assert(srz.bytes[3]  == 0x02);  // 长度
        static_assert(srz.bytes[4]  == 0x06);  // Reset指令码
        static_assert(srz.bytes[5]  == 0xf6);  // 校验和
    }

    {
        // 测试SyncWrite指令
        static constexpr uint8_t dev_list[] = {
            0x01, 0x00, 0x08, 0x00, 0x00, 0xE8, 0x03,  // ID1 + 数据
            0x02, 0x00, 0x08, 0x00, 0x00, 0xE8, 0x03,  // ID2 + 数据
            0x03, 0x00, 0x08, 0x00, 0x00, 0xE8, 0x03,  // ID3 + 数据
            0x04, 0x00, 0x08, 0x00, 0x00, 0xE8, 0x03,  // ID4 + 数据
        };

        constexpr auto srz = [&]{
            auto ret = Serializer{};
            factory.sync_write({.base_addr = 0x2A, .per_dev_data_len = 0x06, .dev_list = std::span(dev_list)}).serialize(ret).unwrap();
            return ret;
        }();

        static_assert(srz.length() == 36);  // FF FF FE 20 83 2A 06 [ID+数据...] [校验和]
        static_assert(srz.bytes[0]  == 0xff);
        static_assert(srz.bytes[1]  == 0xff);
        static_assert(srz.bytes[2]  == 0xfe);  // 广播ID
        static_assert(srz.bytes[3]  == 0x20);  // 长度 = (1+6)*2 + 4 = 18 = 0x12? 让我们重新计算
        static_assert(srz.bytes[4]  == 0x83);  // SyncWrite指令码
        static_assert(srz.bytes[35]  == 0x58); 

    }

    {
        // 测试SyncRead指令
        static constexpr uint8_t dev_ids[] = {0x01, 0x02};  // 读取ID1和ID2
        constexpr auto srz = [&]{
            auto ret = Serializer{};
            factory.sync_read({.base_addr = 0x38, .read_len = 0x08, .dev_ids = std::span(dev_ids)}).serialize(ret).unwrap();
            return ret;
        }();

        static_assert(srz.length() == 10);  // FF FF FE 06 82 38 08 01 02 36
        static_assert(srz.bytes[0]  == 0xff);
        static_assert(srz.bytes[1]  == 0xff);
        static_assert(srz.bytes[2]  == 0xfe);  // 广播ID
        static_assert(srz.bytes[3]  == 0x06);  // 长度 = 2 + 4 = 6
        static_assert(srz.bytes[4]  == 0x82);  // SyncRead指令码
        static_assert(srz.bytes[5]  == 0x38);  // 地址
        static_assert(srz.bytes[6]  == 0x08);  // 读取长度
        static_assert(srz.bytes[7]  == 0x01);  // 第一个ID
        static_assert(srz.bytes[8]  == 0x02);  // 第一个ID
        static_assert(srz.bytes[9]  == 0x36);  // 第一个ID
    }
}


#if 0
// 添加应答包解析测试
[[maybe_unused]] static constexpr void test_response_parse(){
    // 模拟一个应答包: FF FF 01 04 00 18 05 DD
    // ID=1, 长度=4, 状态=0, 数据=[0x18, 0x05], 校验和=0xDD
    std::array<uint8_t, 8> response_data = {0xFF, 0xFF, 0x01, 0x04, 0x00, 0x18, 0x05, 0xDD};
    std::span<const uint8_t> response_span(response_data.data(), response_data.size());
    
    if(auto parsed = ResponsePacket::parse_from_buffer(response_span); parsed.is_ok()){
        auto packet = parsed.unwrap();
        // 检查解析结果
        static_assert(packet.id.bits == 1);
        static_assert(packet.payload_len == 4);
        static_assert(packet.error_status.bits == 0);
        static_assert(packet.data.size() == 2);
        static_assert(packet.verify_checksum());  // 校验和应该正确
        
        // 解析16位数据
        if(auto value = packet.parse_read_uint16(true); value.is_ok()){
            static_assert(value.unwrap() == 0x0518);  // 低字节在前: 0x18, 0x05 -> 0x0518
        }
    }
}
#endif
}
