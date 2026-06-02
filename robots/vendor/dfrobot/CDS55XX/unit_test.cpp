#include "cds55xx.hpp"

using namespace ymd;
using namespace ymd::robots::dfrobot::cds55xx;


namespace{


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



[[maybe_unused]] static constexpr void test_frame_serialize(){


    // constexpr auto payload = make_write_data_payload(addr, std::span(paras));

    {
        constexpr auto id = ServoId(0xfe);
        constexpr auto addr = 0x03;

        constexpr auto paras = std::array<uint8_t,1>{0x01};
        constexpr auto frame = FrameFactory{id}.write_data( addr, std::span(paras));
        constexpr auto expected_frame = std::to_array<uint8_t>({
            0xff, 0xff, 0xfe, 0x04, 0x03, 0x03, 0x01, 0xF6
        });

        static_assert(find_different(std::span(frame), std::span(expected_frame)) < 0);

        // static_assert(frame[0] == 0xFF);
        // static_assert(frame[1] == 0xFF);
        // static_assert(frame[2] == 0xFe);
        // static_assert(frame[3] == 0x04);
        // static_assert(frame[4] == 0x03);
        // static_assert(frame[5] == 0x03);
        // static_assert(frame[6] == 0x01);
        // static_assert(frame[7] == 0xF6);
    }

    {
        constexpr auto id = ServoId(0x01);
        constexpr auto frame = FrameFactory{id}.read_data(0x2b, 0x01);
        constexpr auto expected_frame = std::to_array<uint8_t>({
            0XFF,0XFF,0X01,0X04,0X02,0X2B,0X01,0XCC
        });

        constexpr auto i = find_different(std::span(frame), std::span(expected_frame));
        static_assert( i< 0);
    }

    {
        constexpr auto id = ServoId(0x01);
        constexpr auto frame = FrameFactory{id}.ping();
        constexpr auto expected_frame = std::to_array<uint8_t>({
            0XFF,0XFF,0X01,0X02,0X01,0XFB
        });
        constexpr auto i = find_different(std::span(frame), std::span(expected_frame));
        static_assert( i< 0);
    }

    {
        constexpr auto id = ServoId(0x00);
        constexpr auto frame = FrameFactory{id}.reset();
        constexpr auto expected_frame = std::to_array<uint8_t>({
            0XFF,0XFF,0X00,0X02,0X06,0XF7
        });
        constexpr auto i = find_different(std::span(frame), std::span(expected_frame));
        static_assert( i< 0);

    }
}

}