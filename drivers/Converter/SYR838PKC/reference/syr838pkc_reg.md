
## 1. VSEL0 (0x00)
| Register Name | VSEL0 |
| ---- | ---- |
| Address | 0x00 |

| Field | Bit | R/W | Default | Description |
| ---- | ---- | ---- | ---- | ---- |
| BUCK_EN0 | 7 | R/W | 1 | 软件降压使能。EN引脚为低时，转换器关闭；EN引脚为高时，BUCK_EN位生效 |
| MODE0 | 6 | R/W | 0 | 0=轻载自动PFM模式；1=强制PWM模式 |
| NSEL0 | 5:0 | R/W | 010111 (VOUT=1.0V) | 输出电压配置<br>000000 = 0.7125V<br>000001 = 0.7250V<br>000010 = 0.7375V<br>……<br>010111 = 1.0000V<br>……<br>111111 =1.5000V |

## 2. VSEL1 (0x01)
| Register Name | VSEL1 |
| ---- | ---- |
| Address | 0x01 |

| Field | Bit | R/W | Default | Description |
| ---- | ---- | ---- | ---- | ---- |
| BUCK_EN1 | 7 | R/W | 1 | 软件降压使能。EN引脚为低时，转换器关闭；EN引脚为高时，BUCK_EN位生效 |
| MODE1 | 6 | R/W | 0 | 0=轻载自动PFM模式；1=强制PWM模式 |
| NSEL1 | 5:0 | R/W | 010111 (VOUT=1.0V) | 输出电压配置<br>000000 = 0.7125V<br>000001 = 0.7250V<br>000010 = 0.7375V<br>……<br>010111 = 1.0000V<br>……<br>111111 =1.5000V |

## 3. Control Register (0x02)
| Register Name | Control Register |
| ---- | ---- |
| Address | 0x02 |

| Field | Bit | R/W | Default | Description |
| ---- | ---- | ---- | ---- | ---- |
| Output Discharge | 7 | R/W | 1 | 0=关断输出放电电阻；1=开启输出放电电阻 |
| Slew Rate | 6:4 | R/W | 000=12.5mV/0.15us | 输出电压上升斜率配置<br>000 = 12.5mV/0.15us<br>001 = 12.5mV/0.3us<br>010 = 12.5mV/0.6us<br>011 = 12.5mV/1.2us<br>100 = 12.5mV/2.4us<br>101 = 12.5mV/4.8us<br>110 = 12.5mV/9.6us<br>111 = 12.5mV/19.2us |
| Reserved | 3 | R/W | 0 | 只读恒为0 |
| RESET | 2 | R/W | 0 | 写1将所有寄存器恢复默认值 |
| Reserved | 1:0 | R/W | 00 | 只读恒为0 |

## 4. ID1 Register (0x03)
| Register Name | ID1 Register |
| ---- | ---- |
| Address | 0x03 |

| Field | Bit | R/W | Default | Description |
| ---- | ---- | ---- | ---- | ---- |
| VENDOR | 7:5 | R | 100 | 矽立杰（Silergy）厂商编码 |
| Reserved | 4 | R | 0 | 只读恒为0 |
| DIE_ID | 3:0 | R | 1000 | IC型号选项编码 |

## 5. ID2 Register (0x04)
| Register Name | ID2 Register |
| ---- | ---- |
| Address | 0x04 |

| Field | Bit | R/W | Default | Description |
| ---- | ---- | ---- | ---- | ---- |
| Reserved | 7:4 | R | 0000 | 只读恒为0 |
| DIE_REV | 3:0 | R | 0001 | IC晶圆版本编码 |

## 6. PGOOD Register (0x05)
| Register Name | PGOOD Register |
| ---- | ---- |
| Address | 0x05 |

| Field | Bit | R/W | Default | Description |
| ---- | ---- | ---- | ---- | ---- |
| PGOOD | 7 | R | 0 | 1：降压使能且软启动完成 |
| Reserved | 6:0 | R | 0000000 | 只读恒为0 |

---
文档来源：SILERGY AN_SYR837/SYR838 Rev.0.9A 第7~9页