#pragma once

#include "core/io/regs.hpp"
#include "hal/bus/uart/uart.hpp"

namespace ymd::drivers{




class U13T{
public:
    class Command{
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
            SetBuadrate = 0x2c,
            SetAddress = 0x2d,
            SetAutoMode = 0x2e,
        };
        
        constexpr Command(const Kind kind, bool is_rx):
            is_rx_(is_rx){;}

        static constexpr Command from_raw(const uint8_t raw){
            return Command(static_cast<Kind>(raw), raw & 0x80);
        }

        constexpr Kind kind() const {return kind_;}
        constexpr bool is_rx() const {return is_rx_;}
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

// private:
// private:

    RingBuf<32> recv;
    int8_t dead_ticks = 0;
    const int8_t dead_limit = 3;

    // bool checkNew();
    // void clearBuffer();

    // void lineCb();

    hal::Uart & uart_;

    // template<size_t N>
    // static constexpr std::array<uint8_t, N> make_payload()



public:
    U13T(hal::Uart & uart):uart_(uart){;}

    void init();
    void tick();
    void update();

    void write(std::span<const uint8_t> pdata){
        uart_.writeN(reinterpret_cast<const char *>(static_cast<const uint8_t *>(&pdata[0])), pdata.size());
    }

    static constexpr std::array<uint8_t, 7> make_baudrate_payload(const uint32_t baudrate){
        return {
            uint8_t(baudrate >> 24  ), 
            uint8_t(baudrate >> 16  ), 
            uint8_t(baudrate >> 8   ), 
            uint8_t(baudrate        ), 
            0x98,
            0x24,
            0x31
        };
    }

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
        const uint8_t command = uint8_t(Command::SetBuadrate);
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
        return {0x7F, 0x03, 0x00, 0x10, 0x13};
    }

    static constexpr std::array<uint8_t, 6> make_read_block_data_message(const uint8_t mod_address = 0){
        return {0x7F, 0x04, 0x00, 0x11, 0x01, 0x14};
    }

    void set_baudrate(const uint32_t baudrate){
            // DEBUG_PRINTLN(std::hex, drivers::U13T::make_baudrate_message(9600));
    // const auto msg  =drivers::U13T::make_baudrate_message(115200);
    // const auto msg  =drivers::U13T::make_read_card_num_message(); 
    // u13t.write(std::span(msg));
    }
};

// static constexpr auto msg = U13T::make_baudrate_message(9600);


}