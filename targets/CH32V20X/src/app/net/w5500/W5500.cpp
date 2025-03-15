/*
 * Copyright (c) 2010 by WIZnet <support@wiznet.co.kr>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 *
 * - 10 Apr. 2015
 * Added support for Arduino Ethernet Shield 2
 * by Arduino.org team
 */

 
 #include "W5500.hpp"
 //#if defined(W5500_ETHERNET_SHIELD)
 
 // W5500 controller instance

 using namespace ymd::drivers;

 // SPI details
 uint8_t SPI_CS;
 
 void W5500::init(uint8_t ss_pin)
 {
   SPI_CS = ss_pin;
 
   delay(1000);
   w5500.swReset();
   for (int i=0; i<MAX_SOCK_NUM; i++) {
     uint8_t cntl_byte = (0x0C + (i<<5));
     write( 0x1E, cntl_byte, 2); //0x1E - Sn_RXBUF_SIZE
     write( 0x1F, cntl_byte, 2); //0x1F - Sn_TXBUF_SIZE
   }
 }
 
 #if defined(ESP32)
 // extension for M5Stack ATOM
 void W5500::init(uint8_t sck_pin, uint8_t miso_pin, uint8_t mosi_pin, uint8_t ss_pin)
 {
   SPI_CS = ss_pin;
 
   delay(1000);
   initSS();
   SPI.begin(sck_pin, miso_pin, mosi_pin, ss_pin);
   w5500.swReset();
   for (int i=0; i<MAX_SOCK_NUM; i++) {
     uint8_t cntl_byte = (0x0C + (i<<5));
     write( 0x1E, cntl_byte, 2); //0x1E - Sn_RXBUF_SIZE
     write( 0x1F, cntl_byte, 2); //0x1F - Sn_TXBUF_SIZE
   }
 }
 #endif
 
 uint16_t W5500::getTXFreeSize(SOCKET s)
 {
     uint16_t val=0, val1=0;
     do {
         val1 = readSnTX_FSR(s);
         if (val1 != 0)
             val = readSnTX_FSR(s);
     } 
     while (val != val1);
     return val;
 }
 
 uint16_t W5500::getRXReceivedSize(SOCKET s)
 {
     uint16_t val=0,val1=0;
     do {
         val1 = readSnRX_RSR(s);
         if (val1 != 0)
             val = readSnRX_RSR(s);
     } 
     while (val != val1);
     return val;
 }
 
 void W5500::send_data_processing(SOCKET s, const uint8_t *data, uint16_t len)
 {
   // This is same as having no offset in a call to send_data_processing_offset
   send_data_processing_offset(s, 0, data, len);
 
 }
 
 void W5500::send_data_processing_offset(SOCKET s, uint16_t data_offset, const uint8_t *data, uint16_t len)
 {
     uint16_t ptr = readSnTX_WR(s);
     uint8_t cntl_byte = (0x14+(s<<5));
     ptr += data_offset;
     write(ptr, cntl_byte, data, len);
     ptr += len;
     writeSnTX_WR(s, ptr);
 }
 
 void W5500::recv_data_processing(SOCKET s, uint8_t *data, uint16_t len, uint8_t peek)
 {
     uint16_t ptr;
     ptr = readSnRX_RD(s);
 
     read_data(s, ptr, data, len);
     if (!peek)
     {
         ptr += len;
         writeSnRX_RD(s, ptr);
     }
 }
 
 void W5500::read_data(SOCKET s, uint16_t src, uint8_t *dst, uint16_t len)
 {
     uint8_t cntl_byte = (0x18+(s<<5));
     read((uint16_t)src , cntl_byte, (uint8_t *)dst, len);
 }
 
 uint8_t W5500::write(uint16_t _addr, uint8_t _cb, uint8_t _data)
 {
    //  SPI.beginTransaction(wiznet_SPI_settings);
    //  setSS();  

    uint8_t buf[] = {
        uint8_t(_addr >> 8),
        uint8_t(_addr & 0xFF),
        _cb,
        _data
    };

     spi_drv_.writeBurst<uint8_t>(static_cast<const uint8_t *>(buf), sizeof(buf), DISC);
    //  SPI.transfer(_addr >> 8);
    //  SPI.transfer(_addr & 0xFF);
    //  SPI.transfer(_cb);
    //  SPI.transfer(_data);
    //  resetSS();
    //  SPI.endTransaction();
 
     return 1;
 }
 
 uint16_t W5500::write(uint16_t _addr, uint8_t _cb, const uint8_t *_buf, uint16_t _len)
 {
    //  SPI.beginTransaction(wiznet_SPI_settings);
    //  setSS();
    //  SPI.transfer(_addr >> 8);
    //  SPI.transfer(_addr & 0xFF);
    //  SPI.transfer(_cb);
    //  for (uint16_t i=0; i<_len; i++){
    //      SPI.transfer(_buf[i]);
    //  }
    //  resetSS();
    //  SPI.endTransaction();
 
    uint8_t buf[] = {
        uint8_t(_addr >> 8),
        uint8_t(_addr & 0xFF),
        _cb,
    };

     spi_drv_.writeBurst<uint8_t>(static_cast<const uint8_t *>(buf), sizeof(buf), DISC);
     spi_drv_.writeBurst<uint8_t>(_buf, _len, CONT);

     return _len;
 }
 
 uint8_t W5500::read(uint16_t _addr, uint8_t _cb)
 {
    //  SPI.beginTransaction(wiznet_SPI_settings);
    //  setSS();
    //  SPI.transfer(_addr >> 8);
    //  SPI.transfer(_addr & 0xFF);
    //  SPI.transfer(_cb);
    //  uint8_t _data = SPI.transfer(0);
    //  resetSS();
    //  SPI.endTransaction();
    uint8_t _data = 0;
     return _data;
 }
 
 uint16_t W5500::read(uint16_t _addr, uint8_t _cb, uint8_t *_buf, uint16_t _len)
 { 
    //  SPI.beginTransaction(wiznet_SPI_settings);
    //  setSS();
    //  SPI.transfer(_addr >> 8);
    //  SPI.transfer(_addr & 0xFF);
    //  SPI.transfer(_cb);
    //  for (uint16_t i=0; i<_len; i++){
    //      _buf[i] = SPI.transfer(0);
    //  }
    //  resetSS();
    //  SPI.endTransaction();
 
     return _len;
 }
 
 void W5500::execCmdSn(SOCKET s, SockCMD _cmd) {
     // Send command to socket
     writeSnCR(s, _cmd);
     // Wait for command to complete
     while (readSnCR(s))
     ;
 }
 
 
 uint8_t W5500::readVersion(void)
 {
    //  SPI.beginTransaction(wiznet_SPI_settings);
    //  setSS();
    //  SPI.transfer( 0x00 );
    //  SPI.transfer( 0x39 );
    //  SPI.transfer( 0x01);
    //  uint8_t _data = SPI.transfer(0);
    //  resetSS();
    //  SPI.endTransaction();
    
    uint8_t _data = 0;
     return _data;
 }
 
//  void a(){
//     W5500 w{SpiDrv{Spi1, 0}};
//     w.
//  }
 
 //#endif