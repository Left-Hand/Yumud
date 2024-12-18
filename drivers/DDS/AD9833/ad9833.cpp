#include "ad9833.hpp"


using namespace ymd::drivers;
using namespace ymd;



#define AD9833_Delay __nopn(4)

#define AD9833_SPI_WRITE(data)      spi_drv.writeSingle(data);
#define AD9833_FSYNC(x)             fsync_gpio = x;


// WaveMode
#define AD9833_OUT_SINUS    ((0 << 5) | (0 << 1) | (0 << 3))
#define AD9833_OUT_TRIANGLE ((0 << 5) | (1 << 1) | (0 << 3))
#define AD9833_OUT_MSB      ((1 << 5) | (0 << 1) | (1 << 3))
#define AD9833_OUT_MSB2     ((1 << 5) | (0 << 1) | (0 << 3))

// Registers
#define AD9833_REG_CMD      (0 << 14)
#define AD9833_REG_FREQ0    (1 << 14)
#define AD9833_REG_FREQ1    (2 << 14)
#define AD9833_REG_PHASE0   (6 << 13)
#define AD9833_REG_PHASE1   (7 << 13)


// Registers
#define AD9833_REG_CMD      (0 << 14)
#define AD9833_REG_FREQ0    (1 << 14)
#define AD9833_REG_FREQ1    (2 << 14)
#define AD9833_REG_PHASE0   (6 << 13)
#define AD9833_REG_PHASE1   (7 << 13)

// Command Control Bits
#define AD9833_B28          13  // 1: 连续写入频率寄存器的LSB和MSB，全部写入后才生效
#define AD9833_HLB          12  // 1: 单独写入LSB, 单独写入MSB, B28=1时无效
#define AD9833_FSELECT      11
#define AD9833_PSELECT      10
#define AD9833_PIN_SW       9   // Reserved, should be set to 0
#define AD9833_RESET        8   // 1: reset
#define AD9833_SLEEP1       7   // 1: disable MCLK clock, DAC hold
#define AD9833_SLEEP12      6   // 1: disable DAC, useful for MSB mode
#define AD9833_OPBITEN      5   // 0: sin/triangle, 1: MSB or MSB/2
#define AD9833_SIGN_PIB     4   // Reserved, should be set to 0
#define AD9833_DIV2         3   // 0: MSB, 1: MSB/2
#define AD9833_MODE         1   // 0: sin, 1: triangle


void AD9833::writeData(uint16_t data) {
    AD9833_FSYNC(1);
    AD9833_FSYNC(0);
    AD9833_SPI_WRITE(data);
    AD9833_FSYNC(1);
}


void AD9833::reset() {
    writeData(AD9833_REG_CMD | (1<<AD9833_RESET)); // 复位AD9833，即RESET位为1，0x0100
}



void AD9833::setFreq(uint16_t freq_reg, real_t freq, bool reset) {
    real_t freq_mid, freq_data;
    uint32_t freq_hex;
    uint16_t freq_LSB, freq_MSB;

    // 如果时钟频率不为25MHZ，修改该处的频率值，单位MHz ，AD9833最大支持25MHz
    freq_mid = (real_t) (1 << 28) / 25000000; // 适合25M晶振
    freq_data = freq * freq_mid;

    freq_hex = (uint32_t) freq_data; // 下面需要拆分成两个14位进行处理
    freq_LSB = freq_hex & 0x3fff; // 低14位送给LSB
    freq_MSB = (freq_hex >> 14) & 0x3fff; // 高14位送给MSB

    freq_LSB |= freq_reg;
    freq_MSB |= freq_reg;

    if (reset) {
        writeData(AD9833_REG_CMD | (1<<AD9833_B28) | (1<<AD9833_RESET)); // 选择数据一次写入，B28位和RESET位为1，0x2100
    } else {
        writeData(AD9833_REG_CMD | (1<<AD9833_B28)); // 选择数据一次写入，B28位为1。0x2000
    }
    writeData(freq_LSB);
    writeData(freq_MSB);
}


void AD9833::setPhase(uint16_t phase_reg, uint16_t phase) {
    uint16_t data = phase_reg | phase;
    writeData(data);
}


void AD9833::setWave(WaveMode wave_mode, uint16_t freq_reg, uint16_t phase_reg) {
    uint32_t freq_sel, phase_sel;

    if (freq_reg == AD9833_REG_FREQ0) {
        freq_sel = 0 << AD9833_FSELECT;
    } else {
        freq_sel = 1 << AD9833_FSELECT;
    }
    if (phase_reg == AD9833_REG_PHASE0) {
        phase_sel = 0 << AD9833_PSELECT;
    } else {
        phase_sel = 1 << AD9833_PSELECT;
    }

    uint16_t data = AD9833_REG_CMD | uint8_t(wave_mode) | freq_sel | phase_sel;
    writeData(data);
}


void AD9833::init(uint16_t freq_reg, real_t freq, uint16_t phase_reg, uint16_t phase, WaveMode wave_mode) {
    setFreq(freq_reg, freq, true);
    setPhase(phase_reg, phase);
    setWave(wave_mode, freq_reg, phase_reg);
}