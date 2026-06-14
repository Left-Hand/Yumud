## 📡 CAN ID 格式（11-bit）

```
┌─────────────────────────────────────────┐
│  Bit 10  │  Bits 9-5  │  Bits 4-0       │
├──────────┼────────────┼─────────────────┤
│ ECHO(1) │ NODE_ID(5) │ CMD_ID(5)       │
│ 0x400   │  0x3E0     │  0x01F          │
└─────────────────────────────────────────┘
```

- **ECHO 位**：区分主机命令（0）和设备回复（1）
- **NODE_ID**：设备ID（5位，范围0-31），0表示广播
- **CMD_ID**：命令代码（5位，范围0-31）

## 📊 数据编码格式

### 基础类型转换（小端序 Little Endian）

```c
// float（4字节）
float_to_data(float val, uint8_t *data)
// data[0] = LSB, data[3] = MSB

// int32（4字节）
int32_to_data(int32_t val, uint8_t *data)

// int16（2字节）
int16_to_data(int16_t val, uint8_t *data)
```

## 🔧 完整命令协议

| 命令 | ID | 发送 | 返回 | 描述 |
|------|----|----|------|-----|
| **控制类** | | | | |
| SET_OP_MODE | 0 | mode(1B) | 0x00/0xEE | 设置运行模式 |
| MOTOR_ENABLE | 1 | - | 0x00/0xEE | 使能电机 |
| MOTOR_DISABLE | 2 | - | 0x00/0xEE | 禁用电机 |
| **目标值设置** | | | | |
| SET_TORQUE | 3 | float(4B) | - | 扭矩命令 |
| SET_VELOCITY | 4 | float(4B) | - | 速度命令 |
| SET_POSITION | 5 | float(4B) | - | 位置命令 |
| SYNC | 6 | - | - | 同步目标值 |
| **标定** | | | | |
| CALIB_START | 7 | - | 0x00/0xEE | 启动标定 |
| CALIB_REPORT | 8 | step(4B)+data(4B) | - | 标定报告 |
| CALIB_ABORT | 9 | - | 0x00/0xEE | 中止标定 |
| **反齿槽** | | | | |
| ANTICOGGING_START | 10 | - | 0x00/0xEE | 启动反齿槽标定 |
| ANTICOGGING_REPORT | 11 | step(4B)+value(4B) | - | 反齿槽报告 |
| ANTICOGGING_ABORT | 12 | - | 0x00/0xEE | 中止反齿槽 |
| **状态查询** | | | | |
| SET_HOME | 13 | - | 0x00/0xEE | 设置零点 |
| ERROR_RESET | 14 | - | 0x00/0xEE | 错误复位 |
| GET_STATUSWORD | 15 | - | status(1B)+error(1B) | 获取状态字 |
| STATUSWORD_REPORT | 16 | - | - | 状态字上报 |
| **数据查询** | | | | |
| GET_VALUE_1 | 17 | idx(1B) | data(4B) | 查询值1 |
| GET_VALUE_2 | 18 | idx(1B) | data(4B) | 查询值2 |
| **系统** | | | | |
| HEARTBEAT | 23 | - | - | 心跳包 |
| SET_CONFIG | 24 | idx(4B)+val(4B) | - | 设置配置 |
| GET_CONFIG | 25 | idx(4B) | idx(4B)+val(4B) | 读取配置 |
| SAVE_CONFIG | 26 | - | 0x00/0xEE | 保存配置 |
| RESET_CONFIG | 27 | - | 0x00/0xEE | 复位配置 |
| GET_FW_VERSION | 28 | - | major(1B)+minor(1B) | 固件版本 |
| **DFU** | | | | |
| DFU_START | 29 | - | 0x00/0xEE | 启动DFU |
| DFU_DATA | 30 | data(1-8B) | 0x00/0xEE | DFU数据 |
| DFU_END | 31 | crc(4B)+size(4B) | 0x00/0xEE | 完成DFU |

## 📈 GET_VALUE 查询索引表

```c
// GET_VALUE_1 和 GET_VALUE_2 的 idx 参数
0 → Foc.i_q_filt    // Q轴电流(float)
1 → Encoder.vel     // 编码器速度(float)
2 → Encoder.pos     // 编码器位置(float)
3 → Foc.v_bus_filt  // 总线电压(float)
4 → Foc.i_bus_filt  // 总线电流(float)
5 → Foc.power_filt  // 功率(float)
6 → drv_temp        // DRV温度(float)
7 → ntc_temp        // NTC温度(float)
```

## 🔔 状态字格式

```c
// CAN_CMD_GET_STATUSWORD 返回：
data[0] (status_code):
  Bit 0: switched_on    // 已启用
  Bit 1: target_reached // 目标已到达
  
data[1] (errors_code):
  Bit 0: over_voltage   // 过压
  Bit 1: under_voltage  // 欠压
  Bit 2: over_current   // 过流
  Bit 3: drv_over_tmp   // DRV过温
  Bit 4: ntc_over_tmp   // NTC过温
  Bit 7: selftest_err   // 自检失败
```