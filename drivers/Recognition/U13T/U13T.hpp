#pragma once

#include "drivers/device_defs.h"


namespace ymd::drivers{

class Matcher{
    private:
        scexpr uint8_t item[16][16] = {
            {0xc8,0xf0,0xbd,0xf0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
            {0xcd,0xbb,0xc6,0xc6,0xb5,0xda,0xc8,0xfd,0xb5,0xc0,0xb7,0xe2,0xcb,0xf8,0xcf,0xdf},
            {0xd1,0xaa,0xd5,0xbd,0xcf,0xe6,0xbd,0xad,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
            {0xc7,0xbf,0xb6,0xc9,0xce,0xda,0xbd,0xad,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
            {0xd5,0xbc,0xc1,0xec,0xd7,0xf1,0xd2,0xe5,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
            {0xcb,0xc4,0xb6,0xc9,0xb3,0xe0,0xcb,0xae,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
            {0xc7,0xc9,0xb6,0xc9,0xbd,0xf0,0xc9,0xb3,0xbd,0xad,0x00,0x00,0x00,0x00,0x00,0x00},
            {0xc7,0xbf,0xb6,0xc9,0xb4,0xf3,0xb6,0xc9,0xba,0xd3,0x00,0x00,0x00,0x00,0x00,0x00},
            {0xb7,0xc9,0xb6,0xe1,0xe3,0xf2,0xb6,0xa8,0xc7,0xc5,0x00,0x00,0x00,0x00,0x00,0x00},
            {0xc5,0xc0,0xd1,0xa9,0xc9,0xbd,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
            {0xed,0xae,0xb9,0xa6,0xbb,0xe1,0xca,0xa6,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
            {0xb9,0xfd,0xb2,0xdd,0xb5,0xd8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
            {0xbc,0xa4,0xd5,0xbd,0xc0,0xb0,0xd7,0xd3,0xbf,0xda,0x00,0x00,0x00,0x00,0x00,0x00},
            {0xbb,0xe1,0xc4,0xfe,0xb4,0xf3,0xbb,0xe1,0xca,0xa6,0x00,0x00,0x00,0x00,0x00,0x00},
            {0xce,0xe2,0xc6,0xf0,0xd5,0xf2,0xbb,0xe1,0xd2,0xe9,0x00,0x00,0x00,0x00,0x00,0x00},
            {0xd1,0xd3,0xb0,0xb2,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
        };

        bool marked[16] = {false};
    public:
        uint8_t matchID(const uint8_t * iptr); 
};

class U13T{
    private:
        std::vector<uint8_t> buffer = {};
        std::vector<uint8_t> recv = {};
        int8_t dead_ticks = 0;
        const int8_t dead_limit = 3;

        Matcher matcher;
        // Gpio & serPin = Pin(GPIOA, GPIO_Pin_0);
        // Gpio & serPin;
        // Gpio & ledPin = Pin(GPIOA, GPIO_Pin_1);

        // JQ8900 player = JQ8900(serPin);
        uint16_t ledAlive = 0;

        // void initGpio();
        // void initUart();
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