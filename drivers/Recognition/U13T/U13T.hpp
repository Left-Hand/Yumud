#pragma once

#include "core/utils/bytes/buffer_cursor.hpp"
#include <span>

namespace ymd::drivers::u13t{


struct [[nodiscard]] Command final{
public:
    enum Kind:uint8_t{
        ReadCardNumber = 0x10,
        ReadIdentityCardNumber = 0x20,
        ReadBlockData = 0x11,
        WriteBlockData = 0x12,
        RegisterCard = 0x13,
        UnregisterCard = 0x14,
        Charge = 0x15,
        Deduct = 0x16,
        LoadKey = 0x2b,
        SetBaudrate = 0x2c,
        SetAddress = 0x2d,
        SetAutoMode = 0x2e,
    };

    constexpr Command(const Kind kind, bool is_rx):
        kind_(kind),
        is_rx_(is_rx){;}

    static constexpr Command from_bits(const uint8_t bits){
        return Command(static_cast<Kind>(bits), bits & 0x80);
    }

    constexpr Kind kind() const noexcept {return kind_;}
    constexpr bool is_rx() const noexcept {return is_rx_;}
private:
    Kind kind_:7;
    bool is_rx_:1;
};

class Error{
    enum Kind:uint8_t{
        VerifyErr = 0xFB,
        BlanceInsufficient = 0xfc,
        DeviceError = 0xfe,
        NoCard = 0xff,
    };
};

enum class [[nodiscard]] Status:uint8_t{
    Ok = 0x00,
    NoCard = 0xff,
    Fault = 0xfe,
    OutOfMoney = 0xfc,
    VerifyFailed = 0xfb,
};

enum class [[nodiscard]] CardType:uint8_t{
    S50 = 0x04,
    Ultralight = 0x44,
};


namespace req_msgs{

struct [[nodiscard]] SetBaudrate{
    static constexpr size_t PAYLOAD_LENGTH = 7;
    uint32_t baudrate;

    constexpr void fill_bytes(__restrict uint8_t buf[PAYLOAD_LENGTH]){
        auto cursor = BufferCursor(buf);
        cursor.push_u32be(baudrate); 
        cursor.push_u8be(0x98); 
        cursor.push_u8be(0x24); 
        cursor.push_u8be(0x31); 
    }
};

struct [[nodiscard]] ReadBlockData{
    static constexpr size_t PAYLOAD_LENGTH = 1;
    uint8_t block_num;

    constexpr void fill_bytes(__restrict uint8_t buf[PAYLOAD_LENGTH]){
        buf[0] = block_num;
    }
};

struct [[nodiscard]] WriteBlockData{
    static constexpr size_t PAYLOAD_LENGTH = 17;
    uint8_t block_num;
    std::span<const uint8_t, 16> block_data;

    constexpr void fill_bytes(__restrict uint8_t buf[PAYLOAD_LENGTH]){
        buf[0] = block_num;

        uint8_t * dst = buf + 1;
        for(size_t i = 0; i < 16; i++){
            dst[i] = block_data[i];
        }
    }
};

struct [[nodiscard]] RegisterCard{
    static constexpr size_t PAYLOAD_LENGTH = 5;
    uint8_t block_num;
    uint32_t initial_value;

    constexpr void fill_bytes(__restrict uint8_t buf[PAYLOAD_LENGTH]){
        auto cursor = BufferCursor(buf);
        cursor.push_u8be(block_num); 
        cursor.push_u32be(initial_value); 
    }
};

struct [[nodiscard]] DeleteCard{
    static constexpr size_t PAYLOAD_LENGTH = 4;
    uint8_t block_num;

    constexpr void fill_bytes(__restrict uint8_t buf[PAYLOAD_LENGTH]){
        buf[0] = block_num;
        buf[1] = 0x38;
        buf[2] = 0x52;
        buf[3] = 0x7a;
    }
};

struct [[nodiscard]] ChargeMoney{
    static constexpr size_t PAYLOAD_LENGTH = 5;
    uint8_t block_num;
    uint32_t money;

    constexpr void fill_bytes(__restrict uint8_t buf[PAYLOAD_LENGTH]){
        auto cursor = BufferCursor(buf);
        cursor.push_u8be(block_num); 
        cursor.push_u32be(money); 
    }
};



}

#if 0
struct U13T_MsgFactory:public U13T_Prelude{


    static constexpr uint8_t calc_vcode(const uint8_t len, const uint8_t madder, const uint8_t command, const std::span<const uint8_t> payload){
        uint8_t vcode = len ^ madder ^ command;
        for(const auto & p: payload){
            vcode ^= p;
        }
        return vcode;
    }

    static constexpr std::vector<uint8_t> make_baudrate_message(const uint32_t baudrate, const uint8_t mod_address = 0){
        const uint8_t header = 0x7F;
        const uint8_t len = make_baudrate_payload(baudrate).size() + 3;
        const uint8_t maddr = mod_address;
        const uint8_t command = uint8_t(Command::SetBaudrate);
        const auto payload = make_baudrate_payload(baudrate);
        std::vector<uint8_t> message = {};
        message.push_back(header);
        message.push_back(len);
        message.push_back(maddr);
        message.push_back(command);
        message.insert(message.end(), payload.begin(), payload.end());  // Insert payload elements

        const auto vcode = calc_vcode(len, maddr, command, payload);
        message.push_back(vcode);

        return message;
    }

    static constexpr std::array<uint8_t, 5> make_read_card_num_message(const uint8_t mod_address = 0){
        (void)mod_address;
        return {0x7F, 0x03, 0x00, 0x10, 0x13};
    }

    static constexpr std::array<uint8_t, 6> make_read_block_data_message(const uint8_t mod_address = 0){
        (void)mod_address;
        return {0x7F, 0x04, 0x00, 0x11, 0x01, 0x14};
    }

    void set_baudrate(const uint32_t baudrate){
        (void)(baudrate);
    }
};
#endif


}
