#include "../briter_modbus_frame_factory.hpp"

using namespace ymd;
using namespace ymd::robots::briter;


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
};


[[maybe_unused]] static void test_ser0x01(){
    constexpr auto factory = ModbusFrameFactory{
        {.node_id = 1}
    };

    {
        constexpr auto srz = [&]{
            auto ret = Serializer{};
            factory.heartbeat_modbus(2).serialize(ret).unwrap();
            return ret;
        }();

        // 预期: 01 06 17 70 00 02 0C 64
        static_assert(srz.length() == 8);

        static_assert(srz.bytes[0]  == 0x01);
        static_assert(srz.bytes[1]  == 0x06);
        static_assert(srz.bytes[2]  == 0x17);
        static_assert(srz.bytes[3]  == 0x70);
        static_assert(srz.bytes[4]  == 0x00);
        static_assert(srz.bytes[5]  == 0x02);
        static_assert(srz.bytes[6]  == 0x0C);
        static_assert(srz.bytes[7]  == 0x64);
    }


    {
        constexpr auto srz = [&]{
            auto ret = Serializer{};
            factory.set_current(CurrentCode{100}).serialize(ret).unwrap();
            return ret;
        }();

        // 预期: 01 06 17 72 00 64 2D 8E
        static_assert(srz.length() == 8);

        static_assert(srz.bytes[0]  == 0x01);
        static_assert(srz.bytes[1]  == 0x06);
        static_assert(srz.bytes[2]  == 0x17);
        static_assert(srz.bytes[3]  == 0x72);
        static_assert(srz.bytes[4]  == 0x00);
        static_assert(srz.bytes[5]  == 0x64);
        static_assert(srz.bytes[6]  == 0x2d);
        static_assert(srz.bytes[7]  == 0x8e);
    }

    {
        constexpr auto srz = [&]{
            auto ret = Serializer{};
            factory.set_erpm({-4000}).serialize(ret).unwrap();
            return ret;
        }();

        // 预期: 01 10 17 73 00 02 04 ff ff f0 60 1b 62
        static_assert(srz.length() == 13);

        static_assert(srz.bytes[0]  == 0x01);
        static_assert(srz.bytes[1]  == 0x10);
        static_assert(srz.bytes[2]  == 0x17);
        static_assert(srz.bytes[3]  == 0x73);

        static_assert(srz.bytes[4]  == 0x00);
        static_assert(srz.bytes[5]  == 0x02);
        static_assert(srz.bytes[6]  == 0x04);
        static_assert(srz.bytes[7]  == 0xff);

        static_assert(srz.bytes[8]  == 0xff);
        static_assert(srz.bytes[9]  == 0xf0);
        static_assert(srz.bytes[10]  == 0x60);
        static_assert(srz.bytes[11]  == 0x1b);
        static_assert(srz.bytes[12]  == 0x62);
    }

    {
        constexpr auto srz = [&]{
            auto ret = Serializer{};
            factory.get_duty().serialize(ret).unwrap();
            return ret;
        }();

        // 预期: 01 04 13 8B 00 01 45 64 
        static_assert(srz.length() == 8);

        static_assert(srz.bytes[0]  == 0x01);
        static_assert(srz.bytes[1]  == 0x04);
        static_assert(srz.bytes[2]  == 0x13);
        static_assert(srz.bytes[3]  == 0x8b);

        static_assert(srz.bytes[4]  == 0x00);
        static_assert(srz.bytes[5]  == 0x01);
        static_assert(srz.bytes[6]  == 0x45);
        static_assert(srz.bytes[7]  == 0x64);
    }


    {
        constexpr auto srz = [&]{
            auto ret = Serializer{};
            factory.get_homming_status().serialize(ret).unwrap();
            return ret;
        }();

        // 预期: 01 04 13 94 00 01 74 A2 
        static_assert(srz.length() == 8);

        static_assert(srz.bytes[0]  == 0x01);
        static_assert(srz.bytes[1]  == 0x04);
        static_assert(srz.bytes[2]  == 0x13);
        static_assert(srz.bytes[3]  == 0x94);

        static_assert(srz.bytes[4]  == 0x00);
        static_assert(srz.bytes[5]  == 0x01);
        static_assert(srz.bytes[6]  == 0x74);
        static_assert(srz.bytes[7]  == 0xa2);
    }

    {
        constexpr auto srz = [&]{
            auto ret = Serializer{};
            factory.did_find_z_signal().serialize(ret).unwrap();
            return ret;
        }();

        // 预期: 01 04 13 95 00 01 25 62
        static_assert(srz.length() == 8);

        static_assert(srz.bytes[0]  == 0x01);
        static_assert(srz.bytes[1]  == 0x04);
        static_assert(srz.bytes[2]  == 0x13);
        static_assert(srz.bytes[3]  == 0x95);

        static_assert(srz.bytes[4]  == 0x00);
        static_assert(srz.bytes[5]  == 0x01);
        static_assert(srz.bytes[6]  == 0x25);
        static_assert(srz.bytes[7]  == 0x62);
    }

    {
        constexpr auto srz = [&]{
            auto ret = Serializer{};
            factory.get_position().serialize(ret).unwrap();
            return ret;
        }();

        // 预期: 01 04 13 92 00 02 D4 A2
        static_assert(srz.length() == 8);

        static_assert(srz.bytes[0]  == 0x01);
        static_assert(srz.bytes[1]  == 0x04);
        static_assert(srz.bytes[2]  == 0x13);
        static_assert(srz.bytes[3]  == 0x92);

        static_assert(srz.bytes[4]  == 0x00);
        static_assert(srz.bytes[5]  == 0x02);
        static_assert(srz.bytes[6]  == 0xd4);
        static_assert(srz.bytes[7]  == 0xa2);
    }

    
}


}