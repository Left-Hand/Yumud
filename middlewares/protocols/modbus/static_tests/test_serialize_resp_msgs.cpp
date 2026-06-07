#include "../modbus_msgs.hpp"


using namespace ymd;
using namespace ymd::modbus;

namespace{

struct [[nodiscard]] Serializer final{
    using Error = Infallible;

    std::array<uint8_t, 20> bytes;
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

    static constexpr Error make_oom_error(){return Error{};}
};


[[maybe_unused]] static void test_ser_resp_0x01(){

    {
        // [TCP] 读线圈响应 (0x01) - 返回8个线圈状态，每个线圈占一位
        constexpr uint8_t unitid = 1;
        constexpr auto srz = []{
            auto ret = Serializer{};
            static constexpr uint8_t values[] = {0xFF}; // 8个线圈全ON
            const auto msg = resp_msgs::ReadCoils{
                .coil_values = std::span(values),
            };
            serialize_tcp_msg(ret, msg, unitid, 0x0011).unwrap();
            return ret;
        }();
        // 预期: 00 11 00 00 00 04 01 01 01 FF
        static_assert(srz.length() == 10);
        static_assert(srz.bytes[0]  == 0x00);
        static_assert(srz.bytes[1]  == 0x11);  // 事务ID
        static_assert(srz.bytes[2]  == 0x00);
        static_assert(srz.bytes[3]  == 0x00);
        static_assert(srz.bytes[4]  == 0x00);
        static_assert(srz.bytes[5]  == 0x04);  // 长度 (1+1+1+1=4)
        static_assert(srz.bytes[6]  == 0x01);
        static_assert(srz.bytes[7]  == 0x01);  // 功能码 0x01
        static_assert(srz.bytes[8]  == 0x01);  // 字节计数
        static_assert(srz.bytes[9]  == 0xFF);  // 线圈状态
    }

    {
        // [RTU] 读线圈响应 (0x01) - 返回2个线圈状态
        constexpr uint8_t nodeid = 1;
        constexpr auto srz = []{
            auto ret = Serializer{};
            static constexpr uint8_t values[] = {0x03}; // 2个线圈ON
            const auto msg = resp_msgs::ReadCoils{
                .coil_values = std::span(values),
            };
            serialize_rtu_msg(ret, msg, nodeid).unwrap();
            return ret;
        }();
        // 预期: 01 01 01 03 CRC
        static_assert(srz.length() == 6);
        static_assert(srz.bytes[0] == 0x01);  // 设备地址
        static_assert(srz.bytes[1] == 0x01);  // 功能码
        static_assert(srz.bytes[2] == 0x01);  // 字节计数
        static_assert(srz.bytes[3] == 0x03);  // 线圈状态
    }
}


[[maybe_unused]] static void test_ser_resp_0x02(){
    // ==================== 读离散输入响应 (0x02) ====================
    
    {
        // [TCP] 读离散输入响应 (0x02) - 返回4个离散输入状态
        constexpr uint8_t unitid = 1;
        constexpr auto srz = []{
            auto ret = Serializer{};
            static constexpr uint8_t values[] = {0x0F}; // 4个离散输入ON
            const auto msg = resp_msgs::ReadDiscreteInputs{
                .discrete_input_values = std::span(values),
            };
            serialize_tcp_msg(ret, msg, unitid, 0x0012).unwrap();
            return ret;
        }();
        // 预期: 00 12 00 00 00 04 01 02 01 0F
        static_assert(srz.length() == 10);
        static_assert(srz.bytes[0]  == 0x00);
        static_assert(srz.bytes[1]  == 0x12);  // 事务ID
        static_assert(srz.bytes[2]  == 0x00);
        static_assert(srz.bytes[3]  == 0x00);
        static_assert(srz.bytes[4]  == 0x00);
        static_assert(srz.bytes[5]  == 0x04);  // 长度
        static_assert(srz.bytes[6]  == 0x01);
        static_assert(srz.bytes[7]  == 0x02);  // 功能码 0x02
        static_assert(srz.bytes[8]  == 0x01);  // 字节计数
        static_assert(srz.bytes[9]  == 0x0F);  // 离散输入状态
    }

    {
        // [RTU] 读离散输入响应 (0x02) - 返回8个离散输入状态
        constexpr uint8_t nodeid = 1;
        constexpr auto srz = []{
            auto ret = Serializer{};
            static constexpr uint8_t values[] = {0xAA}; // 交替的ON/OFF
            const auto msg = resp_msgs::ReadDiscreteInputs{
                .discrete_input_values = std::span(values),
            };
            serialize_rtu_msg(ret, msg, nodeid).unwrap();
            return ret;
        }();
        // 预期: 01 02 01 AA CRC
        static_assert(srz.length() == 6);
        static_assert(srz.bytes[0] == 0x01);  // 设备地址
        static_assert(srz.bytes[1] == 0x02);  // 功能码
        static_assert(srz.bytes[2] == 0x01);  // 字节计数
        static_assert(srz.bytes[3] == 0xAA);  // 离散输入状态
    }
}


[[maybe_unused]] static void test_ser_resp_0x03(){
    // ==================== 读保持寄存器响应 (0x03) ====================
    
    {
        // [RTU] 读保持寄存器响应 (0x03) - 返回2个寄存器值
        constexpr uint8_t nodeid = 1;
        constexpr auto srz = []{
            auto ret = Serializer{};
            static constexpr uint16_t values[] = {0x000A, 0x0102};
            const auto msg = resp_msgs::ReadHoldingRegisters{
                .reg_values = std::span(values),
            };
            serialize_rtu_msg(ret, msg, nodeid).unwrap();
            return ret;
        }();
        // 预期: 01 03 04 00 0A 01 02 CRC
        static_assert(srz.length() == 9);
        static_assert(srz.bytes[0] == 0x01);  // 设备地址
        static_assert(srz.bytes[1] == 0x03);  // 功能码
        static_assert(srz.bytes[2] == 0x04);  // 字节计数 (2*2=4)
        static_assert(srz.bytes[3] == 0x00);  // 寄存器1高字节
        static_assert(srz.bytes[4] == 0x0A);  // 寄存器1低字节
        static_assert(srz.bytes[5] == 0x01);  // 寄存器2高字节
        static_assert(srz.bytes[6] == 0x02);  // 寄存器2低字节
    }

    {
        // [TCP] 读保持寄存器响应 (0x03) - 返回2个寄存器值
        constexpr uint8_t unitid = 1;
        constexpr auto srz = []{
            auto ret = Serializer{};
            static constexpr uint16_t values[] = {0x000A, 0x0102};
            const auto msg = resp_msgs::ReadHoldingRegisters{
                .reg_values = std::span(values),
            };
            serialize_tcp_msg(ret, msg, unitid, 0).unwrap();
            return ret;
        }();
        // 预期: 00 00 00 00 00 07 01 03 04 00 0A 01 02
        static_assert(srz.length() == 13);
        static_assert(srz.bytes[0]  == 0x00);  // 事务ID高字节
        static_assert(srz.bytes[1]  == 0x00);  // 事务ID低字节
        static_assert(srz.bytes[2]  == 0x00);  // 协议ID高字节
        static_assert(srz.bytes[3]  == 0x00);  // 协议ID低字节
        static_assert(srz.bytes[4]  == 0x00);  // 长度高字节
        static_assert(srz.bytes[5]  == 0x07);  // 长度低字节 (1+1+1+4=7)
        static_assert(srz.bytes[6]  == 0x01);  // 单元ID
        static_assert(srz.bytes[7]  == 0x03);  // 功能码
        static_assert(srz.bytes[8]  == 0x04);  // 字节计数
        static_assert(srz.bytes[9]  == 0x00);  // 寄存器1高字节
        static_assert(srz.bytes[10] == 0x0A);  // 寄存器1低字节
        static_assert(srz.bytes[11] == 0x01);  // 寄存器2高字节
        static_assert(srz.bytes[12] == 0x02);  // 寄存器2低字节
    }

    {
        // [TCP] 读保持寄存器响应 (0x03) - 返回1个寄存器值
        constexpr uint8_t unitid = 1;
        constexpr auto srz = []{
            auto ret = Serializer{};
            static constexpr uint16_t values[] = {0x00FF};
            const auto msg = resp_msgs::ReadHoldingRegisters{
                .reg_values = std::span(values),
            };
            serialize_tcp_msg(ret, msg, unitid, 0x0097).unwrap();
            return ret;
        }();
        // 预期: 00 97 00 00 00 05 01 03 02 00 FF
        static_assert(srz.length() == 11);
        static_assert(srz.bytes[0] == 0x00);  // 事务ID高字节
        static_assert(srz.bytes[1] == 0x97);  // 事务ID低字节
        static_assert(srz.bytes[2] == 0x00);
        static_assert(srz.bytes[3] == 0x00);
        static_assert(srz.bytes[4] == 0x00);
        static_assert(srz.bytes[5] == 0x05);  // 长度 (1+1+1+2=5)
        static_assert(srz.bytes[6] == 0x01);
        static_assert(srz.bytes[7] == 0x03);  // 功能码
        static_assert(srz.bytes[8] == 0x02);  // 字节计数 (1*2=2)
        static_assert(srz.bytes[9] == 0x00);  // 寄存器值高字节
        static_assert(srz.bytes[10] == 0xFF); // 寄存器值低字节
    }
}


[[maybe_unused]] static void test_ser_resp_0x04(){
    // ==================== 读输入寄存器响应 (0x04) ====================
    
    {
        // [TCP] 读输入寄存器响应 (0x04) - 返回2个寄存器值
        constexpr uint8_t unitid = 1;
        constexpr auto srz = []{
            auto ret = Serializer{};
            static constexpr uint16_t values[] = {0x000A, 0x0102};
            const auto msg = resp_msgs::ReadInputRegisters{
                .reg_values = std::span(values),
            };
            serialize_tcp_msg(ret, msg, unitid, 0x0097).unwrap();
            return ret;
        }();
        // 预期: 00 97 00 00 00 07 01 04 04 00 0A 01 02
        static_assert(srz.length() == 13);
        static_assert(srz.bytes[0]  == 0x00);
        static_assert(srz.bytes[1]  == 0x97);
        static_assert(srz.bytes[2]  == 0x00);
        static_assert(srz.bytes[3]  == 0x00);
        static_assert(srz.bytes[4]  == 0x00);
        static_assert(srz.bytes[5]  == 0x07);  // 长度 (1+1+1+4=7)
        static_assert(srz.bytes[6]  == 0x01);
        static_assert(srz.bytes[7]  == 0x04);  // 功能码 0x04
        static_assert(srz.bytes[8]  == 0x04);  // 字节计数
        static_assert(srz.bytes[9]  == 0x00);
        static_assert(srz.bytes[10] == 0x0A);
        static_assert(srz.bytes[11] == 0x01);
        static_assert(srz.bytes[12] == 0x02);
    }

    {
        // [RTU] 读输入寄存器响应 (0x04) - 返回1个寄存器值
        constexpr uint8_t nodeid = 1;
        constexpr auto srz = []{
            auto ret = Serializer{};
            static constexpr uint16_t values[] = {0x05DC}; // 1500 decimal
            const auto msg = resp_msgs::ReadInputRegisters{
                .reg_values = std::span(values),
            };
            serialize_rtu_msg(ret, msg, nodeid).unwrap();
            return ret;
        }();
        // 预期: 01 04 02 05 DC CRC
        static_assert(srz.length() == 7);
        static_assert(srz.bytes[0] == 0x01);  // 设备地址
        static_assert(srz.bytes[1] == 0x04);  // 功能码
        static_assert(srz.bytes[2] == 0x02);  // 字节计数 (1*2=2)
        static_assert(srz.bytes[3] == 0x05);  // 寄存器值高字节
        static_assert(srz.bytes[4] == 0xDC);  // 寄存器值低字节
    }
}


[[maybe_unused]] static void test_ser_resp_0x05(){
    // ==================== 写单个线圈响应 (0x05) ====================

    {
        // [TCP] 写单个线圈响应 (0x05) - 地址0，值ON
        constexpr uint8_t unitid = 1;
        constexpr auto srz = []{
            auto ret = Serializer{};
            const auto msg = resp_msgs::WriteSingleCoil{
                .coil_addr = 0x0000,
                .coil_value = 0xFF00,  // ON
            };
            serialize_tcp_msg(ret, msg, unitid, 0x00C2).unwrap();
            return ret;
        }();
        // 预期: 00 C2 00 00 00 06 01 05 00 00 FF 00
        static_assert(srz.length() == 12);
        static_assert(srz.bytes[0]  == 0x00);
        static_assert(srz.bytes[1]  == 0xC2);  // 事务ID
        static_assert(srz.bytes[2]  == 0x00);
        static_assert(srz.bytes[3]  == 0x00);
        static_assert(srz.bytes[4]  == 0x00);
        static_assert(srz.bytes[5]  == 0x06);  // 长度 (1+1+2+2=6)
        static_assert(srz.bytes[6]  == 0x01);
        static_assert(srz.bytes[7]  == 0x05);  // 功能码 0x05
        static_assert(srz.bytes[8]  == 0x00);
        static_assert(srz.bytes[9]  == 0x00);
        static_assert(srz.bytes[10] == 0xFF);  // FF00 表示 ON
        static_assert(srz.bytes[11] == 0x00);
    }

    {
        // [TCP] 写单个线圈响应 (0x05) - 地址0，值OFF
        constexpr uint8_t unitid = 1;
        constexpr auto srz = []{
            auto ret = Serializer{};
            const auto msg = resp_msgs::WriteSingleCoil{
                .coil_addr = 0x0000,
                .coil_value = 0x0000,  // OFF
            };
            serialize_tcp_msg(ret, msg, unitid, 0x00C3).unwrap();
            return ret;
        }();
        // 预期: 00 C3 00 00 00 06 01 05 00 00 00 00
        static_assert(srz.length() == 12);
        static_assert(srz.bytes[0]  == 0x00);
        static_assert(srz.bytes[1]  == 0xC3);
        static_assert(srz.bytes[2]  == 0x00);
        static_assert(srz.bytes[3]  == 0x00);
        static_assert(srz.bytes[4]  == 0x00);
        static_assert(srz.bytes[5]  == 0x06);
        static_assert(srz.bytes[6]  == 0x01);
        static_assert(srz.bytes[7]  == 0x05);
        static_assert(srz.bytes[8]  == 0x00);
        static_assert(srz.bytes[9]  == 0x00);
        static_assert(srz.bytes[10] == 0x00);  // 0000 表示 OFF
        static_assert(srz.bytes[11] == 0x00);
    }

    {
        // [RTU] 写单个线圈响应 (0x05) - 地址10，值ON
        constexpr uint8_t nodeid = 1;
        constexpr auto srz = []{
            auto ret = Serializer{};
            const auto msg = resp_msgs::WriteSingleCoil{
                .coil_addr = 0x000A,  // 地址10
                .coil_value = 0xFF00,  // ON
            };
            serialize_rtu_msg(ret, msg, nodeid).unwrap();
            return ret;
        }();
        // 预期: 01 05 00 0A FF 00 CRC
        static_assert(srz.length() == 8);
        static_assert(srz.bytes[0] == 0x01);  // 设备地址
        static_assert(srz.bytes[1] == 0x05);  // 功能码
        static_assert(srz.bytes[2] == 0x00);  // 地址高字节
        static_assert(srz.bytes[3] == 0x0A);  // 地址低字节
        static_assert(srz.bytes[4] == 0xFF);  // 值高字节
        static_assert(srz.bytes[5] == 0x00);  // 值低字节
    }
}

[[maybe_unused]] static void test_ser_resp_0x06(){
    // ==================== 写单个寄存器响应 (0x06) ====================

    {
        // [RTU] 写单个寄存器响应 (0x06) - 地址2，值255
        constexpr uint8_t nodeid = 1;
        constexpr auto srz = []{
            auto ret = Serializer{};
            const auto msg = resp_msgs::WriteSingleHoldingRegister{
                .reg_address = 0x0002,
                .reg_value = 0x00FF,
            };
            serialize_rtu_msg(ret, msg, nodeid).unwrap();
            return ret;
        }();
        // 预期: 01 06 00 02 00 FF CRC
        static_assert(srz.length() == 8);
        static_assert(srz.bytes[0] == 0x01);
        static_assert(srz.bytes[1] == 0x06);
        static_assert(srz.bytes[2] == 0x00);
        static_assert(srz.bytes[3] == 0x02);
        static_assert(srz.bytes[4] == 0x00);
        static_assert(srz.bytes[5] == 0xFF);
    }

    {
        // [TCP] 写单个寄存器响应 (0x06) - 地址2，值255
        constexpr uint8_t unitid = 1;
        constexpr auto srz = []{
            auto ret = Serializer{};
            const auto msg = resp_msgs::WriteSingleHoldingRegister{
                .reg_address = 0x0002,
                .reg_value = 0x00FF,
            };
            serialize_tcp_msg(ret, msg, unitid, 0x015E).unwrap();
            return ret;
        }();
        // 预期: 01 5E 00 00 00 06 01 06 00 02 00 FF
        static_assert(srz.length() == 12);
        static_assert(srz.bytes[0] == 0x01);  // 事务ID高字节
        static_assert(srz.bytes[1] == 0x5E);  // 事务ID低字节
        static_assert(srz.bytes[2] == 0x00);
        static_assert(srz.bytes[3] == 0x00);
        static_assert(srz.bytes[4] == 0x00);
        static_assert(srz.bytes[5] == 0x06);
        static_assert(srz.bytes[6] == 0x01);
        static_assert(srz.bytes[7] == 0x06);  // 功能码 0x06
        static_assert(srz.bytes[8] == 0x00);
        static_assert(srz.bytes[9] == 0x02);  // 寄存器地址
        static_assert(srz.bytes[10] == 0x00);
        static_assert(srz.bytes[11] == 0xFF); // 写入值
    }
}


[[maybe_unused]] static void test_ser_resp_0x0f(){
    // ==================== 写多个线圈响应 (0x0f) ====================

    {
        // [RTU] 写多个线圈响应 (0x0f) - 地址4，写入10个线圈
        constexpr uint8_t nodeid = 1;
        constexpr auto srz = []{
            auto ret = Serializer{};
            const auto msg = resp_msgs::WriteMultipleCoils{
                .base_address = 0x0004,
                .quantity = 0x000A,  // 10个线圈
            };
            serialize_rtu_msg(ret, msg, nodeid).unwrap();
            return ret;
        }();
        // 预期: 01 0F 00 04 00 0A CRC
        static_assert(srz.length() == 8);
        static_assert(srz.bytes[0] == 0x01);  // 设备地址
        static_assert(srz.bytes[1] == 0x0F);  // 功能码 0x0F
        static_assert(srz.bytes[2] == 0x00);  // 起始地址高字节
        static_assert(srz.bytes[3] == 0x04);  // 起始地址低字节
        static_assert(srz.bytes[4] == 0x00);  // 数量高字节
        static_assert(srz.bytes[5] == 0x0A);  // 数量低字节
    }

    {
        // [TCP] 写多个线圈响应 (0x0f) - 地址0，写入8个线圈
        constexpr uint8_t unitid = 1;
        constexpr auto srz = []{
            auto ret = Serializer{};
            const auto msg = resp_msgs::WriteMultipleCoils{
                .base_address = 0x0000,
                .quantity = 0x0008,  // 8个线圈
            };
            serialize_tcp_msg(ret, msg, unitid, 0x0269).unwrap();
            return ret;
        }();
        // 预期: 02 69 00 00 00 06 01 0F 00 00 00 08
        static_assert(srz.length() == 12);
        static_assert(srz.bytes[0]  == 0x02);  // 事务ID高字节
        static_assert(srz.bytes[1]  == 0x69);  // 事务ID低字节
        static_assert(srz.bytes[2]  == 0x00);
        static_assert(srz.bytes[3]  == 0x00);
        static_assert(srz.bytes[4]  == 0x00);
        static_assert(srz.bytes[5]  == 0x06);  // 长度 (1+1+2+2=6)
        static_assert(srz.bytes[6]  == 0x01);
        static_assert(srz.bytes[7]  == 0x0F);  // 功能码 0x0F
        static_assert(srz.bytes[8]  == 0x00);
        static_assert(srz.bytes[9]  == 0x00);  // 起始地址
        static_assert(srz.bytes[10] == 0x00);
        static_assert(srz.bytes[11] == 0x08);  // 数量
    }
}


[[maybe_unused]] static void test_ser_resp_0x10(){
    // ==================== 写多个寄存器响应 (0x10) ====================

    {
        // [RTU] 写多个寄存器响应 (0x10) - 地址4，写入2个寄存器
        constexpr uint8_t nodeid = 1;
        constexpr auto srz = []{
            auto ret = Serializer{};
            const auto msg = resp_msgs::WriteMultipleRegisters{
                .base_address = 0x0004,
                .quantity = 0x0002,  // 2个寄存器
            };
            serialize_rtu_msg(ret, msg, nodeid).unwrap();
            return ret;
        }();
        // 预期: 01 10 00 04 00 02 CRC
        static_assert(srz.length() == 8);
        static_assert(srz.bytes[0] == 0x01);  // 设备地址
        static_assert(srz.bytes[1] == 0x10);  // 功能码 0x10
        static_assert(srz.bytes[2] == 0x00);  // 起始地址高字节
        static_assert(srz.bytes[3] == 0x04);  // 起始地址低字节
        static_assert(srz.bytes[4] == 0x00);  // 数量高字节
        static_assert(srz.bytes[5] == 0x02);  // 数量低字节
    }

    {
        // [TCP] 写多个寄存器响应 (0x10) - 地址0，写入2个寄存器
        constexpr uint8_t unitid = 1;
        constexpr auto srz = []{
            auto ret = Serializer{};
            const auto msg = resp_msgs::WriteMultipleRegisters{
                .base_address = 0x0000,
                .quantity = 0x0002,  // 2个寄存器
            };
            serialize_tcp_msg(ret, msg, unitid, 0x0268).unwrap();
            return ret;
        }();
        // 预期: 02 68 00 00 00 06 01 10 00 00 00 02
        static_assert(srz.length() == 12);
        static_assert(srz.bytes[0]  == 0x02);  // 事务ID高字节
        static_assert(srz.bytes[1]  == 0x68);  // 事务ID低字节
        static_assert(srz.bytes[2]  == 0x00);
        static_assert(srz.bytes[3]  == 0x00);
        static_assert(srz.bytes[4]  == 0x00);
        static_assert(srz.bytes[5]  == 0x06);  // 长度 (1+1+2+2=6)
        static_assert(srz.bytes[6]  == 0x01);
        static_assert(srz.bytes[7]  == 0x10);  // 功能码 0x10
        static_assert(srz.bytes[8]  == 0x00);
        static_assert(srz.bytes[9]  == 0x00);  // 起始地址
        static_assert(srz.bytes[10] == 0x00);
        static_assert(srz.bytes[11] == 0x02);  // 数量
    }
}


[[maybe_unused]] static void test_ser_resp_0x11(){
    // ==================== 报告从机ID响应 (0x11) ====================

    {
        // [RTU] 报告从机ID响应 - 返回从机ID信息
        constexpr uint8_t nodeid = 5;
        constexpr auto srz = []{
            auto ret = Serializer{};
            static constexpr uint8_t data[] = {0x01, 0x02, 0x03, 0x04, 0x05};
            const auto msg = resp_msgs::ReportSlaveId{
                .slave_id_data = std::span(data),
            };
            serialize_rtu_msg(ret, msg, nodeid).unwrap();
            return ret;
        }();
        // 预期: 05 11 01 02 03 04 05 CRC
        static_assert(srz.length() == 9);
        static_assert(srz.bytes[0] == 0x05);  // 设备地址
        static_assert(srz.bytes[1] == 0x11);  // 功能码 0x11
        static_assert(srz.bytes[2] == 0x01);  // 从机ID数据
        static_assert(srz.bytes[3] == 0x02);
        static_assert(srz.bytes[4] == 0x03);
        static_assert(srz.bytes[5] == 0x04);
        static_assert(srz.bytes[6] == 0x05);
        static_assert(srz.bytes[7] == 67);
        static_assert(srz.bytes[8] == 250);
    }

    {
        // [TCP] 报告从机ID响应
        constexpr uint8_t unitid = 10;
        constexpr auto srz = []{
            auto ret = Serializer{};
            static constexpr uint8_t data[] = {0x01, 0x02, 0x03};
            const auto msg = resp_msgs::ReportSlaveId{
                .slave_id_data = std::span(data),
            };
            serialize_tcp_msg(ret, msg, unitid, 0x0001).unwrap();
            return ret;
        }();
        // 预期: 00 01 00 00 00 05 0A 11 01 02 03
        static_assert(srz.length() == 11);
        static_assert(srz.bytes[0] == 0x00);
        static_assert(srz.bytes[1] == 0x01);  // 事务ID
        static_assert(srz.bytes[2] == 0x00);
        static_assert(srz.bytes[3] == 0x00);
        static_assert(srz.bytes[4] == 0x00);
        static_assert(srz.bytes[5] == 0x05);  // 长度 (1+1+3=5)
        static_assert(srz.bytes[6] == 0x0A);  // 单元标识符
        static_assert(srz.bytes[7] == 0x11);  // 功能码
        static_assert(srz.bytes[8] == 0x01);  // 数据
        static_assert(srz.bytes[9] == 0x02);
        static_assert(srz.bytes[10] == 0x03);
    }
}


[[maybe_unused]] static void test_ser_resp_0x16(){
    // ==================== 掩码写寄存器响应 (0x16) ====================

    {
        // [RTU] 掩码写寄存器响应 - 地址0x0004，AND掩码0xFF00，OR掩码0x00FF
        constexpr uint8_t nodeid = 1;
        constexpr auto srz = []{
            auto ret = Serializer{};
            const auto msg = resp_msgs::MaskWriteRegister{
                .reg_address = 0x0004,
                .and_mask = 0xFF00,
                .or_mask = 0x00FF
            };
            serialize_rtu_msg(ret, msg, nodeid).unwrap();
            return ret;
        }();
        // 预期: 01 16 00 04 FF 00 00 FF CRC
        static_assert(srz.length() == 10);
        static_assert(srz.bytes[0] == 0x01);  // 设备地址
        static_assert(srz.bytes[1] == 0x16);  // 功能码 0x16
        static_assert(srz.bytes[2] == 0x00);  // 寄存器地址高字节
        static_assert(srz.bytes[3] == 0x04);  // 寄存器地址低字节
        static_assert(srz.bytes[4] == 0xFF);  // AND掩码高字节
        static_assert(srz.bytes[5] == 0x00);  // AND掩码低字节
        static_assert(srz.bytes[6] == 0x00);  // OR掩码高字节
        static_assert(srz.bytes[7] == 0xFF);  // OR掩码低字节
    }

    {
        // [TCP] 掩码写寄存器响应 - 地址0x0010，AND掩码0x0F0F，OR掩码0xF0F0
        constexpr uint8_t unitid = 2;
        constexpr auto srz = []{
            auto ret = Serializer{};
            const auto msg = resp_msgs::MaskWriteRegister{
                .reg_address = 0x0010,
                .and_mask = 0x0F0F,
                .or_mask = 0xF0F0
            };
            serialize_tcp_msg(ret, msg, unitid, 0x1234).unwrap();
            return ret;
        }();
        // 预期: 12 34 00 00 00 08 02 16 00 10 0F 0F F0 F0
        static_assert(srz.length() == 14);
        static_assert(srz.bytes[0]  == 0x12);  // 事务ID高字节
        static_assert(srz.bytes[1]  == 0x34);  // 事务ID低字节
        static_assert(srz.bytes[2]  == 0x00);
        static_assert(srz.bytes[3]  == 0x00);
        static_assert(srz.bytes[4]  == 0x00);
        static_assert(srz.bytes[5]  == 0x08);  // 长度 (1+1+2+2+2=8)
        static_assert(srz.bytes[6]  == 0x02);  // 单元标识符
        static_assert(srz.bytes[7]  == 0x16);  // 功能码
        static_assert(srz.bytes[8]  == 0x00);
        static_assert(srz.bytes[9]  == 0x10);  // 寄存器地址
        static_assert(srz.bytes[10] == 0x0F);
        static_assert(srz.bytes[11] == 0x0F);  // AND掩码
        static_assert(srz.bytes[12] == 0xF0);
        static_assert(srz.bytes[13] == 0xF0);  // OR掩码
    }
}


[[maybe_unused]] static void test_ser_resp_0x17(){
    // ==================== 读写多个寄存器响应 (0x17) ====================

    {
        // [RTU] 读写多个寄存器响应 - 返回2个寄存器值
        constexpr uint8_t nodeid = 1;
        constexpr auto srz = []{
            auto ret = Serializer{};
            static constexpr uint16_t values[] = {0x0001, 0x1234};
            const auto msg = resp_msgs::ReadWriteRegisters{
                .read_reg_values = std::span(values),
            };
            serialize_rtu_msg(ret, msg, nodeid).unwrap();
            return ret;
        }();
        // 预期: 01 17 04 00 01 12 34 CRC
        static_assert(srz.length() == 9);
        static_assert(srz.bytes[0] == 0x01);  // 设备地址
        static_assert(srz.bytes[1] == 0x17);  // 功能码 0x17
        static_assert(srz.bytes[2] == 0x04);  // 字节计数 (2*2=4)
        static_assert(srz.bytes[3] == 0x00);  // 寄存器1高字节
        static_assert(srz.bytes[4] == 0x01);  // 寄存器1低字节
        static_assert(srz.bytes[5] == 0x12);  // 寄存器2高字节
        static_assert(srz.bytes[6] == 0x34);  // 寄存器2低字节
    }

    {
        // [TCP] 读写多个寄存器响应 - 返回3个寄存器值
        constexpr uint8_t unitid = 1;
        constexpr auto srz = []{
            auto ret = Serializer{};
            static constexpr uint16_t values[] = {0x000A, 0x0102, 0xABCD};
            const auto msg = resp_msgs::ReadWriteRegisters{
                .read_reg_values = std::span(values),
            };
            serialize_tcp_msg(ret, msg, unitid, 0x026A).unwrap();
            return ret;
        }();
        // 预期: 02 6A 00 00 00 09 01 17 06 00 0A 01 02 AB CD
        static_assert(srz.length() == 15);
        static_assert(srz.bytes[0]  == 0x02);  // 事务ID高字节
        static_assert(srz.bytes[1]  == 0x6A);  // 事务ID低字节
        static_assert(srz.bytes[2]  == 0x00);
        static_assert(srz.bytes[3]  == 0x00);
        static_assert(srz.bytes[4]  == 0x00);
        static_assert(srz.bytes[5]  == 0x09);  // 长度 (1+1+1+6=9)
        static_assert(srz.bytes[6]  == 0x01);
        static_assert(srz.bytes[7]  == 0x17);  // 功能码 0x17
        static_assert(srz.bytes[8]  == 0x06);  // 字节计数 (3*2=6)
        static_assert(srz.bytes[9]  == 0x00);
        static_assert(srz.bytes[10] == 0x0A);
        static_assert(srz.bytes[11] == 0x01);
        static_assert(srz.bytes[12] == 0x02);
        static_assert(srz.bytes[13] == 0xAB);
        static_assert(srz.bytes[14] == 0xCD);
    }
}

}