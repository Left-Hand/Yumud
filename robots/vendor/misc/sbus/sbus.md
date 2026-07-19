# SBUS 协议帧格式规范

SBUS（Serial Bus）是 Futaba 开发的串行总线协议，广泛用于 RC 接收机与飞控之间的通信。

## 物理层参数

| 参数 | 值 |
|------|-----|
| 波特率 | 100,000 bps（Fast SBUS：200,000 bps） |
| 数据位 | 8 |
| 校验位 | 偶校验（Even） |
| 停止位 | 2 |
| 逻辑电平 | **反相 UART**（高电平为逻辑 0） |

### 设计背景

SBUS 脱胎于 RS-232，而非 MCU 常见的 TTL UART，这直接导致了两个"非标"特性：

**反相电平**：Futaba 在接收机输出电路中使用光耦做电气隔离。NPN 型光耦有电流时输出下拉为低，天然翻转信号，"反相"是光耦隔离的副产品而非刻意为之。后来 FrSky 等厂商实现 SBUS 兼容时选择了正相，形成了两个变体。

**100,000 波特率**：RS-232 本身没有强制要求使用标准波特率（9600/115200 等是 PC 时代 16× 时钟分频的历史积累）。Futaba 选用 100,000 bps 的实用理由是：25 字节 × 8E2 帧约 3 ms，恰好整除 10 ms 的发包周期，时序对齐整洁。

> 实际接入时，F7/H7 等处理器可通过 UART 外设配置软件反相，无需外部反相器；较老的 MCU 则需在信号线上加反相电路（一个 NPN 三极管即可）。

---

## 帧结构（25 字节）

每帧固定 25 字节，结构如下：

```
| Byte 0  | Bytes 1–22      | Byte 23    | Byte 24  |
|---------|-----------------|------------|----------|
| 0x0F    | 通道数据（22B） | 标志位     | 0x00     |
| 起始字节 |                 |            | 结束字节  |
```

### 各字段说明

| 字段 | 位置 | 长度 | 固定值 | 说明 |
|------|------|------|--------|------|
| 起始字节（Start Byte） | Byte 0 | 1 字节 | `0x0F` | 帧同步标志，所有合法帧必须以此开头 |
| 通道数据（Channel Data） | Bytes 1–22 | 22 字节 | — | 16 个比例通道，每通道 11 bit，共 176 bit |
| 标志位（Flags） | Byte 23 | 1 字节 | — | 数字通道与故障状态标志 |
| 结束字节（End Byte） | Byte 24 | 1 字节 | `0x00` | 帧结束标志 |

---

## 通道数据编码

**容量计算：** 16 通道 × 11 bit/通道 = 176 bit = 22 字节（恰好无冗余）

每通道 11 bit，取值范围 `0–2047`（2¹¹）。数据以 LSB 优先顺序跨字节连续排列：

| 数据字节 | bit 内容（从高到低） |
|----------|----------------------|
| Byte 1 | `[CH1.7 CH1.6 CH1.5 CH1.4 CH1.3 CH1.2 CH1.1 CH1.0]` |
| Byte 2 | `[CH2.4 CH2.3 CH2.2 CH2.1 CH2.0 CH1.10 CH1.9 CH1.8]` |
| Byte 3 | `[CH3.1 CH3.0 CH2.10 CH2.9 CH2.8 CH2.7 CH2.6 CH2.5]` |
| … | … |

**典型通道值范围（以 FrSky 接收机为例）：**

| 行程设置 | 原始值范围 |
|----------|------------|
| ±100% | 172–1811 |
| ±150%（扩展） | 0–2047 |
| Betaflight 映射后 | 1000–2000 µs |

---

## 标志位（Byte 23）字段定义

```
bit 7   bit 6   bit 5        bit 4       bit 3  bit 2  bit 1  bit 0
 CH17    CH18  frame_lost  failsafe       0      0      0      0
(0x80)  (0x40)  (0x20)     (0x10)
```

| bit | 掩码 | 名称 | 说明 |
|-----|------|------|------|
| 7 | `0x80` | CH17 | 数字通道 17（开/关） |
| 6 | `0x40` | CH18 | 数字通道 18（开/关） |
| 5 | `0x20` | frame_lost | 当前帧丢失（接收机与发射机之间单帧丢包） |
| 4 | `0x10` | failsafe | 故障保护激活（连续多帧丢失后触发） |
| 3–0 | — | — | 保留，固定为 0 |

> `frame_lost` 与 `failsafe` 区别：`frame_lost` 表示单帧未收到，`failsafe` 表示接收机已切换至故障保护模式（需连续丢帧达到阈值）。

---

## 时序

| 参数 | 值 |
|------|-----|
| 单帧传输时间 | ~3 ms（25 字节 × 8E2） |
| 标准发送间隔 | 10 ms 或 20 ms |
| Fast SBUS 间隔 | ~6.66 ms（200 kbps） |

---

## 参考资料

- [UZH-RPG SBUS Protocol Wiki](https://github.com/uzh-rpg/rpg_quadrotor_control/wiki/SBUS-Protocol)
- [bolderflight/sbus](https://github.com/bolderflight/sbus)
- [sigrok Protocol Decoder: sbus_futaba](https://sigrok.org/wiki/Protocol_decoder:Sbus_futaba)
- [Futaba S-BUS Introduction PDF](https://cdck-file-uploads-europe1.s3.dualstack.eu-west-1.amazonaws.com/arduino/original/4X/3/6/a/36adfecc5ac5988048603b11d50132c3e5d79b49.pdf)
