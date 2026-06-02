#include "../modbus_msgs.hpp"


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



[[maybe_unused]] static void test_ser0x01(){

    {
        // [TCP] 读线圈 (0x01) - 地址0，读取8个线圈
        constexpr uint8_t unitid = 1;
        constexpr auto serializer = []{
            auto ret = Serializer{};
            const auto msg = req_msgs::ReadCoils{
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
}


[[maybe_unused]] static void test_ser0x03(){
    // ==================== RTU 测试用例 ====================
    
    {
        // [RTU] 读保持寄存器 (0x03) - 地址0，读取2个寄存器
        constexpr uint8_t nodeid = 1;
        constexpr auto serializer = []{
            auto ret = Serializer{};
            const auto msg = req_msgs::ReadHoldingRegisters{
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
            const auto msg = req_msgs::ReadHoldingRegisters{
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

    // ==================== TCP 测试用例 ====================

    {
        // [TCP] 读保持寄存器 (0x03) - 事务ID=0，地址0，读取2个寄存器
        constexpr uint8_t unitid = 1;
        constexpr auto serializer = []{
            auto ret = Serializer{};
            const auto msg = req_msgs::ReadHoldingRegisters{
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
            const auto msg = req_msgs::ReadHoldingRegisters{
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
            const auto msg = req_msgs::ReadHoldingRegisters{
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
}


[[maybe_unused]] static void test_ser0x04(){



    // ==================== 读输入寄存器 (0x04) ====================
    
    {
        // [TCP] 读输入寄存器 (0x04) - 地址0，读取2个寄存器
        constexpr uint8_t unitid = 1;
        constexpr auto serializer = []{
            auto ret = Serializer{};
            const auto msg = req_msgs::ReadInputRegisters{
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


[[maybe_unused]] static void test_ser0x05(){



    {
        // [TCP] 写单个线圈 (0x05) - 地址0，置ON
        constexpr uint8_t unitid = 1;
        constexpr auto serializer = []{
            auto ret = Serializer{};
            const auto msg = req_msgs::WriteSingleCoil{
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
            const auto msg = req_msgs::WriteSingleCoil{
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
}

[[maybe_unused]] static void test_ser0x06(){
    {
        // [RTU] 写单个寄存器 (0x06) - 地址2，写入值255
        constexpr uint8_t nodeid = 1;
        constexpr auto serializer = []{
            auto ret = Serializer{};
            const auto msg = req_msgs::WriteSingleHoldingRegister{
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
        // [TCP] 写单个寄存器 (0x06) - 地址2，写入值255
        constexpr uint8_t unitid = 1;
        constexpr auto serializer = []{
            auto ret = Serializer{};
            const auto msg = req_msgs::WriteSingleHoldingRegister{
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

}


[[maybe_unused]] static void test_ser0x10(){
    {
        // [RTU] 写多个寄存器 (0x10) - 地址4开始，写入2个寄存器 [0x0001, 0x1234]
        constexpr uint8_t nodeid = 1;
        constexpr auto serializer = []{
            auto ret = Serializer{};
            static constexpr uint16_t values[] = {0x0001, 0x1234};
            const auto msg = req_msgs::WriteMultipleRegisters{
                .base_addr = 0x0004,
                .reg_values = std::span(values),
            };
            serialize_rtu_msg(ret, msg, nodeid).unwrap();
            // serialize_rtu_header(ret, 0, 0).unwrap();
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

    {
        // [TCP] 写多个寄存器 (0x10) - 地址0，写入2个寄存器 [0x007B, 0x00EA]
        constexpr uint8_t unitid = 1;
        constexpr auto serializer = []{
            auto ret = Serializer{};
            static constexpr uint16_t values[] = {0x0001, 0x1234};
            const auto msg = req_msgs::WriteMultipleRegisters{
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
}


[[maybe_unused]] static void test_ser0x11(){
    {
        // [RTU] 报告从机ID - 无数据负载请求
        constexpr uint8_t nodeid = 5;
        constexpr auto serializer = []{
            auto ret = Serializer{};
            const auto msg = req_msgs::ReportSlaveId{};
            serialize_rtu_msg(ret, msg, nodeid).unwrap();
            return ret;
        }();
        // 预期: 05 11 CRC
        static_assert(serializer.length() == 4);
        static_assert(serializer.bytes[0] == 0x05);  // 设备地址
        static_assert(serializer.bytes[1] == 0x11);  // 功能码 0x11
        // CRC 在 bytes[2], bytes[3]
    }

    {
        // [TCP] 报告从机ID
        constexpr uint8_t unitid = 10;
        constexpr auto serializer = []{
            auto ret = Serializer{};
            const auto msg = req_msgs::ReportSlaveId{};
            serialize_tcp_msg(ret, msg, unitid, 0x0001).unwrap();
            return ret;
        }();
        // 预期: 00 01 00 00 00 02 0A 11
        static_assert(serializer.length() == 8);
        static_assert(serializer.bytes[0] == 0x00);
        static_assert(serializer.bytes[1] == 0x01);  // 事务ID
        static_assert(serializer.bytes[2] == 0x00);
        static_assert(serializer.bytes[3] == 0x00);
        static_assert(serializer.bytes[4] == 0x00);
        static_assert(serializer.bytes[5] == 0x02);  // 长度 (1+1=2)
        static_assert(serializer.bytes[6] == 0x0A);  // 单元标识符
        static_assert(serializer.bytes[7] == 0x11);  // 功能码
    }
}


[[maybe_unused]] static void test_ser0x16(){
    {
        // [RTU] 掩码写寄存器 - 地址0x0004，AND掩码0xFF00，OR掩码0x00FF
        constexpr uint8_t nodeid = 1;
        constexpr auto serializer = []{
            auto ret = Serializer{};
            const auto msg = req_msgs::MaskWriteRegister{
                .reg_addr = 0x0004,
                .and_mask = 0xFF00,
                .or_mask = 0x00FF
            };
            serialize_rtu_msg(ret, msg, nodeid).unwrap();
            return ret;
        }();
        // 预期: 01 16 00 04 FF 00 00 FF CRC
        static_assert(serializer.length() == 10);
        static_assert(serializer.bytes[0] == 0x01);  // 设备地址
        static_assert(serializer.bytes[1] == 0x16);  // 功能码 0x16
        static_assert(serializer.bytes[2] == 0x00);  // 寄存器地址高字节
        static_assert(serializer.bytes[3] == 0x04);  // 寄存器地址低字节
        static_assert(serializer.bytes[4] == 0xFF);  // AND掩码高字节
        static_assert(serializer.bytes[5] == 0x00);  // AND掩码低字节
        static_assert(serializer.bytes[6] == 0x00);  // OR掩码高字节
        static_assert(serializer.bytes[7] == 0xFF);  // OR掩码低字节
        // CRC 在 bytes[8], bytes[9]
    }

    {
        // [TCP] 掩码写寄存器 - 地址0x0010，AND掩码0x0F0F，OR掩码0xF0F0
        constexpr uint8_t unitid = 2;
        constexpr auto serializer = []{
            auto ret = Serializer{};
            const auto msg = req_msgs::MaskWriteRegister{
                .reg_addr = 0x0010,
                .and_mask = 0x0F0F,
                .or_mask = 0xF0F0
            };
            serialize_tcp_msg(ret, msg, unitid, 0x1234).unwrap();
            return ret;
        }();
        // 预期: 12 34 00 00 00 09 02 16 00 10 0F 0F F0 F0
        static_assert(serializer.length() == 14);
        static_assert(serializer.bytes[0]  == 0x12);  // 事务ID高字节
        static_assert(serializer.bytes[1]  == 0x34);  // 事务ID低字节
        static_assert(serializer.bytes[2]  == 0x00);
        static_assert(serializer.bytes[3]  == 0x00);
        static_assert(serializer.bytes[4]  == 0x00);
        static_assert(serializer.bytes[5]  == 0x08);  // 长度
        static_assert(serializer.bytes[6]  == 0x02);  // 单元标识符
        static_assert(serializer.bytes[7]  == 0x16);  // 功能码
        static_assert(serializer.bytes[8]  == 0x00);
        static_assert(serializer.bytes[9]  == 0x10);  // 寄存器地址
        static_assert(serializer.bytes[10] == 0x0F);
        static_assert(serializer.bytes[11] == 0x0F);  // AND掩码
        static_assert(serializer.bytes[12] == 0xF0);
        static_assert(serializer.bytes[13] == 0xF0);  // OR掩码
    }
}

[[maybe_unused]] static void test_ser0x17_edge(){
    {
        // [RTU] 读写多个寄存器 - 读0个寄存器（边界情况）
        // 注意：标准Modbus要求读数量至少为1，此用例测试序列化器行为
        constexpr uint8_t nodeid = 1;
        constexpr auto serializer = []{
            auto ret = Serializer{};
            static constexpr uint16_t values[] = {0xAAAA, 0xBBBB};
            const auto msg = req_msgs::ReadWriteRegisters{
                .read_start_addr = 0x1000,
                .read_quantity = 0x0000,  // 读0个寄存器
                .write_start_addr = 0x2000,
                .write_quantity = 2,
                .write_reg_values = std::span(values)
            };
            serialize_rtu_msg(ret, msg, nodeid).unwrap();
            return ret;
        }();
        // 验证序列化是否正确处理读数量为0的情况
        static_assert(serializer.length() == 17);
    }

    {
        // [RTU] 读写多个寄存器 - 写数量为0（仅执行读操作）
        constexpr uint8_t nodeid = 1;
        constexpr auto serializer = []{
            auto ret = Serializer{};
            const auto msg = req_msgs::ReadWriteRegisters{
                .read_start_addr = 0x3000,
                .read_quantity = 0x0005,
                .write_start_addr = 0x4000,
                .write_quantity = 0x0000,
                .write_reg_values = {}  // 空span
            };
            serialize_rtu_msg(ret, msg, nodeid).unwrap();
            return ret;
        }();
        // 预期长度: 8(固定头) + 5
        static_assert(serializer.length() == 13);
    }
}


[[maybe_unused]] static void test_ser0x29(){
    {
        // [RTU] 重启指定从机
        constexpr uint8_t nodeid = 8;
        constexpr auto serializer = []{
            auto ret = Serializer{};
            const auto msg = req_msgs::ResetSlave{};
            serialize_rtu_msg(ret, msg, nodeid).unwrap();
            return ret;
        }();
        // 预期: 08 29 CRC
        static_assert(serializer.length() == 4);
        static_assert(serializer.bytes[0] == 0x08);  // 设备地址
        static_assert(serializer.bytes[1] == 0x29);  // 功能码 0x29
    }

    {
        // [TCP] 重启从机
        constexpr uint8_t unitid = 3;
        constexpr auto serializer = []{
            auto ret = Serializer{};
            const auto msg = req_msgs::ResetSlave{};
            serialize_tcp_msg(ret, msg, unitid, 0x5678).unwrap();
            return ret;
        }();
        // 预期: 56 78 00 00 00 02 03 29
        static_assert(serializer.length() == 8);
        static_assert(serializer.bytes[0] == 0x56);
        static_assert(serializer.bytes[1] == 0x78);  // 事务ID
        static_assert(serializer.bytes[2] == 0x00);
        static_assert(serializer.bytes[3] == 0x00);
        static_assert(serializer.bytes[4] == 0x00);
        static_assert(serializer.bytes[5] == 0x02);  // 长度
        static_assert(serializer.bytes[6] == 0x03);  // 单元标识符
        static_assert(serializer.bytes[7] == 0x29);  // 功能码
    }
}


}