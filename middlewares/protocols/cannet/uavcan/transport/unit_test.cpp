#include "uavcan_transport_iterator.hpp"

using namespace ymd;
using namespace ymd::uavcan;

namespace{
template<typename Iter>
static constexpr size_t count_iter(Iter && iter){
    size_t count = 0;
    while(iter.has_next()){
        (void)iter.next();
        ++count;
    }
    return count;
}

template<typename Iter>
static constexpr auto get_nth_result(const size_t i, Iter && iter){
    size_t count = 0;
    while(iter.has_next()){
        auto result = iter.next();
        if(count == i){
            return result;
        }
        ++count;
    }
    // 如果索引超出范围，可以返回默认值或触发错误
    __builtin_trap(); // 或者抛出异常
}


[[maybe_unused]] void test_multi_ser(){
    static constexpr auto header = Header::from_bits(0x12);
    static constexpr auto buf = std::to_array<uint8_t, 8>({0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08});

    auto make_iter = [&]{
        return SerializeIteratorSpawner{
            .state = {
                .transfer_id = TransferId{3}
            }
        }.spawn(header, std::span(buf), Signature{0});
    };

    static constexpr auto times = count_iter(make_iter());
    static_assert(times == 2);
    {
        static constexpr hal::BxCanFrame first_frame = get_nth_result(0, make_iter());
        static_assert(first_frame.identifier().to_extid() == header.to_can_id());
        static_assert(first_frame.dlc().length() == 8);
        static_assert(first_frame.at(2) == 0x01);
        static_assert(first_frame.at(3) == 0x02);
        static_assert(first_frame.at(4) == 0x03);
        static_assert(first_frame.at(5) == 0x04);
        static_assert(first_frame.at(6) == 0x05);
        static constexpr TailByte tail_byte = TailByte::from_bits(first_frame.at(7));

        static_assert(tail_byte.transfer_id == 3);
        static_assert(tail_byte.toggle == 0);
        static_assert(tail_byte.is_end_of_transfer == 0);
        static_assert(tail_byte.is_start_of_transfer == 1);

    }

    {
        static constexpr hal::BxCanFrame second_frame = get_nth_result(1, make_iter());
        static_assert(second_frame.identifier().to_extid() == header.to_can_id());
        static_assert(second_frame.dlc().length() == 4);
        
        static_assert(second_frame.at(0) == 0x06);
        static_assert(second_frame.at(1) == 0x07);
        static_assert(second_frame.at(2) == 0x08);
        static constexpr TailByte tail_byte = TailByte::from_bits(second_frame.at(3));

        static_assert(tail_byte.transfer_id == 3);
        static_assert(tail_byte.toggle == 1);
        static_assert(tail_byte.is_end_of_transfer == 1);
        static_assert(tail_byte.is_start_of_transfer == 0);
    }
}

[[maybe_unused]] void test_full_single_frame(){
    static constexpr auto header = Header::from_bits(0x12);
    static constexpr auto buf_7 = std::to_array<uint8_t, 7>(
        {1,2,3,4,5,6,7});

    auto make_iter = [&]{
        return SerializeIteratorSpawner{
            .state = {
                .transfer_id = TransferId{4}
            }
        }.spawn(header, std::span(buf_7), Signature{0});
    };

    static constexpr auto times = count_iter(make_iter());
    static_assert(times == 1); // 7字节数据应为单帧传输
    
    {
        // 单帧 - 数据(7) + 尾字节(1) = 8字节，不需要CRC因为刚好<=7字节数据
        static constexpr hal::BxCanFrame single_frame = get_nth_result(0, make_iter());
        static_assert(single_frame.identifier().to_extid() == header.to_can_id());
        static_assert(single_frame.dlc().length() == 8); // 7 bytes data + 1 byte tail
        
        static_assert(single_frame.at(0) == 1);  // 数据1
        static_assert(single_frame.at(1) == 2);  // 数据2
        static_assert(single_frame.at(2) == 3);  // 数据3
        static_assert(single_frame.at(3) == 4);  // 数据4
        static_assert(single_frame.at(4) == 5);  // 数据5
        static_assert(single_frame.at(5) == 6);  // 数据6
        static_assert(single_frame.at(6) == 7);  // 数据7
        static constexpr TailByte tail_byte = TailByte::from_bits(single_frame.at(7));

        static_assert(tail_byte.transfer_id == 4);
        static_assert(tail_byte.toggle == 0); // 单帧传输
        static_assert(tail_byte.is_end_of_transfer == 1); // 结束帧
        static_assert(tail_byte.is_start_of_transfer == 1); // 开始帧
    }
}

[[maybe_unused]] void test_long_multi_ser(){
    static constexpr auto header = Header::from_bits(0x12);
    static constexpr auto buf_large = std::to_array<uint8_t, 20>(
        {
            1,2,3,4,5,
            6,7,8,9,10,11,12,
            13,14,15,16,17,18,
            19,20});

    auto make_iter = [&]{
        return SerializeIteratorSpawner{
            .state = {
                .transfer_id = TransferId{7}
            }
        }.spawn(header, std::span(buf_large), Signature{0});
    };

    static constexpr auto times = count_iter(make_iter());
    static_assert(times == 4);
    
    {
        // 第一帧 - CRC(2) + 数据(5) + 尾字节(1) = 8字节
        static constexpr hal::BxCanFrame first_frame = get_nth_result(0, make_iter());
        static_assert(first_frame.identifier().to_extid() == header.to_can_id());
        static_assert(first_frame.dlc().length() == 8);
        
        // CRC在前两个字节，后面是数据1-5
        static_assert(first_frame.at(2) == 1);  // 数据1
        static_assert(first_frame.at(3) == 2);  // 数据2
        static_assert(first_frame.at(4) == 3);  // 数据3
        static_assert(first_frame.at(5) == 4);  // 数据4
        static_assert(first_frame.at(6) == 5);  // 数据5
        static constexpr TailByte tail_byte = TailByte::from_bits(first_frame.at(7));

        static_assert(tail_byte.transfer_id == 7);
        static_assert(tail_byte.toggle == 0); 
        static_assert(tail_byte.is_end_of_transfer == 0); 
        static_assert(tail_byte.is_start_of_transfer == 1); 
    }

    {
        // 第二帧 - 数据(6,7,8,9,10,11,12) + 尾字节(1) = 8字节
        static constexpr hal::BxCanFrame second_frame = get_nth_result(1, make_iter());
        static_assert(second_frame.identifier().to_extid() == header.to_can_id());
        static_assert(second_frame.dlc().length() == 8); 
        
        static_assert(second_frame.at(0) == 6);   // 数据6
        static_assert(second_frame.at(1) == 7);   // 数据7
        static_assert(second_frame.at(2) == 8);   // 数据8
        static_assert(second_frame.at(3) == 9);   // 数据9
        static_assert(second_frame.at(4) == 10);  // 数据10
        static_assert(second_frame.at(5) == 11);  // 数据11
        static_assert(second_frame.at(6) == 12);  // 数据12
        static constexpr TailByte tail_byte = TailByte::from_bits(second_frame.at(7));

        static_assert(tail_byte.transfer_id == 7);
        static_assert(tail_byte.toggle == 1); // toggle翻转
        static_assert(tail_byte.is_end_of_transfer == 0); 
        static_assert(tail_byte.is_start_of_transfer == 0); 
    }

    {
        // 第三帧 - 数据(13,14,15,16,17,18) + 尾字节(1) = 7字节
        static constexpr hal::BxCanFrame third_frame = get_nth_result(2, make_iter());
        static_assert(third_frame.identifier().to_extid() == header.to_can_id());
        static_assert(third_frame.dlc().length() == 8); 
        
        static_assert(third_frame.at(0) == 13); // 数据13
        static_assert(third_frame.at(1) == 14); // 数据14
        static_assert(third_frame.at(2) == 15); // 数据15
        static_assert(third_frame.at(3) == 16); // 数据16
        static_assert(third_frame.at(4) == 17); // 数据17
        static_assert(third_frame.at(5) == 18); // 数据18
        static_assert(third_frame.at(6) == 19); // 数据19
        static constexpr TailByte tail_byte = TailByte::from_bits(third_frame.at(7));

        static_assert(tail_byte.transfer_id == 7);
        static_assert(tail_byte.toggle == 0); // toggle再次翻转
        static_assert(tail_byte.is_end_of_transfer == 0); // 还没结束
        static_assert(tail_byte.is_start_of_transfer == 0); 
    }

    {
        // 第四帧 - 数据(20) + 尾字节(1) = 2字节
        static constexpr hal::BxCanFrame fourth_frame = get_nth_result(3, make_iter());
        static_assert(fourth_frame.identifier().to_extid() == header.to_can_id());
        static_assert(fourth_frame.dlc().length() == 2); 
        
        static_assert(fourth_frame.at(0) == 20); // 数据19
        static constexpr TailByte tail_byte = TailByte::from_bits(fourth_frame.at(1));

        static_assert(tail_byte.transfer_id == 7);
        static_assert(tail_byte.toggle == 1); // toggle再次翻转
        static_assert(tail_byte.is_end_of_transfer == 1); // 结束帧
        static_assert(tail_byte.is_start_of_transfer == 0); 
    }
}

}