## BiSS-C 协议时序格式文档

BiSS-C（Bidirectional Serial Synchronous Communication - mode C）是一种用于编码器的高速串行同步通信协议，由 iC-Haus 定义并开源。

### 信号线定义

| 信号 | 方向 | 说明 |
|------|------|------|
| MA   | 主机→从机 | 时钟线（Master Clock） |
| SLO  | 从机→主机 | 数据输出线（Slave Data Out） |
| SLI  | 主机→从机 | 数据输入线（单向模式下不使用） |

### 通信时序

1. **空闲状态**：MA 和 SLO 均为高电平（逻辑 1）
2. **帧启动**：主机在 MA 上产生第一个下降沿，从机在第一个上升沿（LAT）锁存传感器数据
3. **ACK 阶段**：第二个 MA 上升沿后，从机将 SLO 拉低（= 0）作为应答，直到从机准备好发送 Start 位为止
4. **Start 位**：从机发送 1 位 Start 位（固定为 1），表示数据传输开始
5. **CDS 位**：从机发送 1 位 CDS（Control Data Slave）位，用于寄存器/菊花链通信的控制通道
6. **数据传输**：从机依次发送位置数据、状态位和 CRC
7. **Timeout 阶段**：主机停止发送时钟，SLO 在从机完成内部计时后恢复高电平，标志一帧结束

### 数据帧格式

帧内容按传输顺序如下：

```
[ ACK(低) | Start(1) | CDS | 位置数据(n位, MSB first) | nE | nW | CRC(6位, 取反) ]
```

| 字段 | 位数 | 说明 |
|------|------|------|
| ACK  | 不定 | 从机拉低 SLO 的应答阶段，不参与 CRC 计算 |
| Start（STR） | 1 | 固定为 1，不参与 CRC 计算 |
| CDS  | 1 | Control Data Slave，不参与 CRC 计算 |
| 位置数据 | n | n 位绝对位置，MSB 先发；**参与 CRC 计算** |
| nE（错误标志） | 1 | 低有效（0 = 有错误）；**参与 CRC 计算** |
| nW（警告标志） | 1 | 低有效（0 = 有警告）；**参与 CRC 计算** |
| CRC-6 | 6 | 对位置数据+nE+nW 的校验值，**在线路上取反发送** |

> 示例（26 位分辨率编码器）：ACK + Start(1) + CDS(1) + 26位位置 + nE + nW + 6位CRC = 共 36 位有效载荷

### CRC-6 校验规格

- **多项式**：x⁶ + x¹ + x⁰，即 `0x43`（二进制 `1000011`）
- **初始种子**：`0b000000`（全 0）
- **覆盖范围**：位置数据 + nE + nW（不含 ACK、Start、CDS）
- **发送规则**：从机将计算结果**逐位取反**后发送至 SLO
- **验证规则**：主机将接收到的 CRC 取反后，与自行计算的值比较


### 关键特性

- 最高时钟频率：**10 MHz**（实际受线缆长度和从机规格限制，部分器件限制在 2.5 MHz）
- 数据位宽可变，由从机分辨率决定
- 支持线路延迟补偿（通过 Timeout 同步机制）
- 采用主从架构，支持点对点或菊花链（Daisy-Chain）多从机连接
- Timeout 阶段 MA 的电平即为 CDM（Control Data Master）位，用于寄存器读写通信
- 控制帧使用独立的 CRC-4（多项式 `0x13`），与位置数据 CRC-6 相互独立

### 参考资料

- [BiSS-C Protocol Description Rev D2（官方规范）](https://biss-interface.com/wp-content/uploads/2024/07/BiSS_C_protocol_D2en.pdf)
- [BiSS-C Unidirectional Protocol Rev A3](https://biss-interface.com/wp-content/uploads/2024/07/BiSS_C_unidirectional_protocol_A3en.pdf)
- [TI BiSS-C Firmware Reference TIDU794](https://www.ti.com/lit/pdf/tidu794)
- [RLS CRC Application Note CRCD01](https://www.rls.si/media/custom/upload/CRCD01_03.pdf)


