#include "tamagawa_utils.hpp"
#include "tamagawa_primitive.hpp"
#include "tamagawa_msgs.hpp"
#include "tamagawa_serialize.hpp"

#include "core/utils/Result.hpp"
#include <atomic>
#include <memory>

using namespace ymd;
using namespace ymd::drivers::tamagawa;


namespace {

struct Serializer{
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

    constexpr size_t length() const noexcept {
        return ind;
    }

    constexpr std::span<const uint8_t> collected_bytes() const noexcept {
        return std::span<const uint8_t>(bytes.data(), ind);
    }
};

[[maybe_unused]] static void test_ser_request(){

    // Test WriteEEprom request (0x32 01 69)
    {
        constexpr auto serializer = []{
            auto recv = Serializer{};

            constexpr auto req = req_msgs::WriteEEprom{
                .address = 0x01,
                .val = 0x69
            };
            serialize_msg(recv, req).unwrap();
            return recv;
        }();

        static_assert(serializer.length() == 0x04);
        static_assert(serializer.bytes[0] == 0x32);  // CF code
        static_assert(serializer.bytes[1] == 0x01);  // Address
        static_assert(serializer.bytes[2] == 0x69);  // Value
        // Last byte is CRC which should be calculated
        static_assert(serializer.bytes[3] == 0x5a);  // CRC for [0x32, 0x01, 0x69]
    }


    // Test GetAbs request (0x02)
    {
        constexpr auto serializer = []{
            auto recv = Serializer{};

            constexpr auto req = req_msgs::GetAbs{};
            serialize_msg(recv, req).unwrap();
            return recv;
        }();

        static_assert(serializer.length() == 0x01);
        static_assert(serializer.bytes[0] == 0x02);
    }

    // Test ReadEEprom request (0xEA 01)
    {
        constexpr auto serializer = []{
            auto recv = Serializer{};

            constexpr auto req = req_msgs::ReadEEprom{
                .address = 0x01
            };
            serialize_msg(recv, req).unwrap();
            return recv;
        }();

        static_assert(serializer.length() == 0x03);
        static_assert(serializer.bytes[0] == 0xea);  // CF code
        static_assert(serializer.bytes[1] == 0x01);  // Address
        // Last byte is CRC which should be calculated
        static_assert(serializer.bytes[2] == 0xeb);  // CRC for [0xea, 0x01]
    }

    // Test GetAllInfo request (0x1A)
    {
        constexpr auto serializer = []{
            auto recv = Serializer{};

            constexpr auto req = req_msgs::GetAllInfo{};
            serialize_msg(recv, req).unwrap();
            return recv;
        }();

        static_assert(serializer.length() == 0x01);
        static_assert(serializer.bytes[0] == 0x1a);
    }

    // Test GetAbm request (0x8A)
    {
        constexpr auto serializer = []{
            auto recv = Serializer{};

            constexpr auto req = req_msgs::GetAbm{};
            serialize_msg(recv, req).unwrap();
            return recv;
        }();

        static_assert(serializer.length() == 0x01);
        static_assert(serializer.bytes[0] == 0x8a);
    }

    // Test GetVersion request (0x92)
    {
        constexpr auto serializer = []{
            auto recv = Serializer{};

            constexpr auto req = req_msgs::GetVersion{};
            serialize_msg(recv, req).unwrap();
            return recv;
        }();

        static_assert(serializer.length() == 0x01);
        static_assert(serializer.bytes[0] == 0x92);
    }

    // Test ClearAbmAndFault request (0x62)
    {
        constexpr auto serializer = []{
            auto recv = Serializer{};

            constexpr auto req = req_msgs::ClearAbmAndFault{};
            serialize_msg(recv, req).unwrap();
            return recv;
        }();

        static_assert(serializer.length() == 0x01);
        static_assert(serializer.bytes[0] == 0x62);
    }

    // Test ClearAbs request (0xC2)
    {
        constexpr auto serializer = []{
            auto recv = Serializer{};

            constexpr auto req = req_msgs::ClearAbs{};
            serialize_msg(recv, req).unwrap();
            return recv;
        }();

        static_assert(serializer.length() == 0x01);
        static_assert(serializer.bytes[0] == 0xc2);
    }
}

template<size_t N>
static constexpr std::span<const uint8_t, N - 2> 
noheadtail(std::span<const uint8_t, N> in){
    return std::span<const uint8_t, N - 2>(std::next(in.begin()), std::prev(in.end()));
}



[[maybe_unused]] static void test_ser_response(){
    // Test GetAbs response: 02 00 10 78 14 7E
    {
        constexpr uint8_t received_bytes[] = {0x02, 0x00, 0x10, 0x78, 0x14, 0x7e};
        static_assert(verify_checksum(std::span(received_bytes)).is_ok());
        constexpr auto msg = resp_msgs::GetAbs::from_bytes(noheadtail(std::span{received_bytes}));
        static_assert(msg.sf.is_none());
        // Check the ABS values: 0x10, 0x78, 0x14 -> little endian: 0x147810
        static_assert(msg.abs.bytes[0] == 0x10);
        static_assert(msg.abs.bytes[1] == 0x78);
        static_assert(msg.abs.bytes[2] == 0x14);
    }

    // Test GetAbm response: 8A 00 00 00 00 8A
    {
        constexpr uint8_t received_bytes[] = {0x8a, 0x00, 0x00, 0x00, 0x00, 0x8a};
        static_assert(verify_checksum(std::span(received_bytes)).is_ok());
        constexpr auto msg = resp_msgs::GetAbm::from_bytes(noheadtail(std::span{received_bytes}));
        static_assert(msg.sf.is_none());
        // Check the ABM values: 0x00, 0x00, 0x00 -> all zeros
        static_assert(msg.abm.bytes[0] == 0x00);
        static_assert(msg.abm.bytes[1] == 0x00);
        static_assert(msg.abm.bytes[2] == 0x00);
    }

    // Test GetAllInfo response: 1A 00 7A 6D 0F 0A 00 00 00 00 08
    {
        constexpr uint8_t received_bytes[] = {
            0x1a, 0x00, 0x7a, 0x6d, 0x0f, 
            0x0a, 0x00, 0x00, 0x00, 0x00,
            0x08
        };

        static_assert(verify_checksum(std::span(received_bytes)).is_ok());
        constexpr auto msg = resp_msgs::GetAllInfo::from_bytes(noheadtail(std::span{received_bytes}));
        static_assert(msg.sf.is_none());
        // Check the ABS values: 0x7a, 0x6d, 0x0f -> little endian: 0x0f6d7a
        static_assert(msg.abs.bytes[0] == 0x7a);
        static_assert(msg.abs.bytes[1] == 0x6d);
        static_assert(msg.abs.bytes[2] == 0x0f);
        // Check ENC_ID: 0x0a
        static_assert(msg.enc_id == 0x0a);
        // Check ABM values: 0x00, 0x00, 0x00 -> all zeros
        static_assert(msg.abm.bytes[0] == 0x00);
        static_assert(msg.abm.bytes[1] == 0x00);
        static_assert(msg.abm.bytes[2] == 0x00);
        // Check ALMC: 0x00
        static_assert(std::bit_cast<uint8_t>(msg.almc) == 0x00);
    }

    // Test GetVersion response: 92 89 13 31 75 96 2A 9D 13 7E
    {
        constexpr uint8_t received_bytes[] = {
            0x92, 0x89, 0x13, 0x31, 0x75, 
            0x96, 0x2A, 0x9D, 0x13, 0x7e
        };

        static_assert(verify_checksum(std::span(received_bytes)).is_ok());
        constexpr auto msg = resp_msgs::GetVersion::from_bytes(noheadtail(std::span{received_bytes}));
        static_assert(msg.encoder_product_code.low == 0x1389);
        static_assert(msg.encoder_product_code.high == 0x7531);
        static_assert(msg.firmware_version.low == 0x2a96);
        static_assert(msg.firmware_version.high == 0x139d);
    }

    // Test ReadEEprom response: EA 01 69 82
    {
        constexpr uint8_t received_bytes[] = {0xea, 0x01, 0x69, 0x82};
        static_assert(verify_checksum(std::span(received_bytes)).is_ok());

        constexpr auto msg = resp_msgs::ReadEEprom::from_bytes(noheadtail(std::span{received_bytes}));
        static_assert(msg.address == 0x01);
        static_assert(msg.val == 0x69);
    }

    // Test ClearAbmAndFault response: 62 00 A9 99 18 4A
    {
        constexpr uint8_t received_bytes[] = {0x62, 0x00, 0xa9, 0x99, 0x18, 0x4a};
        static_assert(verify_checksum(std::span(received_bytes)).is_ok());

        constexpr auto msg = resp_msgs::ClearAbmAndFault::from_bytes(noheadtail(std::span{received_bytes}));
        static_assert(msg.sf.is_none());
        // Check the ABS values: 0xa9, 0x99, 0x18 -> little endian: 0x1899a9
        static_assert(msg.abs.bytes[0] == 0xa9);
        static_assert(msg.abs.bytes[1] == 0x99);
        static_assert(msg.abs.bytes[2] == 0x18);
    }

    // Test WriteEEprom response: 32 01 69 5A
    {
        constexpr uint8_t received_bytes[] = {0x32, 0x01, 0x69, 0x5a};
        static_assert(verify_checksum(std::span(received_bytes)).is_ok());

        constexpr auto msg = resp_msgs::WriteEEprom::from_bytes(noheadtail(std::span{received_bytes}));
        static_assert(msg.address == 0x01);
        static_assert(msg.val == 0x69);
    }
}


[[maybe_unused]] static void test_crc(){
    {
        static constexpr uint8_t CRC_VALUE = ChecksumBuilder::from_default()
            .push_byte(0x8a)
            .finalize();
        static_assert(CRC_VALUE == 0x8a);
    }

    // Test CRC for WriteEEprom request: [0x32, 0x01, 0x69] -> CRC should be 0x5a
    {
        static constexpr uint8_t CRC_VALUE = ChecksumBuilder::from_default()
            .push_byte(0x32)
            .push_byte(0x01)
            .push_byte(0x69)
            .finalize();
        static_assert(CRC_VALUE == 0x5a);
    }

    // Test CRC for ReadEEprom request: [0xea, 0x01] -> CRC should be 0xeb
    {
        static constexpr uint8_t CRC_VALUE = ChecksumBuilder::from_default()
            .push_byte(0xea)
            .push_byte(0x01)
            .finalize();
        static_assert(CRC_VALUE == 0xeb);
    }
}

}