#include "cdctl01a.hpp"


#if 0
// 初始化
cd_write(REG_CLK_CTRL, 0x80); // Soft reset
cd_write(REG_PIN_RE_CTRL, 0x10); // Set RE_N pin to low
cd_write(REG_SETTING, 0x11); // Enable push-pull output
cd_write(REG_FILTER, 0x0c); // Set FILTER
// Set baudrates
cd_write(REG_DIV_LS_L, 11); // 1 Mbps @ 12MHz sysclk
cd_write(REG_DIV_LS_H, 0);
cd_write(REG_DIV_HS_L, 2); // 4 Mbps @ 12MHz sysclk
cd_write(REG_DIV_HS_H, 0);
// cd_write(REG_RX_CTRL, 0x11); // Reset RX buffers and flags (optional)
// Enable interrupts (optional)
// cd_write(REG_INT_MASK, BIT_FLAG_TX_ERROR | BIT_FLAG_RX_ERROR \
| BIT_FLAG_RX_LOST | BIT_FLAG_RX_PENDING);
#endif

#if 0
// 发送
uint8_t tx_buf[] = {
0x0c, 0x0d, 0x02, // src_addr, dst_addr, data_len
0x01, 0x00 // data[0], data[1]
};
cd_write_chunk(REG_TX, tx_buf, tx_buf[2] + 3); // Write frame without CRC
while (!(cd_read(REG_INT_FLAG) & 0x20)); // Make sure we can successfully switch to the next page
cd_write(REG_TX_CTRL, 0x03); // Trigger send by switching TX page
#endif

#if 0
// 接收
while (!(cd_read(REG_INT_FLAG) & 0x02)); // Wait for RX page ready
cd_read_chunk(REG_RX, rx_buf, 3); // Read frame header
cd_read_chunk(REG_RX, rx_buf + 3, rx_buf[2]); // Read frame data
cd_write(REG_RX_CTRL, 0x03); // Finish read by switching RX page
#endif