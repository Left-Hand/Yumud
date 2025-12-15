#pragma once

#include <cstdint>
#include <cstddef>

// 参考来源：

// 无许可证
// https://github.com/ProfFan/icm426xx/blob/main/src/fifo.rs

//  * 注意：本实现为完全原创，未使用上述项目的任何代码。
//  * 参考仅用于理解问题领域，未复制任何具体实现。

namespace ymd::drivers::icm42688::details{

// FifoHeader: 8位bitfield结构体
struct [[nodiscard]] FifoHeader {
    uint8_t odr_changed_gyro : 1;      // 1: The ODR for gyro is different for this gyro data packet compared to the previous gyro packet
    uint8_t odr_changed_accel : 1;     // 1: The ODR for accel is different for this accel data packet compared to the previous accel packet
    uint8_t has_timestamp_fsync : 2;   // 10: Packet contains ODR Timestamp
    uint8_t has_20bit : 1;             // 1: Packet has a new and valid sample of extended 20-bit data for gyro and/or accel
    uint8_t has_gyro : 1;              // 1: Packet is sized so that gyro data have location in the packet, FIFO_GYRO_EN must be 1
    uint8_t has_accel : 1;             // 1: Packet is sized so that accel data have location in the packet, FIFO_ACCEL_EN must be 1
    uint8_t header_msg : 1;            // 1: FIFO is empty
};

static_assert(sizeof(FifoHeader) == 1);

// FifoPacket4: 数据包结构体，共20字节
struct [[nodiscard]] FifoPacket4 {
    FifoHeader fifo_header;
    uint8_t accel_data_x1;      // Accel X [19:12]
    uint8_t accel_data_x0;      // Accel X [11:4]
    uint8_t accel_data_y1;      // Accel Y [19:12]
    uint8_t accel_data_y0;      // Accel Y [11:4]
    uint8_t accel_data_z1;      // Accel Z [19:12]
    uint8_t accel_data_z0;      // Accel Z [11:4]
    uint8_t gyro_data_x1;       // Gyro X [19:12]
    uint8_t gyro_data_x0;       // Gyro X [11:4]
    uint8_t gyro_data_y1;       // Gyro Y [19:12]
    uint8_t gyro_data_y0;       // Gyro Y [11:4]
    uint8_t gyro_data_z1;       // Gyro Z [19:12]
    uint8_t gyro_data_z0;       // Gyro Z [11:4]
    uint8_t temp_data1;         // Temperature[15:8]
    uint8_t temp_data0;         // Temperature[7:0]
    uint8_t timestamp_h;        // TimeStamp[15:8]
    uint8_t timestamp_l;        // TimeStamp[7:0]
    uint8_t ext_accel_x_gyro_x; // Accel X [3:0] Gyro X [3:0]
    uint8_t ext_accel_y_gyro_y; // Accel Y [3:0] Gyro Y [3:0]
    uint8_t ext_accel_z_gyro_z; // Accel Z [3:0] Gyro Z [3:0]
private:
    // 内部辅助函数，将高8位、低8位和扩展4位拼成20位有符号整数
    int32_t convert_parts_to_20bit(uint8_t high_8, uint8_t low_8, uint8_t ext_low_4) const {
        uint32_t high_12 = (uint32_t(high_8) << 12);
        uint32_t low_12 = (uint32_t(low_8) << 4);
        uint32_t ext_4 = (uint32_t(ext_low_4) & 0xF);
        uint32_t value = high_12 | low_12 | ext_4;

        // 符号扩展
        if (value & 0x80000) {
            value |= 0xFFF00000;
        }
        return static_cast<int32_t>(value);
    }

public:
    [[nodiscard]] constexpr int32_t accel_data_x() const {
        uint8_t ext_accel_x = (ext_accel_x_gyro_x & 0xF0) >> 4;
        return convert_parts_to_20bit(accel_data_x1, accel_data_x0, ext_accel_x);
    }

    [[nodiscard]] constexpr int32_t accel_data_y() const {
        uint8_t ext_accel_y = (ext_accel_y_gyro_y & 0xF0) >> 4;
        return convert_parts_to_20bit(accel_data_y1, accel_data_y0, ext_accel_y);
    }

    [[nodiscard]] constexpr int32_t accel_data_z() const {
        uint8_t ext_accel_z = (ext_accel_z_gyro_z & 0xF0) >> 4;
        return convert_parts_to_20bit(accel_data_z1, accel_data_z0, ext_accel_z);
    }

    [[nodiscard]] constexpr int32_t gyro_data_x() const {
        uint8_t ext_gyro_x = ext_accel_x_gyro_x & 0x0F;
        return convert_parts_to_20bit(gyro_data_x1, gyro_data_x0, ext_gyro_x);
    }

    [[nodiscard]] constexpr int32_t gyro_data_y() const {
        uint8_t ext_gyro_y = ext_accel_y_gyro_y & 0x0F;
        return convert_parts_to_20bit(gyro_data_y1, gyro_data_y0, ext_gyro_y);
    }

    [[nodiscard]] constexpr int32_t gyro_data_z() const {
        uint8_t ext_gyro_z = ext_accel_z_gyro_z & 0x0F;
        return convert_parts_to_20bit(gyro_data_z1, gyro_data_z0, ext_gyro_z);
    }

    [[nodiscard]] constexpr uint16_t temperature_raw() const {
        return (uint16_t(temp_data1) << 8) | temp_data0;
    }

    [[nodiscard]] constexpr uint16_t timestamp() const {
        return (uint16_t(timestamp_h) << 8) | timestamp_l;
    }
};

// 静态断言结构体大小为20字节
static_assert(sizeof(FifoPacket4) == 20, "FifoPacket4 must be 20 bytes");

}