# MP2980 规格书 Page30~Page36 寄存器说明
文档版本：MP2980 Rev.1.0 11/9/2023
## 目录
1. 寄存器总表（Page30）
2. REF_LSB (00h)（Page31）
3. REF_MSB (01h)（Page31）
4. CONTROL1 (02h)（Page32~33）
5. CONTROL2 (03h)（Page34）
6. ILIM (04h)（Page35）
7. INTERRUPT_STATUS (05h)（Page35）
8. INTERRUPT_MASK (06h)（Page36）

---

## 一、寄存器总表 Page30
| 地址Addr | 寄存器名 | 读写类型 | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 | 上电默认值 |
| ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- |
| 00h | REF_LSB | R/W | - | - | - | - | - | VREF_L[2:0] | 0000 0100 |
| 01h | REF_MSB | R/W | VREF_H[7:0] | 0011 1110 |
| 02h | CONTROL1 | R/W | SR[1:0] | DISCHG | DITHER | PNG_LATCH | RESERVED | GO_BIT | ENPWR | 0100 010x（x由外部引脚上电配置） |
| 03h | CONTROL2 | R/W | FSW[1:0] | - | BB_FSW | OCP_MODE[1:0] | OVP_MODE[1:0] | xx00 0101（x由外部引脚上电配置） |
| 04h | ILIM | R/W | - | - | - | - | ILIM[2:0] | 0000 1xxx（x由ILIM引脚上电配置） |
| 05h | INTERRUPT_STATUS | R/W | - | - | - | OTP | - | OVP | OCP | PNG | 0000 0000 |
| 06h | INTERRUPT_MASK | R/W | - | - | - | M_OTP | - | M_OVP | M_OCP | M_PNG | 0000 0001 |

> 备注：
> 1. Reserved 保留位，应用中禁止写入任意数值；
> 2. 标记`x`代表上电默认值由外部硬件引脚（FS/ILIM/ADDR）配置，软件上电后可覆盖修改。

---

## 二、REF_LSB (00h) Page31
### 功能
配置反馈基准电压VREF低3位，1LSB=1mV。
| Bit位 | 读写 | 位名称 | 默认值 | 功能描述 |
| ---- | ---- | ---- | ---- | ---- |
| D2:D0 | R/W | VREF_L | 3'b100 | VREF电压低3位，分辨率1mV |

## 三、REF_MSB (01h) Page31
### 功能
配置反馈基准电压VREF高8位，与REF_LSB拼接为11位无符号整数，计算公式：
$$V_{REF}(V) = \frac{VREF[10:0]}{1000}$$
- VREF_H：高8位，1LSB=8mV
- VREF_L：低3位，1LSB=1mV

| Bit位 | 读写 | 位名称 | 默认值 | 功能描述 |
| ---- | ---- | ---- | ---- | ---- |
| D7:D0 | R/W | VREF_H | 8'b00111110 | VREF电压高8位 |

默认值组合：REF_MSB=00111110，REF_LSB=100 → VREF=500mV=0.5V

---

## 四、CONTROL1 (02h) Page32~33
寄存器作用：VREF电压变化斜率、输出放电、扩频、PNG锁存、电压切换使能、功率开关总使能
| Bit位 | 读写 | 位名称 | 默认值 | 功能描述 |
| ---- | ---- | ---- | ---- | ---- |
| D7:D6 | R/W | SR | 2'b01 | VREF切换斜率（软启动阶段不生效）<br>00：38mV/ms<br>01：50mV/ms<br>10：75mV/ms<br>11：150mV/ms<br>VOUT实际斜率 = SR斜率 × 分压反馈比 |
| D5 | R/W | DISCHG | 0 | 输出放电使能位 |
| D4 | R/W | DITHER | 0 | 扩频FSS开关<br>0：关闭扩频<br>1：开启扩频 |
| D3 | R/W | PNG_LATCH | 0 | 电源异常PNG锁存控制<br>0：VOUT恢复正常后PNG自动清零<br>1：PNG触发后锁存，需写0xFF清除中断状态 |
| D2 | R/W | RESERVED | 1 | 保留位，固定写1 |
| D1 | R/W | GO_BIT | 0 | VOUT电压切换触发位<br>1：加载REF寄存器新值，VOUT开始升降压；电压到达目标后硬件自动置0<br>使用流程：先改写REF_LSB/REF_MSB → 置GO_BIT=1 → 轮询GO_BIT判断切换完成<br>放电延时20ms，大容量输出电容可延长放电时间 |
| D0 | R/W | ENPWR | x | 功率开关总使能<br>0：关闭功率转换，内部电路保持供电；关断后自动放电200ms，VREF复位至0.5V<br>1：开启升降压功率回路<br>上电默认值由ADDR引脚分压决定 |

### 输出放电触发条件（任意一条满足即开启放电）
1. GO_BIT=1，GO_BIT清零后额外维持20ms放电；
2. DISCHG=1；
3. OVP_MODE=01且FB超过127%VREF；
4. ENPWR由1置0，放电200ms；
5. EN引脚拉低关断，放电200ms；
6. VIN欠压锁存触发，残留AVDD电压下放电200ms。

---

## 五、CONTROL2 (03h) Page34
寄存器作用：开关频率、升降压模式分频、过流保护模式、过压保护模式
| Bit位 | 读写 | 位名称 | 默认值 | 功能描述 |
| ---- | ---- | ---- | ---- | ---- |
| D7:D6 | R/W | FSW | 2'bx0 | 主开关频率，上电由FS引脚配置，运行中可改写<br>00：200kHz<br>01：300kHz<br>10：400kHz<br>11：600kHz |
| D5 | - | - | 0 | 保留 |
| D4 | R/W | BB_FSW | 0 | Buck-Boost区间分频系数<br>0：升降压区间频率=主频率×40%<br>1：升降压区间频率=主频率×80% |
| D3:D2 | R/W | OCP_MODE | 2'b01 | 逐周期限流后保护模式<br>00：无打嗝/锁存，仅逐周期限流<br>01：打嗝保护（FB<60%VREF触发，关断时间由SS电容放电时长决定）<br>10：锁存关断，需重新上电/翻转EN/ENPWR恢复<br>11：保留 |
| D1:D0 | R/W | OVP_MODE | 2'b01 | FB=127%VREF时过压保护模式<br>00：无保护，持续调节，不放电<br>01：内部电阻放电，FB回落至111%VREF恢复<br>10：锁存关断，无放电<br>11：保留 |

---

## 六、ILIM (04h) Page35
寄存器作用：平均输出恒流限流阈值配置，上电默认值由ILIM引脚分压锁存，运行中软件可修改
| Bit位 | 读写 | 位名称 | 默认值 | 阈值电压 |
| ---- | ---- | ---- | ---- | ---- |
| D2:D0 | R/W | ILIM[2:0] | 3'bxxx | 000：27.9mV<br>001：33.3mV（ILIM引脚浮空默认）<br>010：39.3mV<br>011：45.1mV<br>100：51.2mV<br>101：56.8mV<br>110：62.8mV<br>111：68.7mV |

> 说明：仅平均限流触发时不会置OCP中断、不会拉低INT引脚；仅逐周期开关限流才会上报OCP。

---

## 七、INTERRUPT_STATUS (05h) Page35
只读中断状态寄存器，故障位锁存，写入0xFF整寄存器清零所有中断标志与INT引脚电平
| Bit位 | 读写 | 位名称 | 默认值 | 功能说明 | 复位条件 |
| ---- | ---- | ---- | ---- | ---- | ---- |
| D7:D5 | - | - | 0 | 保留 | - |
| D4 | R/W | OTP | 0 | 1=芯片结温超150℃过热保护触发 | 写0xFF清零 |
| D3 | - | - | 0 | 保留 | - |
| D2 | R/W | OVP | 0 | 1=输出过压触发（FB>127%VREF） | 写0xFF清零 |
| D1 | R/W | OCP | 0 | 1=逐周期开关限流触发，且软启动完成、FB<60%VREF | 写0xFF清零 |
| D0 | R/W | PNG | 0 | 1=VOUT超出PG上下阈值<br>PNG_LATCH=0：实时状态；PNG_LATCH=1：锁存 | PNG_LATCH=0：电压恢复自动清零；PNG_LATCH=1：写0xFF清零 |

---

## 八、INTERRUPT_MASK (06h) Page36
中断屏蔽寄存器，屏蔽对应故障时INT引脚不会被拉低，但故障状态位仍会置1
| Bit位 | 读写 | 位名称 | 默认值 | 功能描述 |
| ---- | ---- | ---- | ---- | ---- |
| D7:D5 | - | - | 0 | 保留 |
| D4 | R/W | M_OTP | 0 | 0：过热故障上报INT；1：屏蔽过热中断 |
| D3 | - | - | 0 | 保留 |
| D2 | R/W | M_OVP | 0 | 0：过压故障上报INT；1：屏蔽过压中断 |
| D1 | R/W | M_OCP | 0 | 0：过流故障上报INT；1：屏蔽过流中断 |
| D0 | R/W | M_PNG | 1 | 0：电源异常PNG上报INT；1：默认屏蔽PNG中断 |

> 重要：屏蔽位仅控制INT引脚输出，不影响INTERRUPT_STATUS寄存器内故障标志位。