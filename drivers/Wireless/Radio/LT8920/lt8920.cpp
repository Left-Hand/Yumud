#include "lt8920.hpp"

// void LT8920::read(uint8_t *buffer, size_t maxBuffer){
//     uint16_t value = readRegister(R_STATUS);
//     if (bitRead(value, STATUS_CRC_BIT) == 0){
//         //CRC ok

//         uint16_t data = readRegister(R_FIFO);
//         uint8_t packetSize = data >> 8;
//         if(maxBuffer < packetSize+1)
//         {
//             //BUFFER TOO SMALL
//             return -2;
//         }

//         uint8_t pos;
//         buffer[pos++] = (data & 0xFF);
//         while (pos < packetSize)
//         {
//         data = readRegister(R_FIFO);
//         buffer[pos++] = data >> 8;
//         buffer[pos++] = data & 0xFF;
//         }

//         return packetSize;
//     }else{
//             //CRC error
//             return -1;
//     }
// }

// void LT8920::startListening()
// {
//   writeRegister(R_CHANNEL, _channel & CHANNEL_MASK);   //turn off rx/tx
//   delay(3);
//   writeRegister(R_FIFO_CONTROL, 0x0080);  //flush rx
//   writeRegister(R_CHANNEL,  (_channel & CHANNEL_MASK) | _BV(CHANNEL_RX_BIT));   //enable RX
//   delay(5);
// }

// bool LT8920::sendPacket(uint8_t *data, size_t packetSize)
// {
//   if (packetSize < 1 || packetSize > 255)
//   {
//     return false;
//   }

//   writeRegister(R_CHANNEL, 0x0000);
//   writeRegister(R_FIFO_CONTROL, 0x8000);  //flush tx

//   //packets are sent in 16bit words, and the first word will be the packet size.
//   //start spitting out words until we are done.

//   uint8_t pos = 0;
//   writeRegister2(R_FIFO, packetSize, data[pos++]);
//   while (pos < packetSize)
//   {
//     uint8_t msb = data[pos++];
//     uint8_t lsb = data[pos++];

//     writeRegister2(R_FIFO, msb, lsb);
//   }

//   writeRegister(R_CHANNEL,  (_channel & CHANNEL_MASK) | _BV(CHANNEL_TX_BIT));   //enable TX

//   //Wait until the packet is sent.
//   while (digitalRead(_pin_pktflag) == 0)
//   {
//       //do nothing.
//   }

//   return true;
// }

// void LT8920::begin()
// {
//     if(_pin_reset > 0)
//     {
//         digitalWrite(_pin_reset, LOW);
//         delay(200);
//         digitalWrite(_pin_reset, HIGH);
//         delay(200);

//     }
//   //setup

//   writeRegister(0, 0x6fe0);
//   writeRegister(1, 0x5681);
//   writeRegister(2, 0x6617);
//   writeRegister(4, 0x9cc9);    //why does this differ from powerup (5447)
//   writeRegister(5, 0x6637);    //why does this differ from powerup (f000)
//   writeRegister(8, 0x6c90);    //power (default 71af) UNDOCUMENTED

//   setCurrentControl(4, 0);     // power & gain.

//   writeRegister(10, 0x7ffd);   //bit 0: XTAL OSC enable
//   writeRegister(11, 0x0000);   //bit 8: Power down RSSI (0=  RSSI operates normal)
//   writeRegister(12, 0x0000);
//   writeRegister(13, 0x48bd);   //(default 4855)

//   writeRegister(22, 0x00ff);
//   writeRegister(23, 0x8005);  //bit 2: Calibrate VCO before each Rx/Tx enable
//   writeRegister(24, 0x0067);
//   writeRegister(25, 0x1659);
//   writeRegister(26, 0x19e0);
//   writeRegister(27, 0x1300);  //bits 5:0, Crystal Frequency adjust
//   writeRegister(28, 0x1800);

//   //fedcba9876543210
//   writeRegister(32, 0x5000);  //AAABBCCCDDEEFFFG  A preamble length, B, syncword length, c trailer length, d packet type
//   //                  E FEC_type, F BRCLK_SEL, G reserved
//   //0x5000 = 0101 0000 0000 0000 = preamble 010 (3 bytes), B 10 (48 bits)
//   writeRegister(33, 0x3fc7);
//   writeRegister(34, 0x2000);  //
//   writeRegister(35, 0x0300);  //POWER mode,  bit 8/9 on = retransmit = 3x (default)
//   setSyncWord(0x03805a5a03800380);

//   writeRegister(40, 0x4401);  //max allowed error bits = 0 (01 = 0 error bits)
//   writeRegister(R_PACKETCONFIG,
//       PACKETCONFIG_CRC_ON |
//       PACKETCONFIG_PACK_LEN_ENABLE |
//       PACKETCONFIG_FW_TERM_TX
//   );

//   writeRegister(42, 0xfdb0);
//   writeRegister(43, 0x000f);

//   //setDataRate(LT8920_1MBPS);

//   writeRegister(R_FIFO, 0x0000);  //TXRX_FIFO_REG (FIFO queue)

//   writeRegister(R_FIFO_CONTROL, 0x8080); //Fifo Rx/Tx queue reset

//   delay(200);
//   writeRegister(R_CHANNEL, _BV(CHANNEL_TX_BIT));  //set TX mode.  (TX = bit 8, RX = bit 7, so RX would be 0x0080)
//   delay(2);
//   writeRegister(R_CHANNEL, _channel);  // Frequency = 2402 + channel
// }