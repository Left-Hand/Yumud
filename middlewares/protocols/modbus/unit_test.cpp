#include "modbus_msgs.hpp"
#include "core/utils/Result.hpp"


using namespace ymd;
using namespace ymd::modbus;

namespace{

struct [[nodiscard]] Serializer final{
    using Error = Infallible;

    std::array<uint8_t, 20> bytes;
    size_t ind = 0;

    constexpr Result<void, Error> push_bytes(std::span<const uint8_t> input_bytes){
        // std::copy_n(input_bytes.begin(), input_bytes.end(), bytes.begin());
        for(size_t i = 0; i < input_bytes.size(); i++){
            bytes[ind + i] = input_bytes[i];
        }
        ind += input_bytes.size();
        return Ok();
    }

    constexpr std::span<const uint8_t> collected_bytes() const {
        return std::span(bytes.data(), ind);
    }

    constexpr size_t length() const {return ind;}

    static constexpr Error make_length_exceed_error(){return Error{};}
};


[[maybe_unused]] void test_ser_funcs(){
    
    // ==================== RTU 测试用例 ====================
    
    {
        // [RTU] 读保持寄存器 (0x03) - 地址0，读取2个寄存器
        constexpr uint8_t nodeid = 1;
        constexpr auto serializer = []{
            auto ret = Serializer{};
            const auto msg = req_msg::ReadHoldingRegisters{
                .base_addr = 0x0000,
                .quantity = 0x02,
            };
            serialize_rtu_msg(ret, msg, nodeid).unwrap();
            return ret;
        }();
        // 预期: 01 03 00 00 00 02 C4 0B
        static_assert(serializer.length() == 8);
        static_assert(serializer.bytes[0] == 0x01);  // 设备地址
        static_assert(serializer.bytes[1] == 0x03);  // 功能码
        static_assert(serializer.bytes[2] == 0x00);  // 起始地址高字节
        static_assert(serializer.bytes[3] == 0x00);  // 起始地址低字节
        static_assert(serializer.bytes[4] == 0x00);  // 寄存器数量高字节
        static_assert(serializer.bytes[5] == 0x02);  // 寄存器数量低字节
        static_assert(serializer.bytes[6] == 0xC4);  // CRC 低字节
        static_assert(serializer.bytes[7] == 0x0B);  // CRC 高字节
    }

    {
        // [RTU] 读保持寄存器 (0x03) - 地址100，读取1个寄存器
        constexpr uint8_t nodeid = 1;
        constexpr auto serializer = []{
            auto ret = Serializer{};
            const auto msg = req_msg::ReadHoldingRegisters{
                .base_addr = 100,
                .quantity = 0x01,
            };
            serialize_rtu_msg(ret, msg, nodeid).unwrap();
            return ret;
        }();
        // 预期: 01 03 00 64 00 01 CRC  (CRC需要计算)
        static_assert(serializer.length() == 8);
        static_assert(serializer.bytes[0] == 0x01);
        static_assert(serializer.bytes[1] == 0x03);
        static_assert(serializer.bytes[2] == 0x00);
        static_assert(serializer.bytes[3] == 0x64);  // 100 = 0x64
        static_assert(serializer.bytes[4] == 0x00);
        static_assert(serializer.bytes[5] == 0x01);
        // CRC 字节位置: bytes[6], bytes[7]
    }

    {
        // [RTU] 写单个寄存器 (0x06) - 地址2，写入值255
        constexpr uint8_t nodeid = 1;
        constexpr auto serializer = []{
            auto ret = Serializer{};
            const auto msg = req_msg::WriteSingleHoldingRegister{
                .reg_addr = 0x0002,
                .reg_value = 0x00FF,
            };
            serialize_rtu_msg(ret, msg, nodeid).unwrap();
            return ret;
        }();
        // 预期: 01 06 00 02 00 FF CRC
        static_assert(serializer.length() == 8);
        static_assert(serializer.bytes[0] == 0x01);
        static_assert(serializer.bytes[1] == 0x06);
        static_assert(serializer.bytes[2] == 0x00);
        static_assert(serializer.bytes[3] == 0x02);
        static_assert(serializer.bytes[4] == 0x00);
        static_assert(serializer.bytes[5] == 0xFF);
    }

    {
        // [RTU] 写多个寄存器 (0x10) - 地址4开始，写入2个寄存器 [0x0001, 0x1234]
        constexpr uint8_t nodeid = 1;
        constexpr auto serializer = []{
            auto ret = Serializer{};
            static constexpr uint16_t values[] = {0x0001, 0x1234};
            const auto msg = req_msg::WriteMultipleRegisters{
                .base_addr = 0x0004,
                .reg_values = std::span(values),
            };
            serialize_rtu_msg(ret, msg, nodeid).unwrap();
            return ret;
        }();
        // 预期: 01 10 00 04 00 02 04 00 01 12 34 CRC
        static_assert(serializer.length() == 13);
        static_assert(serializer.bytes[0] == 0x01);  // 设备地址
        static_assert(serializer.bytes[1] == 0x10);  // 功能码 0x10
        static_assert(serializer.bytes[2] == 0x00);  // 起始地址高字节
        static_assert(serializer.bytes[3] == 0x04);  // 起始地址低字节
        static_assert(serializer.bytes[4] == 0x00);  // 寄存器数量高字节
        static_assert(serializer.bytes[5] == 0x02);  // 寄存器数量低字节
        static_assert(serializer.bytes[6] == 0x04);  // 字节数 (2寄存器 × 2字节)
        static_assert(serializer.bytes[7] == 0x00);  // 数据1高字节
        static_assert(serializer.bytes[8] == 0x01);  // 数据1低字节
        static_assert(serializer.bytes[9] == 0x12);  // 数据2高字节
        static_assert(serializer.bytes[10] == 0x34); // 数据2低字节
        // CRC 在 bytes[11], bytes[12]
    }

    // ==================== TCP 测试用例 ====================

    {
        // [TCP] 读保持寄存器 (0x03) - 事务ID=0，地址0，读取2个寄存器
        constexpr uint8_t unitid = 1;
        constexpr auto serializer = []{
            auto ret = Serializer{};
            const auto msg = req_msg::ReadHoldingRegisters{
                .base_addr = 0x0000,
                .quantity = 0x02,
            };
            serialize_tcp_msg(ret, msg, unitid, 0).unwrap();
            return ret;
        }();
        // 预期: 00 00 00 00 00 06 01 03 00 00 00 02
        static_assert(serializer.length() == 12);
        static_assert(serializer.bytes[0]  == 0x00);  // 事务ID高字节
        static_assert(serializer.bytes[1]  == 0x00);  // 事务ID低字节
        static_assert(serializer.bytes[2]  == 0x00);  // 协议ID高字节
        static_assert(serializer.bytes[3]  == 0x00);  // 协议ID低字节
        static_assert(serializer.bytes[4]  == 0x00);  // 长度高字节
        static_assert(serializer.bytes[5]  == 0x06);  // 长度低字节 (1+1+2+2=6)
        static_assert(serializer.bytes[6]  == 0x01);  // 单元ID
        static_assert(serializer.bytes[7]  == 0x03);  // 功能码
        static_assert(serializer.bytes[8]  == 0x00);  // 起始地址高字节
        static_assert(serializer.bytes[9]  == 0x00);  // 起始地址低字节
        static_assert(serializer.bytes[10] == 0x00);  // 寄存器数量高字节
        static_assert(serializer.bytes[11] == 0x02);  // 寄存器数量低字节
    }

    {
        // [TCP] 读保持寄存器 (0x03) - 事务ID=0x0097，地址0，读取2个寄存器
        constexpr uint8_t unitid = 1;
        constexpr auto serializer = []{
            auto ret = Serializer{};
            const auto msg = req_msg::ReadHoldingRegisters{
                .base_addr = 0x0000,
                .quantity = 0x02,
            };
            serialize_tcp_msg(ret, msg, unitid, 0x0097).unwrap();
            return ret;
        }();
        // 预期: 00 97 00 00 00 06 01 03 00 00 00 02
        static_assert(serializer.length() == 12);
        static_assert(serializer.bytes[0] == 0x00);  // 事务ID高字节
        static_assert(serializer.bytes[1] == 0x97);  // 事务ID低字节
        static_assert(serializer.bytes[2] == 0x00);
        static_assert(serializer.bytes[3] == 0x00);
        static_assert(serializer.bytes[4] == 0x00);
        static_assert(serializer.bytes[5] == 0x06);
        static_assert(serializer.bytes[6] == 0x01);
        static_assert(serializer.bytes[7] == 0x03);
        static_assert(serializer.bytes[8] == 0x00);
        static_assert(serializer.bytes[9] == 0x00);
        static_assert(serializer.bytes[10] == 0x00);
        static_assert(serializer.bytes[11] == 0x02);
    }

    {
        // [TCP] 读保持寄存器 (0x03) - 地址100，读取1个寄存器
        constexpr uint8_t unitid = 1;
        constexpr auto serializer = []{
            auto ret = Serializer{};
            const auto msg = req_msg::ReadHoldingRegisters{
                .base_addr = 100,
                .quantity = 0x01,
            };
            serialize_tcp_msg(ret, msg, unitid, 0).unwrap();
            return ret;
        }();
        // 预期: 00 00 00 00 00 06 01 03 00 64 00 01
        static_assert(serializer.length() == 12);
        static_assert(serializer.bytes[0]  == 0x00);
        static_assert(serializer.bytes[1]  == 0x00);
        static_assert(serializer.bytes[2]  == 0x00);
        static_assert(serializer.bytes[3]  == 0x00);
        static_assert(serializer.bytes[4]  == 0x00);
        static_assert(serializer.bytes[5]  == 0x06);
        static_assert(serializer.bytes[6]  == 0x01);
        static_assert(serializer.bytes[7]  == 0x03);
        static_assert(serializer.bytes[8]  == 0x00);
        static_assert(serializer.bytes[9]  == 0x64);  // 100 = 0x64
        static_assert(serializer.bytes[10] == 0x00);
        static_assert(serializer.bytes[11] == 0x01);
    }

    {
        // [TCP] 写单个寄存器 (0x06) - 地址2，写入值255
        constexpr uint8_t unitid = 1;
        constexpr auto serializer = []{
            auto ret = Serializer{};
            const auto msg = req_msg::WriteSingleHoldingRegister{
                .reg_addr = 0x0002,
                .reg_value = 0x00FF,
            };
            serialize_tcp_msg(ret, msg, unitid, 0x015E).unwrap();
            return ret;
        }();
        // 预期: 01 5E 00 00 00 06 01 06 00 02 00 FF
        static_assert(serializer.length() == 12);
        static_assert(serializer.bytes[0] == 0x01);  // 事务ID高字节
        static_assert(serializer.bytes[1] == 0x5E);  // 事务ID低字节
        static_assert(serializer.bytes[2] == 0x00);
        static_assert(serializer.bytes[3] == 0x00);
        static_assert(serializer.bytes[4] == 0x00);
        static_assert(serializer.bytes[5] == 0x06);
        static_assert(serializer.bytes[6] == 0x01);
        static_assert(serializer.bytes[7] == 0x06);  // 功能码 0x06
        static_assert(serializer.bytes[8] == 0x00);
        static_assert(serializer.bytes[9] == 0x02);  // 寄存器地址
        static_assert(serializer.bytes[10] == 0x00);
        static_assert(serializer.bytes[11] == 0xFF); // 写入值
    }

    {
        // [TCP] 写多个寄存器 (0x10) - 地址0，写入2个寄存器 [0x007B, 0x00EA]
        constexpr uint8_t unitid = 1;
        constexpr auto serializer = []{
            auto ret = Serializer{};
            static constexpr uint16_t values[] = {0x0001, 0x1234};
            const auto msg = req_msg::WriteMultipleRegisters{
                .base_addr = 0x0000,
                .reg_values = std::span(values),
            };
            serialize_tcp_msg(ret, msg, unitid, 0x0268).unwrap();
            return ret;
        }();
        // 预期: 02 68 00 00 ' 00 0B 01 10 ' 00 00 00 02 ' 04 00 7B 00 EA
        static_assert(serializer.length() == 17);
        static_assert(serializer.bytes[0]  == 0x02);  // 事务ID高字节
        static_assert(serializer.bytes[1]  == 0x68);  // 事务ID低字节
        static_assert(serializer.bytes[2]  == 0x00);
        static_assert(serializer.bytes[3]  == 0x00);
        static_assert(serializer.bytes[4]  == 0x00);
        static_assert(serializer.bytes[5]  == 0x0B);  // 长度 (1+1+2+2+1+4=11)
        static_assert(serializer.bytes[6]  == 0x01);
        static_assert(serializer.bytes[7]  == 0x10);  // 功能码 0x10
        static_assert(serializer.bytes[8]  == 0x00);
        static_assert(serializer.bytes[9]  == 0x00);  // 起始地址
        static_assert(serializer.bytes[10] == 0x00);
        static_assert(serializer.bytes[11] == 0x02);  // 寄存器数量
        static_assert(serializer.bytes[12] == 0x04);  // 字节数
        static_assert(serializer.bytes[13] == 0x00);
        static_assert(serializer.bytes[14] == 0x01);  // 值1
        static_assert(serializer.bytes[15] == 0x12);
        static_assert(serializer.bytes[16] == 0x34);  // 值2
    }

    {
        // [TCP] 读线圈 (0x01) - 地址0，读取8个线圈
        constexpr uint8_t unitid = 1;
        constexpr auto serializer = []{
            auto ret = Serializer{};
            const auto msg = req_msg::ReadCoils{
                .base_addr = 0x0000,
                .quantity = 0x08,
            };
            serialize_tcp_msg(ret, msg, unitid, 0x0011).unwrap();
            return ret;
        }();
        // 预期: 00 11 00 00 00 06 01 01 00 00 00 08
        static_assert(serializer.length() == 12);
        static_assert(serializer.bytes[0]  == 0x00);
        static_assert(serializer.bytes[1]  == 0x11);  // 事务ID
        static_assert(serializer.bytes[2]  == 0x00);
        static_assert(serializer.bytes[3]  == 0x00);
        static_assert(serializer.bytes[4]  == 0x00);
        static_assert(serializer.bytes[5]  == 0x06);
        static_assert(serializer.bytes[6]  == 0x01);
        static_assert(serializer.bytes[7]  == 0x01);  // 功能码 0x01
        static_assert(serializer.bytes[8]  == 0x00);
        static_assert(serializer.bytes[9]  == 0x00);
        static_assert(serializer.bytes[10] == 0x00);
        static_assert(serializer.bytes[11] == 0x08);
    }

    {
        // [TCP] 写单个线圈 (0x05) - 地址0，置ON
        constexpr uint8_t unitid = 1;
        constexpr auto serializer = []{
            auto ret = Serializer{};
            const auto msg = req_msg::WriteSingleCoil{
                .coil_addr = 0x0000,
                .coil_enabled = EN,  // ON
            };
            serialize_tcp_msg(ret, msg, unitid, 0x00C2).unwrap();
            return ret;
        }();
        // 预期: 00 C2 00 00 00 06 01 05 00 00 FF 00
        static_assert(serializer.length() == 12);
        static_assert(serializer.bytes[0]  == 0x00);
        static_assert(serializer.bytes[1]  == 0xC2);  // 事务ID
        static_assert(serializer.bytes[2]  == 0x00);
        static_assert(serializer.bytes[3]  == 0x00);
        static_assert(serializer.bytes[4]  == 0x00);
        static_assert(serializer.bytes[5]  == 0x06);
        static_assert(serializer.bytes[6]  == 0x01);
        static_assert(serializer.bytes[7]  == 0x05);  // 功能码 0x05
        static_assert(serializer.bytes[8]  == 0x00);
        static_assert(serializer.bytes[9]  == 0x00);
        static_assert(serializer.bytes[10] == 0xFF);  // FF00 表示 ON
        static_assert(serializer.bytes[11] == 0x00);
    }

    {
        // [TCP] 写单个线圈 (0x05) - 地址0，置OFF
        constexpr uint8_t unitid = 1;
        constexpr auto serializer = []{
            auto ret = Serializer{};
            const auto msg = req_msg::WriteSingleCoil{
                .coil_addr = 0x0000,
                .coil_enabled = DISEN,  // OFF
            };
            serialize_tcp_msg(ret, msg, unitid, 0x00C3).unwrap();
            return ret;
        }();
        // 预期: 00 C3 00 00 00 06 01 05 00 00 00 00
        static_assert(serializer.length() == 12);
        static_assert(serializer.bytes[0]  == 0x00);
        static_assert(serializer.bytes[1]  == 0xC3);
        static_assert(serializer.bytes[2]  == 0x00);
        static_assert(serializer.bytes[3]  == 0x00);
        static_assert(serializer.bytes[4]  == 0x00);
        static_assert(serializer.bytes[5]  == 0x06);
        static_assert(serializer.bytes[6]  == 0x01);
        static_assert(serializer.bytes[7]  == 0x05);
        static_assert(serializer.bytes[8]  == 0x00);
        static_assert(serializer.bytes[9]  == 0x00);
        static_assert(serializer.bytes[10] == 0x00);  // 0000 表示 OFF
        static_assert(serializer.bytes[11] == 0x00);
    }

    // ==================== 读输入寄存器 (0x04) ====================
    
    {
        // [TCP] 读输入寄存器 (0x04) - 地址0，读取2个寄存器
        constexpr uint8_t unitid = 1;
        constexpr auto serializer = []{
            auto ret = Serializer{};
            const auto msg = req_msg::ReadInputRegisters{
                .base_addr = 0x0000,
                .quantity = 0x02,
            };
            serialize_tcp_msg(ret, msg, unitid, 0x0097).unwrap();
            return ret;
        }();
        // 预期: 00 97 00 00 00 06 01 04 00 00 00 02
        static_assert(serializer.length() == 12);
        static_assert(serializer.bytes[0]  == 0x00);
        static_assert(serializer.bytes[1]  == 0x97);
        static_assert(serializer.bytes[2]  == 0x00);
        static_assert(serializer.bytes[3]  == 0x00);
        static_assert(serializer.bytes[4]  == 0x00);
        static_assert(serializer.bytes[5]  == 0x06);
        static_assert(serializer.bytes[6]  == 0x01);
        static_assert(serializer.bytes[7]  == 0x04);  // 功能码 0x04
        static_assert(serializer.bytes[8]  == 0x00);
        static_assert(serializer.bytes[9]  == 0x00);
        static_assert(serializer.bytes[10] == 0x00);
        static_assert(serializer.bytes[11] == 0x02);
    }
}
}