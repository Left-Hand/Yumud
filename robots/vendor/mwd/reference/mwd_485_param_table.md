# 慕纬度电机 RS485 协议 - 控制参数表（完整版）
**文档版本**：V2.36  
**适用**：K / ZH / TS / L 系列电机  
**数据类型**：全部小端序（Little‑Endian）

## 电机控制参数总表
| 参数ID（十六进制） | 参数ID（十进制） | 参数名称 | 数据类型 | 长度 | 字节排布说明 |
|---|---|---|---|---|---|
| 0x0A | 10 | 角度环 PID | 3×uint16_t | 6字节 | Kp(2) + Ki(2) + Kd(2) |
| 0x0B | 11 | 速度环 PID | 3×uint16_t | 6字节 | Kp(2) + Ki(2) + Kd(2) |
| 0x0C | 12 | 电流环 PID | 3×uint16_t | 6字节 | Kp(2) + Ki(2) + Kd(2) |
| 0x1E | 30 | 最大力矩电流 | int16_t | 2字节 | 占第3、4字节 |
| 0x20 | 32 | 最大速度 | int32_t | 4字节 | 占第3～6字节 |
| 0x22 | 34 | 角度限制 | int32_t | 4字节 | 占第3～6字节 |
| 0x24 | 36 | 电流斜率 | int32_t | 4字节 | 占第3～6字节 |
| 0x26 | 38 | 速度斜率 | int32_t | 4字节 | 占第3～6字节 |

---

## 1. 参数ID 0x0A（10）：角度环 PID
- 长度：6 字节
- 结构：**Kp(2) + Ki(2) + Kd(2)**
- 类型：uint16_t（小端）

| 数据字节 | 含义 | 类型 |
|---|---|---|
| DATA1 | anglePidKp 低字节 | uint8_t |
| DATA2 | anglePidKp 高字节 | uint8_t |
| DATA3 | anglePidKi 低字节 | uint8_t |
| DATA4 | anglePidKi 高字节 | uint8_t |
| DATA5 | anglePidKd 低字节 | uint8_t |
| DATA6 | anglePidKd 高字节 | uint8_t |

---

## 2. 参数ID 0x0B（11）：速度环 PID
- 长度：6 字节
- 结构：**Kp(2) + Ki(2) + Kd(2)**
- 类型：uint16_t（小端）

| 数据字节 | 含义 | 类型 |
|---|---|---|
| DATA1 | speedPidKp 低字节 | uint8_t |
| DATA2 | speedPidKp 高字节 | uint8_t |
| DATA3 | speedPidKi 低字节 | uint8_t |
| DATA4 | speedPidKi 高字节 | uint8_t |
| DATA5 | speedPidKd 低字节 | uint8_t |
| DATA6 | speedPidKd 高字节 | uint8_t |

---

## 3. 参数ID 0x0C（12）：电流环 PID
- 长度：6 字节
- 结构：**Kp(2) + Ki(2) + Kd(2)**
- 类型：uint16_t（小端）

| 数据字节 | 含义 | 类型 |
|---|---|---|
| DATA1 | currentPidKp 低字节 | uint8_t |
| DATA2 | currentPidKp 高字节 | uint8_t |
| DATA3 | currentPidKi 低字节 | uint8_t |
| DATA4 | currentPidKi 高字节 | uint8_t |
| DATA5 | currentPidKd 低字节 | uint8_t |
| DATA6 | currentPidKd 高字节 | uint8_t |

---

## 4. 参数ID 0x1E（30）：最大力矩电流
- 长度：2 字节
- 类型：int16_t（小端）
- 存放位置：DATA3、DATA4

| 数据字节 | 含义 | 类型 |
|---|---|---|
| DATA3 | 最大力矩电流 低字节 | uint8_t |
| DATA4 | 最大力矩电流 高字节 | uint8_t |

---

## 5. 参数ID 0x20（32）：最大速度
- 长度：4 字节
- 类型：int32_t（小端）
- 存放位置：DATA3～DATA6

| 数据字节 | 含义 | 类型 |
|---|---|---|
| DATA3 | 最大速度 第1字节（最低） | uint8_t |
| DATA4 | 最大速度 第2字节 | uint8_t |
| DATA5 | 最大速度 第3字节 | uint8_t |
| DATA6 | 最大速度 第4字节（最高） | uint8_t |

---

## 6. 参数ID 0x22（34）：角度限制
- 长度：4 字节
- 类型：int32_t（小端）
- 存放位置：DATA3～DATA6

| 数据字节 | 含义 | 类型 |
|---|---|---|
| DATA3 | 角度限制 第1字节（最低） | uint8_t |
| DATA4 | 角度限制 第2字节 | uint8_t |
| DATA5 | 角度限制 第3字节 | uint8_t |
| DATA6 | 角度限制 第4字节（最高） | uint8_t |

---

## 7. 参数ID 0x24（36）：电流斜率
- 长度：4 字节
- 类型：int32_t（小端）
- 存放位置：DATA3～DATA6

| 数据字节 | 含义 | 类型 |
|---|---|---|
| DATA3 | 电流斜率 第1字节（最低） | uint8_t |
| DATA4 | 电流斜率 第2字节 | uint8_t |
| DATA5 | 电流斜率 第3字节 | uint8_t |
| DATA6 | 电流斜率 第4字节（最高） | uint8_t |

---

## 8. 参数ID 0x26（38）：速度斜率
- 长度：4 字节
- 类型：int32_t（小端）
- 存放位置：DATA3～DATA6

| 数据字节 | 含义 | 类型 |
|---|---|---|
| DATA3 | 速度斜率 第1字节（最低） | uint8_t |
| DATA4 | 速度斜率 第2字节 | uint8_t |
| DATA5 | 速度斜率 第3字节 | uint8_t |
| DATA6 | 速度斜率 第4字节（最高） | uint8_t |

### 统一说明
- 所有参数均通过 **0xC0（读）/0xC1（写）** 命令操作
- 读写帧数据固定 7 字节：`DATA0=参数ID，DATA1~DATA6=参数内容`
- 全部采用 **小端序**，校验为累加和低8位
