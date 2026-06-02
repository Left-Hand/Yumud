#include "../briter_modbus_api_facade.hpp"

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

    constexpr size_t length() const {return ind;}

    static constexpr Error make_length_exceed_error(){return Error{};}
};



[[maybe_unused]] static void test_ser0x01(){
    constexpr auto factory = FrameFactory{
        {.node_id = 1}
    };

    {
        constexpr auto serializer = [&]{
            auto ret = Serializer{};
            factory.heartbeat(2).serialize(ret).unwrap();
            return ret;
        }();

        // 预期: 01 06 17 70 00 02 0C 64
        static_assert(serializer.length() == 8);
        static_assert(serializer.bytes[0]  == 0x01);
        static_assert(serializer.bytes[1]  == 0x06);
        static_assert(serializer.bytes[2]  == 0x17);
        static_assert(serializer.bytes[3]  == 0x70);
        static_assert(serializer.bytes[4]  == 0x00);
        static_assert(serializer.bytes[5]  == 0x02);
        static_assert(serializer.bytes[6]  == 0x0C);
        static_assert(serializer.bytes[7]  == 0x64);
    }


    {
        constexpr auto serializer = [&]{
            auto ret = Serializer{};
            factory.set_current(CurrentCode{100}).serialize(ret).unwrap();
            return ret;
        }();

        // 预期: 01 06 17 72 00 64 2D 8E
        static_assert(serializer.length() == 8);
        static_assert(serializer.bytes[0]  == 0x01);
        static_assert(serializer.bytes[1]  == 0x06);
        static_assert(serializer.bytes[2]  == 0x17);
        static_assert(serializer.bytes[3]  == 0x72);
        static_assert(serializer.bytes[4]  == 0x00);
        static_assert(serializer.bytes[5]  == 0x64);
        static_assert(serializer.bytes[6]  == 0x2d);
        static_assert(serializer.bytes[7]  == 0x8e);
    }

    {
        constexpr auto serializer = [&]{
            auto ret = Serializer{};
            factory.set_erpm({-4000}).serialize(ret).unwrap();
            return ret;
        }();

        // 预期: 01 10 17 73 00 02 04 ff ff f0 60 1b 62
        static_assert(serializer.length() == 13);
        static_assert(serializer.bytes[0]  == 0x01);
        static_assert(serializer.bytes[1]  == 0x10);
        static_assert(serializer.bytes[2]  == 0x17);
        static_assert(serializer.bytes[3]  == 0x73);

        static_assert(serializer.bytes[4]  == 0x00);
        static_assert(serializer.bytes[5]  == 0x02);
        static_assert(serializer.bytes[6]  == 0x04);
        static_assert(serializer.bytes[7]  == 0xff);

        static_assert(serializer.bytes[8]  == 0xff);
        static_assert(serializer.bytes[9]  == 0xf0);
        static_assert(serializer.bytes[10]  == 0x60);
        static_assert(serializer.bytes[11]  == 0x1b);
        static_assert(serializer.bytes[12]  == 0x62);
    }
}


}