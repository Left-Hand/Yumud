#pragma once


// template>class Tx{
//     Fifo_t<uint8_t, LT8960L_MAX_FIFO_SIZE> fifo_;

//     size_t write(std::span<const uint8_t> pbuf){
//         fifo_.push(pbuf);
//         return pbuf.size();
//     }

//     size_t pending() const {
//         return fifo_.available();
//     }
// };


// class Rx{
//     Fifo_t<uint8_t, LT8960L_MAX_FIFO_SIZE> fifo_;

//     size_t read(std::span<uint8_t> pbuf){
//         fifo_.pop(pbuf);
//         return pbuf.size();
//     }

//     size_t awailable() const {
//         return fifo_.available();
//     }
// };
