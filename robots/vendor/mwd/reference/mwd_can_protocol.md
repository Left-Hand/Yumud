# 慕纬度科技 电机CAN总线通讯协议 V2.35


## 一、CAN总线参数
- 总线接口：CAN
- 波特率（常规模式，单电机命令）：1Mbps（默认）、500kbps、250kbps、125kbps、100kbps
- 波特率（广播模式，多电机命令）：1Mbps、500kbps

---

## 二、单电机命令
同一总线上共可以挂载多达32（视总线负载情况而定）个驱动，为了防止总线冲突，每个驱动需要设置不同的ID。
主控向总线发送单电机命令，对应ID的电机在收到命令后执行，并在一段时间后（0.25ms内）向主控发送回复。命令报文和回复报文格式如下：
- 标识符：0x140 + ID（1~32）
- 帧格式：数据帧
- 帧类型：标准帧
- DLC：8字节

### 1. 读取电机状态1和错误标志命令（1帧）
该命令读取当前电机的温度、电压和错误状态标志

| 数据域 | 说明 | 数据 |
| --- | --- | --- |
| DATA[0] | 命令字节 | 0x9A |
| DATA[1] | NULL | 0x00 |
| DATA[2] | NULL | 0x00 |
| DATA[3] | NULL | 0x00 |
| DATA[4] | NULL | 0x00 |
| DATA[5] | NULL | 0x00 |
| DATA[6] | NULL | 0x00 |
| DATA[7] | NULL | 0x00 |

#### 驱动回复（1帧）
电机在收到命令后回复主机，该帧数据包含了以下参数：
1. 电机温度temperature（int8_t类型，单位1℃/LSB）
2. 母线电压voltage（uint16_t类型，单位0.01V/LSB）
3. 母线电流current（uint16_t类型，单位0.01A/LSB）
4. 电机状态motorState（uint8_t类型，各个位代表不同的电机状态）
5. 错误标志errorState（uint8_t类型，各个位代表不同的电机错误状态）

| 数据域 | 说明 | 数据 |
| --- | --- | --- |
| DATA[0] | 命令字节 | 0x9A |
| DATA[1] | 电机温度 | DATA[1] = *(uint8_t *)(&temperature) |
| DATA[2] | 母线电压低字节 | DATA[2] = *(uint8_t *)(&voltage) |
| DATA[3] | 母线电压高字节 | DATA[3] = *((uint8_t *)(&voltage)+1) |
| DATA[4] | 母线电流低字节 | DATA[4] = *(uint8_t *)(&current) |
| DATA[5] | 母线电流高字节 | DATA[5] = *((uint8_t *)(&current)+1) |
| DATA[6] | 电机状态字节 | DATA[6] = motorState |
| DATA[7] | 错误状态字节 | DATA[7] = errorState |

**备注**：
1. motorState = 0x00 电机处于开启状态；motorState = 0x10 电机处于关闭状态
2. errorState各位具体状态表

| errorState位 | 状态说明 | 0 | 1 |
| --- | --- | --- | --- |
| 0 | 电压状态 | 电压正常 | 低压保护 |
| 1 | 无效 | - | - |
| 2 | 无效 | - | - |
| 3 | 温度状态 | 温度正常 | 过温保护 |
| 4 | 无效 | - | - |
| 5 | 无效 | - | - |
| 6 | 无效 | - | - |
| 7 | 无效 | - | - |

---

### 2. 清除电机错误标志命令（1帧）
该命令清除当前电机的错误状态，电机收到后返回

| 数据域 | 说明 | 数据 |
| --- | --- | --- |
| DATA[0] | 命令字节 | 0x9B |
| DATA[1] | NULL | 0x00 |
| DATA[2] | NULL | 0x00 |
| DATA[3] | NULL | 0x00 |
| DATA[4] | NULL | 0x00 |
| DATA[5] | NULL | 0x00 |
| DATA[6] | NULL | 0x00 |
| DATA[7] | NULL | 0x00 |

#### 驱动回复（1帧）
电机在收到命令后回复主机。回复数据和读取电机状态1和错误标志命令相同（仅命令字节DATA[0]不同，这里为0x9B）

**备注**：
1. 电机状态没有恢复正常时，错误标志无法清除

---

### 3. 读取电机状态2命令（1帧）
该命令读取当前电机的温度、电机转矩电流（K、TS）/电机输出功率（L）、转速、编码器位置

| 数据域 | 说明 | 数据 |
| --- | --- | --- |
| DATA[0] | 命令字节 | 0x9C |
| DATA[1] | NULL | 0x00 |
| DATA[2] | NULL | 0x00 |
| DATA[3] | NULL | 0x00 |
| DATA[4] | NULL | 0x00 |
| DATA[5] | NULL | 0x00 |
| DATA[6] | NULL | 0x00 |
| DATA[7] | NULL | 0x00 |

#### 驱动回复（1帧）
电机在收到命令后回复主机，该帧数据中包含了以下参数：
1. 电机温度temperature（int8_t类型，1℃/LSB）
2. K、TS电机的转矩电流值iq或L电机的输出功率值power，int16_t类型。TS电机iq分辨率为(66/4096 A)/LSB；K电机iq分辨率为(33/4096 A)/LSB。L电机power范围-1000~1000
3. 电机转速speed（int16_t类型，1dps/LSB）
4. 编码器位置值encoder（uint16_t类型，14bit编码器0~16383，16bit编码器0~65535）

| 数据域 | 说明 | 数据 |
| --- | --- | --- |
| DATA[0] | 命令字节 | 0x9C |
| DATA[1] | 电机温度 | DATA[1] = *(uint8_t *)(&temperature) |
| DATA[2] | 转矩电流低字节/输出功率低字节（L系列） | DATA[2] = *(uint8_t *)(&iq)/DATA[2] = *(uint8_t *)(&power) |
| DATA[3] | 转矩电流高字节/输出功率高字节（L系列） | DATA[3] = *((uint8_t *)(&iq)+1)/DATA[3] = *((uint8_t *)(&power)+1) |
| DATA[4] | 电机速度低字节 | DATA[4] = *(uint8_t *)(&speed) |
| DATA[5] | 电机速度高字节 | DATA[5] = *((uint8_t *)(&speed)+1) |
| DATA[6] | 编码器位置低字节 | DATA[6] = *(uint8_t *)(&encoder) |
| DATA[7] | 编码器位置高字节 | DATA[7] = *((uint8_t *)(&encoder)+1) |

---

### 4. 读取电机状态3命令（1帧）
由于L电机没有相电流采样，该命令在L电机上无作用。该命令读取当前电机的温度和3相电流数据

| 数据域 | 说明 | 数据 |
| --- | --- | --- |
| DATA[0] | 命令字节 | 0x9D |
| DATA[1] | NULL | 0x00 |
| DATA[2] | NULL | 0x00 |
| DATA[3] | NULL | 0x00 |
| DATA[4] | NULL | 0x00 |
| DATA[5] | NULL | 0x00 |
| DATA[6] | NULL | 0x00 |
| DATA[7] | NULL | 0x00 |

#### 驱动回复（1帧）
电机在收到命令后回复主机，该帧数据包含了以下数据：
1. 电机温度temperature（int8_t类型，1℃/LSB）
2. 相电流数据iA、iB、iC，int16_t类型，TS电机相电流分辨率(66/4096 A)/LSB；K电机(33/4096 A)/LSB

| 数据域 | 说明 | 数据 |
| --- | --- | --- |
| DATA[0] | 命令字节 | 0x9D |
| DATA[1] | 电机温度 | DATA[1] = *(uint8_t *)(&temperature) |
| DATA[2] | A相电流低字节 | DATA[2] = *(uint8_t *)(&iA) |
| DATA[3] | A相电流高字节 | DATA[3] = *((uint8_t *)(&iA)+1) |
| DATA[4] | B相电流低字节 | DATA[4] = *(uint8_t *)(&iB) |
| DATA[5] | B相电流高字节 | DATA[5] = *((uint8_t *)(&iB)+1) |
| DATA[6] | C相电流低字节 | DATA[6] = *(uint8_t *)(&iC) |
| DATA[7] | C相电流高字节 | DATA[7] = *((uint8_t *)(&iC)+1) |

---

### 5. 电机关闭命令
将电机从开启状态（上电后默认状态）切换到关闭状态，清除电机转动圈数及之前接收的控制指令，LED由常亮转为慢闪。此时电机仍然可以回复控制命令，但不会执行动作

| 数据域 | 说明 | 数据 |
| --- | --- | --- |
| DATA[0] | 命令字节 | 0x80 |
| DATA[1] | NULL | 0x00 |
| DATA[2] | NULL | 0x00 |
| DATA[3] | NULL | 0x00 |
| DATA[4] | NULL | 0x00 |
| DATA[5] | NULL | 0x00 |
| DATA[6] | NULL | 0x00 |
| DATA[7] | NULL | 0x00 |

#### 驱动回复
和主机发送相同

---

### 6. 电机运行命令
将电机从关闭状态切换到开启状态，LED由慢闪转为常亮。此时再发送控制指令即可控制电机动作

| 数据域 | 说明 | 数据 |
| --- | --- | --- |
| DATA[0] | 命令字节 | 0x88 |
| DATA[1] | NULL | 0x00 |
| DATA[2] | NULL | 0x00 |
| DATA[3] | NULL | 0x00 |
| DATA[4] | NULL | 0x00 |
| DATA[5] | NULL | 0x00 |
| DATA[6] | NULL | 0x00 |
| DATA[7] | NULL | 0x00 |

#### 驱动回复
和主机发送相同

---

### 7. 电机停止命令
停止电机，但不清除电机运行状态。再次发送控制指令即可控制电机动作

| 数据域 | 说明 | 数据 |
| --- | --- | --- |
| DATA[0] | 命令字节 | 0x81 |
| DATA[1] | NULL | 0x00 |
| DATA[2] | NULL | 0x00 |
| DATA[3] | NULL | 0x00 |
| DATA[4] | NULL | 0x00 |
| DATA[5] | NULL | 0x00 |
| DATA[6] | NULL | 0x00 |
| DATA[7] | NULL | 0x00 |

#### 驱动回复（1帧）
和主机发送相同

---

### 8. 抱闸器控制命令
控制抱闸器的开合，或者读取当前抱闸器的状态

| 数据域 | 说明 | 数据 |
| --- | --- | --- |
| DATA[0] | 命令字节 | 0x8C |
| DATA[1] | 抱闸器状态控制和读取字节 | 0x00：抱闸器断电，刹车启动；0x01：抱闸器通电，刹车释放；0x10：读取抱闸器状态 |
| DATA[2] | NULL | 0x00 |
| DATA[3] | NULL | 0x00 |
| DATA[4] | NULL | 0x00 |
| DATA[5] | NULL | 0x00 |
| DATA[6] | NULL | 0x00 |
| DATA[7] | NULL | 0x00 |

#### 驱动回复（1帧）
| 数据域 | 说明 | 数据 |
| --- | --- | --- |
| DATA[0] | 命令字节 | 0x8C |
| DATA[1] | 抱闸器状态字节 | 0x00：抱闸器断电，刹车启动；0x01：抱闸器通电，刹车释放 |
| DATA[2] | NULL | 0x00 |
| DATA[3] | NULL | 0x00 |
| DATA[4] | NULL | 0x00 |
| DATA[5] | NULL | 0x00 |
| DATA[6] | NULL | 0x00 |
| DATA[7] | NULL | 0x00 |

---

### 9. 开环控制命令（仅L电机实现，其他电机无效）
主机发送该命令以控制输出到电机的开环电压，控制值powerControl为int16_t类型，数值范围-850~850（电机电流和扭矩因电机而异）

| 数据域 | 说明 | 数据 |
| --- | --- | --- |
| DATA[0] | 命令字节 | 0xA0 |
| DATA[1] | NULL | 0x00 |
| DATA[2] | NULL | 0x00 |
| DATA[3] | NULL | 0x00 |
| DATA[4] | 开环控制值低字节 | DATA[4] = *(uint8_t *)(&powerControl) |
| DATA[5] | 开环控制值高字节 | DATA[5] = *((uint8_t *)(&powerControl)+1) |
| DATA[6] | NULL | 0x00 |
| DATA[7] | NULL | 0x00 |

**备注**：
1. 该命令中的控制值powerControl不受上位机中的Max Power值限制

#### 驱动回复（1帧）
回复包含：温度、输出功率、转速、编码器位置

| 数据域 | 说明 | 数据 |
| --- | --- | --- |
| DATA[0] | 命令字节 | 0xA0 |
| DATA[1] | 电机温度 | DATA[1] = *(uint8_t *)(&temperature) |
| DATA[2] | 输出功率低字节 | DATA[2] = *(uint8_t *)(&power) |
| DATA[3] | 输出功率高字节 | DATA[3] = *((uint8_t *)(&power)+1) |
| DATA[4] | 电机速度低字节 | DATA[4] = *(uint8_t *)(&speed) |
| DATA[5] | 电机速度高字节 | DATA[5] = *((uint8_t *)(&speed)+1) |
| DATA[6] | 编码器位置低字节 | DATA[6] = *(uint8_t *)(&encoder) |
| DATA[7] | 编码器位置高字节 | DATA[7] = *((uint8_t *)(&encoder)+1) |

---

### 10. 转矩闭环控制命令（仅K、ZH、TS电机实现）
主机发送该命令以控制电机的转矩电流输出，控制值iqControl为int16_t类型，范围-2048~2048；K电机对应-16.5A~16.5A，TS电机对应-33A~33A

| 数据域 | 说明 | 数据 |
| --- | --- | --- |
| DATA[0] | 命令字节 | 0xA1 |
| DATA[1] | NULL | 0x00 |
| DATA[2] | NULL | 0x00 |
| DATA[3] | NULL | 0x00 |
| DATA[4] | 转矩电流控制值低字节 | DATA[4] = *(uint8_t *)(&iqControl) |
| DATA[5] | 转矩电流控制值高字节 | DATA[5] = *((uint8_t *)(&iqControl)+1) |
| DATA[6] | NULL | 0x00 |
| DATA[7] | NULL | 0x00 |

**备注**：
1. 该命令中的控制值iqControl不受上位机中的Max Torque Current值限制

#### 驱动回复
回复包含：温度、转矩电流、转速、编码器位置

| 数据域 | 说明 | 数据 |
| --- | --- | --- |
| DATA[0] | 命令字节 | 0xA1 |
| DATA[1] | 电机温度 | DATA[1] = *(uint8_t *)(&temperature) |
| DATA[2] | 转矩电流低字节 | DATA[2] = *(uint8_t *)(&iq) |
| DATA[3] | 转矩电流高字节 | DATA[3] = *((uint8_t *)(&iq)+1) |
| DATA[4] | 电机速度低字节 | DATA[4] = *(uint8_t *)(&speed) |
| DATA[5] | 电机速度高字节 | DATA[5] = *((uint8_t *)(&speed)+1) |
| DATA[6] | 编码器位置低字节 | DATA[6] = *(uint8_t *)(&encoder) |
| DATA[7] | 编码器位置高字节 | DATA[7] = *((uint8_t *)(&encoder)+1) |

---

### 11. 速度闭环控制命令（1帧）
控制电机速度，0.01dps/LSB；speedControl为int32_t类型

| 数据域 | 说明 | 数据 |
| --- | --- | --- |
| DATA[0] | 命令字节 | 0xA2 |
| DATA[1] | NULL | 0x00 |
| DATA[2] | NULL | 0x00 |
| DATA[3] | NULL | 0x00 |
| DATA[4] | 速度控制低字节 | DATA[4] = *(uint8_t *)(&speedControl) |
| DATA[5] | 速度控制 | DATA[5] = *((uint8_t *)(&speedControl)+1) |
| DATA[6] | 速度控制 | DATA[6] = *((uint8_t *)(&speedControl)+2) |
| DATA[7] | 速度控制高字节 | DATA[7] = *((uint8_t *)(&speedControl)+3) |

**备注**：
1. 受Max Speed限制
2. 受Max Acceleration限制
3. K/ZH/TS受Max Torque Current限制；L受Max Power限制

#### 驱动回复（1帧）
L电机同开环回复；K/ZH/TS同转矩闭环回复（命令字节改为0xA2）

---

### 12. 多圈位置闭环控制命令1（1帧）
控制多圈角度，angleControl为int32_t，0.01°/LSB，36000=360°

| 数据域 | 说明 | 数据 |
| --- | --- | --- |
| DATA[0] | 命令字节 | 0xA3 |
| DATA[1] | NULL | 0x00 |
| DATA[2] | NULL | 0x00 |
| DATA[3] | NULL | 0x00 |
| DATA[4] | 位置控制低字节 | DATA[4] = *(uint8_t *)(&angleControl) |
| DATA[5] | 位置控制 | DATA[5] = *((uint8_t *)(&angleControl)+1) |
| DATA[6] | 位置控制 | DATA[6] = *((uint8_t *)(&angleControl)+2) |
| DATA[7] | 位置控制高字节 | DATA[7] = *((uint8_t *)(&angleControl)+3) |

**备注**：受Max Angle、Max Speed、Max Acceleration、限流/限功率限制

#### 驱动回复（1帧）
L电机同开环；K/ZH/TS同转矩闭环（命令字节0xA3）

---

### 13. 多圈位置闭环控制命令2（1帧）
多圈位置控制+独立限速，maxSpeed为uint16_t，1dps/LSB

| 数据域 | 说明 | 数据 |
| --- | --- | --- |
| DATA[0] | 命令字节 | 0xA4 |
| DATA[1] | NULL | 0x00 |
| DATA[2] | 速度限制低字节 | DATA[2] = *(uint8_t *)(&maxSpeed) |
| DATA[3] | 速度限制高字节 | DATA[3] = *((uint8_t *)(&maxSpeed)+1) |
| DATA[4] | 位置控制低字节 | DATA[4] = *(uint8_t *)(&angleControl) |
| DATA[5] | 位置控制 | DATA[5] = *((uint8_t *)(&angleControl)+1) |
| DATA[6] | 位置控制 | DATA[6] = *((uint8_t *)(&angleControl)+2) |
| DATA[7] | 位置控制高字节 | DATA[7] = *((uint8_t *)(&angleControl)+3) |

#### 驱动回复（1帧）
L电机同开环；K/ZH/TS同转矩闭环（命令字节0xA4）

---

### 14. 单圈位置闭环控制命令1（1帧）
单圈位置控制，指定方向；spinDirection：0x00顺时针，0x01逆时针

| 数据域 | 说明 | 数据 |
| --- | --- | --- |
| DATA[0] | 命令字节 | 0xA5 |
| DATA[1] | 转动方向字节 | DATA[1] = spinDirection |
| DATA[2] | NULL | 0x00 |
| DATA[3] | NULL | 0x00 |
| DATA[4] | 位置控制字节1 | DATA[4] = *(uint8_t *)(&angleControl) |
| DATA[5] | 位置控制字节2 | DATA[5] = *((uint8_t *)(&angleControl)+1) |
| DATA[6] | 位置控制字节3 | DATA[6] = *((uint8_t *)(&angleControl)+2) |
| DATA[7] | 位置控制字节4 | DATA[7] = *((uint8_t *)(&angleControl)+3) |

#### 驱动回复（1帧）
L电机同开环；K/ZH/TS同转矩闭环（命令字节0xA5）

---

### 15. 单圈位置闭环控制命令2（1帧）
单圈位置控制+方向+独立限速

| 数据域 | 说明 | 数据 |
| --- | --- | --- |
| DATA[0] | 命令字节 | 0xA6 |
| DATA[1] | 转动方向字节 | DATA[1] = spinDirection |
| DATA[2] | 速度限制字节1 | DATA[2] = *(uint8_t *)(&maxSpeed) |
| DATA[3] | 速度限制字节2 | DATA[3] = *((uint8_t *)(&maxSpeed)+1) |
| DATA[4] | 位置控制字节1 | DATA[4] = *(uint8_t *)(&angleControl) |
| DATA[5] | 位置控制字节2 | DATA[5] = *((uint8_t *)(&angleControl)+1) |
| DATA[6] | 位置控制字节3 | DATA[6] = *((uint8_t *)(&angleControl)+2) |
| DATA[7] | 位置控制字节4 | DATA[7] = *((uint8_t *)(&angleControl)+3) |

#### 驱动回复（1帧）
L电机同开环；K/ZH/TS同转矩闭环（命令字节0xA6）

---

### 16. 增量位置闭环控制命令1（1帧）
位置增量控制，angleIncrement为int32_t，0.01°/LSB，方向由符号决定

| 数据域 | 说明 | 数据 |
| --- | --- | --- |
| DATA[0] | 命令字节 | 0xA7 |
| DATA[1] | NULL | 0x00 |
| DATA[2] | NULL | 0x00 |
| DATA[3] | NULL | 0x00 |
| DATA[4] | 位置控制低字节 | DATA[4] = *(uint8_t *)(&angleIncrement) |
| DATA[5] | 位置控制 | *((uint8_t *)(&angleIncrement)+1) |
| DATA[6] | 位置控制 | *((uint8_t *)(&angleIncrement)+2) |
| DATA[7] | 位置控制高字节 | *((uint8_t *)(&angleIncrement)+3) |

#### 驱动回复（1帧）
L电机同开环；K/ZH/TS同转矩闭环（命令字节0xA7）

---

### 17. 增量位置闭环控制命令2（1帧）
位置增量控制+独立限速

| 数据域 | 说明 | 数据 |
| --- | --- | --- |
| DATA[0] | 命令字节 | 0xA8 |
| DATA[1] | NULL | 0x00 |
| DATA[2] | 速度限制低字节 | DATA[2] = *(uint8_t *)(&maxSpeed) |
| DATA[3] | 速度限制高字节 | DATA[3] = *((uint8_t *)(&maxSpeed)+1) |
| DATA[4] | 位置控制低字节 | DATA[4] = *(uint8_t *)(&angleIncrement) |
| DATA[5] | 位置控制 | DATA[5] = *((uint8_t *)(&angleIncrement)+1) |
| DATA[6] | 位置控制 | DATA[6] = *((uint8_t *)(&angleIncrement)+2) |
| DATA[7] | 位置控制高字节 | DATA[7] = *((uint8_t *)(&angleIncrement)+3) |

#### 驱动回复（1帧）
L电机同开环；K/ZH/TS同转矩闭环（命令字节0xA8）

---

### 18. 读取PID参数命令（1帧）

| 数据域 | 说明 | 数据 |
| --- | --- | --- |
| DATA[0] | 命令字节 | 0x30 |
| DATA[1] | NULL | 0x00 |
| DATA[2] | NULL | 0x00 |
| DATA[3] | NULL | 0x00 |
| DATA[4] | NULL | 0x00 |
| DATA[5] | NULL | 0x00 |
| DATA[6] | NULL | 0x00 |
| DATA[7] | NULL | 0x00 |

#### 驱动回复（1帧）
| 数据域 | 说明 | 数据 |
| --- | --- | --- |
| DATA[0] | 命令字节 | 0x30 |
| DATA[1] | NULL | 0x00 |
| DATA[2] | 位置环P参数 | DATA[2] = anglePidKp |
| DATA[3] | 位置环I参数 | DATA[3] = anglePidKi |
| DATA[4] | 速度环P参数 | DATA[4] = speedPidKp |
| DATA[5] | 速度环I参数 | DATA[5] = speedPidKi |
| DATA[6] | 转矩环P参数 | DATA[6] = iqPidKp |
| DATA[7] | 转矩环I参数 | DATA[7] = iqPidKi |

---

### 19. 写入PID参数到RAM命令（1帧）
断电失效

| 数据域 | 说明 | 数据 |
| --- | --- | --- |
| DATA[0] | 命令字节 | 0x31 |
| DATA[1] | NULL | 0x00 |
| DATA[2] | 位置环P参数 | DATA[2] = anglePidKp |
| DATA[3] | 位置环I参数 | DATA[3] = anglePidKi |
| DATA[4] | 速度环P参数 | DATA[4] = speedPidKp |
| DATA[5] | 速度环I参数 | DATA[5] = speedPidKi |
| DATA[6] | 转矩环P参数 | DATA[6] = iqPidKp |
| DATA[7] | 转矩环I参数 | DATA[7] = iqPidKi |

#### 驱动回复
与发送命令一致

---

### 20. 写入PID参数到ROM命令（1帧）
断电保持

| 数据域 | 说明 | 数据 |
| --- | --- | --- |
| DATA[0] | 命令字节 | 0x32 |
| DATA[1] | NULL | 0x00 |
| DATA[2] | 位置环P参数 | DATA[2] = anglePidKp |
| DATA[3] | 位置环I参数 | DATA[3] = anglePidKi |
| DATA[4] | 速度环P参数 | DATA[4] = speedPidKp |
| DATA[5] | 速度环I参数 | DATA[5] = speedPidKi |
| DATA[6] | 转矩环P参数 | DATA[6] = iqPidKp |
| DATA[7] | 转矩环I参数 | DATA[7] = iqPidKi |

#### 驱动回复
与发送命令一致

---

### 21. 读取加速度命令（1帧）

| 数据域 | 说明 | 数据 |
| --- | --- | --- |
| DATA[0] | 命令字节 | 0x33 |
| DATA[1] | NULL | 0x00 |
| DATA[2] | NULL | 0x00 |
| DATA[3] | NULL | 0x00 |
| DATA[4] | NULL | 0x00 |
| DATA[5] | NULL | 0x00 |
| DATA[6] | NULL | 0x00 |
| DATA[7] | NULL | 0x00 |

#### 驱动回复（1帧）
加速度Accel为int32_t，单位1dps/s

| 数据域 | 说明 | 数据 |
| --- | --- | --- |
| DATA[0] | 命令字节 | 0x33 |
| DATA[1] | NULL | 0x00 |
| DATA[2] | NULL | 0x00 |
| DATA[3] | NULL | 0x00 |
| DATA[4] | 加速度低字节1 | DATA[4] = *(uint8_t *)(&Accel) |
| DATA[5] | 加速度字节2 | DATA[5] = *((uint8_t *)(&Accel)+1) |
| DATA[6] | 加速度字节3 | DATA[6] = *((uint8_t *)(&Accel)+2) |
| DATA[7] | 加速度字节4 | DATA[7] = *((uint8_t *)(&Accel)+3) |

---

### 22. 写入加速度到RAM命令（1帧）
断电失效，Accel为int32_t，1dps/s

| 数据域 | 说明 | 数据 |
| --- | --- | --- |
| DATA[0] | 命令字节 | 0x34 |
| DATA[1] | NULL | 0x00 |
| DATA[2] | NULL | 0x00 |
| DATA[3] | NULL | 0x00 |
| DATA[4] | 加速度低字节1 | DATA[4] = *(uint8_t *)(&Accel) |
| DATA[5] | 加速度字节2 | DATA[5] = *((uint8_t *)(&Accel)+1) |
| DATA[6] | 加速度字节3 | DATA[6] = *((uint8_t *)(&Accel)+2) |
| DATA[7] | 加速度字节4 | DATA[7] = *((uint8_t *)(&Accel)+3) |

#### 驱动回复
与发送命令一致

---

### 23. 读取编码器数据命令（1帧）

| 数据域 | 说明 | 数据 |
| --- | --- | --- |
| DATA[0] | 命令字节 | 0x90 |
| DATA[1] | NULL | 0x00 |
| DATA[2] | NULL | 0x00 |
| DATA[3] | NULL | 0x00 |
| DATA[4] | NULL | 0x00 |
| DATA[5] | NULL | 0x00 |
| DATA[6] | NULL | 0x00 |
| DATA[7] | NULL | 0x00 |

#### 驱动回复（1帧）
包含：编码器位置、原始位置、零偏

| 数据域 | 说明 | 数据 |
| --- | --- | --- |
| DATA[0] | 命令字节 | 0x90 |
| DATA[1] | NULL | 0x00 |
| DATA[2] | 编码器位置低字节 | DATA[2] = *(uint8_t *)(&encoder) |
| DATA[3] | 编码器位置高字节 | DATA[3] = *((uint8_t *)(&encoder)+1) |
| DATA[4] | 编码器原始位置低字节 | DATA[4] = *(uint8_t *)(&encoderRaw) |
| DATA[5] | 编码器原始位置高字节 | DATA[5] = *((uint8_t *)(&encoderRaw)+1) |
| DATA[6] | 编码器零偏低字节 | DATA[6] = *(uint8_t *)(&encoderOffset) |
| DATA[7] | 编码器零偏高字节 | DATA[7] = *((uint8_t *)(&encoderOffset)+1) |

---

### 24. 写入编码器值到ROM作为电机零点命令（1帧）

| 数据域 | 说明 | 数据 |
| --- | --- | --- |
| DATA[0] | 命令字节 | 0x91 |
| DATA[1] | NULL | 0x00 |
| DATA[2] | NULL | 0x00 |
| DATA[3] | NULL | 0x00 |
| DATA[4] | NULL | 0x00 |
| DATA[5] | NULL | 0x00 |
| DATA[6] | 编码器零偏低字节 | DATA[6] = *(uint8_t *)(&encoderOffset) |
| DATA[7] | 编码器零偏高字节 | DATA[7] = *((uint8_t *)(&encoderOffset)+1) |

#### 驱动回复
与发送命令一致

---

### 25. 写入当前位置到ROM作为电机零点命令（1帧）
将当前编码器位置写入ROM作为零点，**需重新上电生效，不建议频繁写入**

| 数据域 | 说明 | 数据 |
| --- | --- | --- |
| DATA[0] | 命令字节 | 0x19 |
| DATA[1] | NULL | 0x00 |
| DATA[2] | NULL | 0x00 |
| DATA[3] | NULL | 0x00 |
| DATA[4] | NULL | 0x00 |
| DATA[5] | NULL | 0x00 |
| DATA[6] | 编码器零偏低字节 | DATA[6] = *(uint8_t *)(&encoderOffset) |
| DATA[7] | 编码器零偏高字节 | DATA[7] = *((uint8_t *)(&encoderOffset)+1) |

#### 驱动回复
返回设置的零偏值

---

### 26. 读取多圈角度命令（1帧）

| 数据域 | 说明 | 数据 |
| --- | --- | --- |
| DATA[0] | 命令字节 | 0x92 |
| DATA[1] | NULL | 0x00 |
| DATA[2] | NULL | 0x00 |
| DATA[3] | NULL | 0x00 |
| DATA[4] | NULL | 0x00 |
| DATA[5] | NULL | 0x00 |
| DATA[6] | NULL | 0x00 |
| DATA[7] | NULL | 0x00 |

#### 驱动回复（1帧）
motorAngle为int64_t，0.01°/LSB

| 数据域 | 说明 | 数据 |
| --- | --- | --- |
| DATA[0] | 命令字节 | 0x92 |
| DATA[1] | 角度低字节1 | DATA[1] = *(uint8_t *)(&motorAngle) |
| DATA[2] | 角度字节2 | DATA[2] = *((uint8_t *)(&motorAngle)+1) |
| DATA[3] | 角度字节3 | DATA[3] = *((uint8_t *)(&motorAngle)+2) |
| DATA[4] | 角度字节4 | DATA[4] = *((uint8_t *)(&motorAngle)+3) |
| DATA[5] | 角度字节5 | DATA[5] = *((uint8_t *)(&motorAngle)+4) |
| DATA[6] | 角度字节6 | DATA[6] = *((uint8_t *)(&motorAngle)+5) |
| DATA[7] | 角度字节7 | DATA[7] = *((uint8_t *)(&motorAngle)+6) |

---

### 27. 读取单圈角度命令（1帧）

| 数据域 | 说明 | 数据 |
| --- | --- | --- |
| DATA[0] | 命令字节 | 0x94 |
| DATA[1] | NULL | 0x00 |
| DATA[2] | NULL | 0x00 |
| DATA[3] | NULL | 0x00 |
| DATA[4] | NULL | 0x00 |
| DATA[5] | NULL | 0x00 |
| DATA[6] | NULL | 0x00 |
| DATA[7] | NULL | 0x00 |

#### 驱动回复（1帧）
circleAngle为uint32_t，0.01°/LSB，0~36000×减速比-1

| 数据域 | 说明 | 数据 |
| --- | --- | --- |
| DATA[0] | 命令字节 | 0x94 |
| DATA[1] | NULL | 0x00 |
| DATA[2] | NULL | 0x00 |
| DATA[3] | NULL | 0x00 |
| DATA[4] | 单圈角度低字节1 | DATA[4] = *(uint8_t *)(&circleAngle) |
| DATA[5] | 单圈角度字节2 | DATA[5] = *((uint8_t *)(&circleAngle)+1) |
| DATA[6] | 单圈角度字节3 | DATA[6] = *((uint8_t *)(&circleAngle)+2) |
| DATA[7] | 单圈角度高字节4 | DATA[7] = *((uint8_t *)(&circleAngle)+3) |

---

### 28. 清除电机角度命令（1帧）暂未实现
清除多圈/单圈角度，当前位置设为零点，断电失效

| 数据域 | 说明 | 数据 |
| --- | --- | --- |
| DATA[0] | 命令字节 | 0x95 |
| DATA[1] | NULL | 0x00 |
| DATA[2] | NULL | 0x00 |
| DATA[3] | NULL | 0x00 |
| DATA[4] | NULL | 0x00 |
| DATA[5] | NULL | 0x00 |
| DATA[6] | NULL | 0x00 |
| DATA[7] | NULL | 0x00 |

#### 驱动回复
与发送命令一致