#pragma once

#include "core/io/regs.hpp"
#include "hal/bus/uart/uart.hpp"

namespace ymd::drivers{




class U13T{
public:
    class Command{
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

private:
    RingBuf<32> recv;
    int8_t dead_ticks = 0;
    const int8_t dead_limit = 3;

    bool checkNew();
    void clearBuffer();

    void lineCb();

    hal::Uart & uart_;
public:
    U13T(hal::Uart & uart):uart_(uart){;}

    void init();
    void tick();
    void update();
};


}