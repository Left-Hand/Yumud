

// void osc_test(){
//     uart2.init(115200 * 8, Uart::Mode::TxRx);
//     IOStream & logger = uart2;
//     logger.setSpace(",");
//     logger.set_eps(4);
//     Gpio & i2sSck = hal::PA<1>();
//     Gpio & i2sSda = hal::PA<0>();
//     Gpio & i2sWs = hal::PA<4>();
//     i2sSck.outpp();
//     i2sSda.outpp();
//     i2sWs.outpp();
//     I2sSw i2sSw(i2sSck, i2sSda, i2sWs);
//     i2sSw.init(114514);
//     // I2sDrv i2sDrvTm = I2sDrv(i2sSw);
//     // TM8211 extern_dac(i2sDrvTm);
//     // extern_dac.

//     // extern_dac.setDistort(0);
//     // extern_dac.setRail(real_t(0.3), real_t(3.0));
//     while(true){
//         // real_t audio_out = sin(40*t);
//         // real_t audio_volt = audio_out * 2 + 2.5;
//         // extern_dac.setVoltage(frac(t), frac(t));
//         // extern_dac.setChData(0, );
//         // uint32_t data = 0x8000 + int(0x7fff * sin(400 * t));
//         // data |= (data << 16);
//         // static uint16_t cnt;
//         // i2sSw.write((++cnt) << 16);
//         // uint16_t data = int(frac(t) * 0xffff);
//         static uint16_t data = 0;
//         data+= 80;
//         uint32_t data_out=  data<<16;
//         i2sSw.write(data_out);

//         // {
//         //     i2sWs.clr();
//         //     clock::delay(10us);
//         //     for(int i = 16; i > 0; i--)
//         //     {
//         //         i2sSck.clr();
//         //     clock::delay(10us);
//         //         i2sSda = (data >> i) & 0x01;
//         //     clock::delay(10us);
//         //        // digitalWrite(BCK, HIGH);
//         //         i2sSck.set();
//         //     clock::delay(10us);
//         //     }
//         //                     i2sSda.clr();
//         //     for(int i = 16; i > 0; i--)
//         //     {
//         //         i2sSck.clr();
//         //     clock::delay(10us);

//         //     // clock::delay(10us);
//         //         // digitalWrite(BCK, HIGH);
//         //         i2sSck.set();
//         //     clock::delay(10us);
//         //     }
//         //     i2sWs.set();
//         //     clock::delay(10us);
//         //     i2sWs.clr();
//         // }




//         logger.println(data_out);
//         // clock::delay(100us);
//         Sys::Clock::reCalculateTime();
//     }
// }


