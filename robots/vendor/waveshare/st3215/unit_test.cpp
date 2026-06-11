#include "st3215_serialize.hpp"
#include "st3215_api_facade.hpp"

using namespace ymd;
using namespace ymd::robots::waveshare::st3215;


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

static constexpr int find_different(
    std::span<const uint8_t> lhs, 
    std::span<const uint8_t> rhs
){
    for(size_t i = 0; i < lhs.size(); ++i){
        if(lhs[i] != rhs[i]){
            return i;
        }
    }
    return -1;
};



#if 1

[[maybe_unused]] static constexpr void test_frame_serialize(){
    static constexpr auto factory = FrameFactory{.state = {.id = {1}}};

    // constexpr auto payload = make_write_data_payload(addr, std::span(paras));
    {
        constexpr auto srz = [&]{
            auto ret = Serializer{};
            factory.ping().serialize(ret).unwrap();
            return ret;
        }();

        // 预期: FF FF 01 01 01 Fb
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

        // 预期: 0XFF 0XFF 0X01 0X04 0X02 0X38 0X02 0XBE
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

        // 预期: 0XFF 0XFF 0X01 0X04 0X02 0X38 0X02 0XBE
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

}
#endif

}