
---

## 目录
- [消息（Messages）](#消息messages)
- [枚举（Enums）](#枚举enums)

---

## 消息（Messages）

### HEARTBEAT (ID: 0)
**描述：** 心跳包，标识系统/组件在线、类型与状态。
| 字段名 | 类型 | 值（枚举） | 描述 |
|---|---|---|---|
| type | uint8_t | MAV_TYPE | 载具/组件类型（四轴、直升机、相机等） |
| autopilot | uint8_t | MAV_AUTOPILOT | 飞控类型（如 ArduPilot、PX4） |
| base_mode | uint8_t | MAV_MODE_FLAG | 系统模式位图（解锁、武装、手动/自动） |
| custom_mode | uint32_t | - | 飞控自定义模式标识 |
| system_status | uint8_t | MAV_STATE | 系统状态（待机、飞行、故障） |
| mavlink_version | uint8_t | - | MAVLink 版本（协议自动填充） |

---

### SYS_STATUS (ID: 1)
**描述：** 系统核心状态（传感器、电池、负载、通信）。
| 字段名 | 类型 | 单位 | 值（枚举） | 描述 |
|---|---|---|---|---|
| onboard_control_sensors_present | uint32_t | - | MAV_SYS_STATUS_SENSOR | 传感器在位掩码（0=不存在，1=存在） |
| onboard_control_sensors_enabled | uint32_t | - | MAV_SYS_STATUS_SENSOR | 传感器使能掩码（0=关闭，1=开启） |
| onboard_control_sensors_health | uint32_t | - | MAV_SYS_STATUS_SENSOR | 传感器健康掩码（0=故障，1=正常） |
| load | uint16_t | d% | - | 主循环最大占用率（0–1000，建议<1000） |
| voltage_battery | uint16_t | mV | UINT16_MAX=无效 | 电池电压 |
| current_battery | int16_t | cA | -1=无效 | 电池电流 |
| battery_remaining | int8_t | % | -1=无效 | 剩余电量百分比 |
| drop_rate_comm | uint16_t | c% | - | 通信丢包率（UART/I2C/SPI/CAN） |
| errors_comm | uint16_t | - | - | 通信错误数 |
| errors_count1~4 | uint16_t | - | - | 飞控自定义错误码 |
| onboard_control_sensors_present_extended | uint32_t | - | MAV_SYS_STATUS_SENSOR_EXTENDED | 扩展传感器在位掩码 |
| onboard_control_sensors_enabled_extended | uint32_t | - | MAV_SYS_STATUS_SENSOR_EXTENDED | 扩展传感器使能掩码 |
| onboard_control_sensors_health_extended | uint32_t | - | MAV_SYS_STATUS_SENSOR_EXTENDED | 扩展传感器健康掩码 |

---

### SYSTEM_TIME (ID: 2)
**描述：** 系统时间（UNIX 时间+启动时间），用于日志与同步。
| 字段名 | 类型 | 单位 | 描述 |
|---|---|---|---|
| time_unix_usec | uint64_t | us | UNIX 纪元时间戳（微秒） |
| time_boot_ms | uint32_t | ms | 系统启动后时间戳（毫秒） |

---

### PING (ID: 4)
**描述：** 链路时延测试（已废弃，推荐 TIMESYNC）。
| 字段名 | 类型 | 单位 | 描述 |
|---|---|---|---|
| time_usec | uint64_t | us | 时间戳（UNIX 或启动后） |
| seq | uint32_t | - | PING 序列号 |
| target_system | uint8_t | - | 目标系统 ID（0=广播） |
| target_component | uint8_t | - | 目标组件 ID（0=广播） |

---

### CHANGE_OPERATOR_CONTROL (ID: 5)
**描述：** 请求控制此MAV（载具）的权限切换。
| 字段名 | 类型 | 描述 |
|---|---|---|
| target_system | uint8_t | 系统ID，地面站请求控制的载具 |
| control_request | uint8_t | 0: 请求控制此MAV，1: 释放控制权 |
| version | uint8_t | 0: 密钥明文，1-255: 未来不同哈希/加密变体 |
| passkey | char[25] | 密码/密钥，25个字符以内，NULL结尾 |

---

### CHANGE_OPERATOR_CONTROL_ACK (ID: 6)
**描述：** 控制权切换请求的接受/拒绝确认消息。
| 字段名 | 类型 | 描述 |
|---|---|---|
| gcs_system_id | uint8_t | 发送此消息的地面站ID |
| control_request | uint8_t | 0: 请求控制此MAV，1: 释放控制权 |
| ack | uint8_t | 0: 确认，1: 否认-密码错误，2: 否认-不支持的加密方式，3: 否认-已在控制中 |

---

### AUTH_KEY (ID: 7)
**描述：** 发送识别此系统的加密签名/密钥（注意：需通过加密通道传输以确保安全）。
| 字段名 | 类型 | 描述 |
|---|---|---|
| key | char[32] | 加密密钥 |

---

### LINK_NODE_STATUS (ID: 8) - WIP
**描述：** 通信链中各节点的状态信息（开发中，生产环境中慎用）。
| 字段名 | 类型 | 单位 | 描述 |
|---|---|---|---|
| timestamp | uint64_t | ms | 时间戳（系统启动后时间） |
| tx_buf | uint8_t | % | 剩余发送缓冲区空间 |
| rx_buf | uint8_t | % | 剩余接收缓冲区空间 |
| tx_rate | uint32_t | bytes/s | 发送速率 |
| rx_rate | uint32_t | bytes/s | 接收速率 |
| rx_parse_err | uint16_t | bytes | 未能正确解析的字节数 |
| tx_overflows | uint16_t | bytes | 发送缓冲区溢出（达到UINT16_MAX时回绕） |
| rx_overflows | uint16_t | bytes | 接收缓冲区溢出（达到UINT16_MAX时回绕） |
| messages_sent | uint32_t | - | 已发送消息数 |
| messages_received | uint32_t | - | 已接收消息数（估算） |
| messages_lost | uint32_t | - | 丢失消息数（估算） |

---

### SET_MODE (ID: 11) - 已废弃
**描述：** 设置系统模式（已废弃，使用COMMAND_LONG配合MAV_CMD_DO_SET_MODE代替）。
| 字段名 | 类型 | 值（枚举） | 描述 |
|---|---|---|---|
| target_system | uint8_t | - | 设置模式的系统 |
| base_mode | uint8_t | MAV_MODE_FLAG | 新的基础模式 |
| custom_mode | uint32_t | - | 新的飞控自定义模式 |

---

### PARAM_REQUEST_READ (ID: 20)
**描述：** 请求读取指定参数ID的板载参数值（参数存储为键[字符串] -> 值[float]）。
| 字段名 | 类型 | 描述 |
|---|---|---|
| target_system | uint8_t | 目标系统ID |
| target_component | uint8_t | 目标组件ID |
| param_id | char[16] | 参数ID字符串，以NULL结尾（若小于16字符）或无NULL终止符（若恰好16字符） |
| param_index | int16_t | 参数索引，设为-1表示使用参数ID作为标识符 |

---

### PARAM_REQUEST_LIST (ID: 21)
**描述：** 请求此组件的所有参数（接收方将以PARAM_VALUE消息广播所有参数值）。
| 字段名 | 类型 | 描述 |
|---|---|---|
| target_system | uint8_t | 目标系统ID |
| target_component | uint8_t | 目标组件ID |

---

### PARAM_SET (ID: 22)
**描述：** 发送命令将指定参数设置为某个值（设置完成后应通过PARAM_VALUE消息广播当前值）。
| 字段名 | 类型 | 值（枚举） | 描述 |
|---|---|---|---|
| target_system | uint8_t | - | 目标系统ID |
| target_component | uint8_t | - | 目标组件ID |
| param_id | char[16] | - | 参数ID字符串 |
| param_value | float | - | 参数值 |
| param_type | uint8_t | MAV_PARAM_TYPE | 参数类型 |

---

### PARAM_VALUE (ID: 23)
**描述：** 参数的当前值（响应参数请求或参数变化时广播）。
| 字段名 | 类型 | 值（枚举） | 描述 |
|---|---|---|---|
| param_id | char[16] | - | 参数ID字符串 |
| param_value | float | - | 参数值 |
| param_type | uint8_t | MAV_PARAM_TYPE | 参数类型 |
| param_count | uint16_t | - | 总参数数量 |
| param_index | uint16_t | - | 此参数索引 |

---

### GPS_RAW_INT (ID: 24)
**描述：** GPS 原始数据（非融合定位）。
| 字段名 | 类型 | 单位 | 值（枚举） | 描述 |
|---|---|---|---|---|
| time_usec | uint64_t | us | - | 时间戳 |
| fix_type | uint8_t | - | GPS_FIX_TYPE | 定位类型（0=无，2=2D，3=3D） |
| lat | int32_t | degE7 | - | 纬度（WGS84，度数×1e7） |
| lon | int32_t | degE7 | - | 经度（WGS84，度数×1e7） |
| alt | int32_t | mm | - | 海拔（毫米，MSL） |
| eph | uint16_t | cm | - | 水平精度（厘米） |
| epv | uint16_t | cm | - | 垂直精度（厘米） |
| vel | int16_t | cm/s | - | 地速（厘米/秒） |
| cog | uint16_t | cdeg | - | 航向角（百分度） |
| satellites_visible | uint8_t | - | - | 可见卫星数 |

---

### SCALED_IMU (ID: 26)
**描述：** 通常9DOF传感器的缩放IMU读数（包含磁力计、陀螺仪、加速度计数据）。
| 字段名 | 类型 | 单位 | 描述 |
|---|---|---|---|
| time_boot_ms | uint32_t | ms | 启动时间戳 |
| xacc | int16_t | mG | X轴加速度 |
| yacc | int16_t | mG | Y轴加速度 |
| zacc | int16_t | mG | Z轴加速度 |
| xgyro | int16_t | mrad/s | X轴角速度 |
| ygyro | int16_t | mrad/s | Y轴角速度 |
| zgyro | int16_t | mrad/s | Z轴角速度 |
| xmag | int16_t | mT | X轴磁场强度 |
| ymag | int16_t | mT | Y轴磁场强度 |
| zmag | int16_t | mT | Z轴磁场强度 |

---

### SCALED_PRESSURE (ID: 29)
**描述：** 气压数据（已缩放）。
| 字段名 | 类型 | 单位 | 描述 |
|---|---|---|---|
| time_boot_ms | uint32_t | ms | 启动时间戳 |
| press_abs | float | hPa | 绝对气压 |
| press_diff | float | hPa | 差分压力（高度计） |
| temperature | int16_t | cdegC | 温度 |

---

### ATTITUDE (ID: 30)
**描述：** 姿态数据（欧拉角+角速度）。
| 字段名 | 类型 | 单位 | 描述 |
|---|---|---|---|
| time_boot_ms | uint32_t | ms | 启动时间戳 |
| roll | float | rad | 横滚角 |
| pitch | float | rad | 俯仰角 |
| yaw | float | rad | 偏航角 |
| rollspeed | float | rad/s | 横滚角速度 |
| pitchspeed | float | rad/s | 俯仰角速度 |
| yawspeed | float | rad/s | 偏航角速度 |

---

### GLOBAL_POSITION_INT (ID: 33)
**描述：** 融合后全局位置（经纬高+相对高度）。
| 字段名 | 类型 | 单位 | 描述 |
|---|---|---|---|
| time_boot_ms | uint32_t | ms | 启动时间戳 |
| lat | int32_t | degE7 | 纬度（WGS84） |
| lon | int32_t | degE7 | 经度（WGS84） |
| alt | int32_t | mm | 海拔（MSL） |
| relative_alt | int32_t | mm | 相对高度（起飞点） |
| vx | int16_t | cm/s | 北向速度 |
| vy | int16_t | cm/s | 东向速度 |
| vz | int16_t | cm/s | 下向速度 |
| hdg | uint16_t | cdeg | 航向角（百分度） |

---

### RC_CHANNELS_RAW (ID: 35)
**描述：** 遥控器原始通道值（0-65535）。
| 字段名 | 类型 | 单位 | 描述 |
|---|---|---|---|
| time_boot_ms | uint32_t | ms | 启动时间戳 |
| port | uint8_t | - | RC接收机端口号（0-3） |
| chan1_raw | uint16_t | μs | 通道1原始值 |
| chan2_raw | uint16_t | μs | 通道2原始值 |
| chan3_raw | uint16_t | μs | 通道3原始值 |
| chan4_raw | uint16_t | μs | 通道4原始值 |
| chan5_raw | uint16_t | μs | 通道5原始值 |
| chan6_raw | uint16_t | μs | 通道6原始值 |
| chan7_raw | uint16_t | μs | 通道7原始值 |
| chan8_raw | uint16_t | μs | 通道8原始值 |
| rssi | uint8_t | - | 接收信号强度指示（0-100，0=无效） |

---

### SERVO_OUTPUT_RAW (ID: 36)
**描述：** 伺服输出原始脉冲宽度值（0-65535）。
| 字段名 | 类型 | 单位 | 描述 |
|---|---|---|---|
| time_usec | uint32_t | us | 时间戳（微秒） |
| port | uint8_t | - | 端口号（0为MAIN，1为AUX） |
| servo1_raw | uint16_t | μs | 伺服1输出脉冲宽度 |
| servo2_raw | uint16_t | μs | 伺服2输出脉冲宽度 |
| servo3_raw | uint16_t | μs | 伺服3输出脉冲宽度 |
| servo4_raw | uint16_t | μs | 伺服4输出脉冲宽度 |
| servo5_raw | uint16_t | μs | 伺服5输出脉冲宽度 |
| servo6_raw | uint16_t | μs | 伺服6输出脉冲宽度 |
| servo7_raw | uint16_t | μs | 伺服7输出脉冲宽度 |
| servo8_raw | uint16_t | μs | 伺服8输出脉冲宽度 |

---

### MISSION_REQUEST_PARTIAL_LIST (ID: 37)
**描述：** 请求任务项的部分列表（用于任务上传/下载）。
| 字段名 | 类型 | 描述 |
|---|---|---|
| target_system | uint8_t | 目标系统ID |
| target_component | uint8_t | 目标组件ID |
| start_index | int16_t | 起始索引 |
| end_index | int16_t | 结束索引 |

---

### MISSION_WRITE_PARTIAL_LIST (ID: 38)
**描述：** 任务部分列表的写入请求（用于任务上传）。
| 字段名 | 类型 | 描述 |
|---|---|---|
| target_system | uint8_t | 目标系统ID |
| target_component | uint8_t | 目标组件ID |
| start_index | int16_t | 起始索引 |
| end_index | int16_t | 结束索引 |

---

### MISSION_ITEM (ID: 39)
**描述：** 任务项（坐标、高度、动作等）。
| 字段名 | 类型 | 值（枚举） | 描述 |
|---|---|---|---|
| target_system | uint8_t | - | 目标系统ID |
| target_component | uint8_t | - | 目标组件ID |
| seq | uint16_t | - | 任务序号 |
| frame | uint8_t | MAV_FRAME | 坐标系 |
| command | uint16_t | MAV_CMD | 命令ID |
| current | uint8_t | - | 0: 任务项，1: 当前任务 |
| autocontinue | uint8_t | - | 自动继续到下一任务 |
| param1 | float | - | 参数1 |
| param2 | float | - | 参数2 |
| param3 | float | - | 参数3 |
| param4 | float | - | 参数4 |
| x | float | - | X位置或参数 |
| y | float | - | Y位置或参数 |
| z | float | - | Z位置或参数 |

---

### MISSION_REQUEST (ID: 40)
**描述：** 请求特定任务项（用于任务上传/下载）。
| 字段名 | 类型 | 描述 |
|---|---|---|
| target_system | uint8_t | 目标系统ID |
| target_component | uint8_t | 目标组件ID |
| seq | uint16_t | 任务序号 |

---

### MISSION_SET_CURRENT (ID: 41)
**描述：** 设置当前任务（激活特定任务项）。
| 字段名 | 类型 | 描述 |
|---|---|---|
| target_system | uint8_t | 目标系统ID |
| target_component | uint8_t | 目标组件ID |
| seq | uint16_t | 任务序号 |

---

### MISSION_CURRENT (ID: 42)
**描述：** 当前执行的任务序号。
| 字段名 | 类型 | 描述 |
|---|---|---|
| seq | uint16_t | 当前任务序号 |

---

### MISSION_REQUEST_LIST (ID: 43)
**描述：** 请求任务列表信息（总任务数）。
| 字段名 | 类型 | 描述 |
|---|---|---|
| target_system | uint8_t | 目标系统ID |
| target_component | uint8_t | 目标组件ID |

---

### MISSION_COUNT (ID: 44)
**描述：** 任务总数（响应任务列表请求）。
| 字段名 | 类型 | 描述 |
|---|---|---|
| target_system | uint8_t | 目标系统ID |
| target_component | uint8_t | 目标组件ID |
| count | uint16_t | 任务总数 |

---

### MISSION_CLEAR_ALL (ID: 45)
**描述：** 清除所有任务（删除任务列表）。
| 字段名 | 类型 | 描述 |
|---|---|---|
| target_system | uint8_t | 目标系统ID |
| target_component | uint8_t | 目标组件ID |

---

### MISSION_ITEM_REACHED (ID: 46)
**描述：** 任务项已到达（报告任务执行进度）。
| 字段名 | 类型 | 描述 |
|---|---|---|
| seq | uint16_t | 已到达的任务序号 |

---

### MISSION_ACK (ID: 47)
**描述：** 任务操作确认（任务上传/下载结果）。
| 字段名 | 类型 | 值（枚举） | 描述 |
|---|---|---|---|
| target_system | uint8_t | - | 目标系统ID |
| target_component | uint8_t | - | 目标组件ID |
| type | uint8_t | MAV_MISSION_RESULT | 任务操作结果 |

---

### SET_GPS_GLOBAL_ORIGIN (ID: 48)
**描述：** 设置GPS全球原点（基准坐标系）。
| 字段名 | 类型 | 单位 | 描述 |
|---|---|---|---|
| target_system | uint8_t | - | 目标系统ID |
| latitude | int32_t | degE7 | 纬度 |
| longitude | int32_t | degE7 | 经度 |
| altitude | int32_t | mm | 海拔（毫米） |
| time_usec | uint64_t | us | 时间戳（微秒） |

---

### GPS_GLOBAL_ORIGIN (ID: 49)
**描述：** GPS全球原点（基准坐标系）的位置信息。
| 字段名 | 类型 | 单位 | 描述 |
|---|---|---|---|
| latitude | int32_t | degE7 | 纬度 |
| longitude | int32_t | degE7 | 经度 |
| altitude | int32_t | mm | 海拔（毫米） |
| time_usec | uint64_t | us | 时间戳（微秒） |

---

### PARAM_MAP_RC (ID: 50)
**描述：** 将参数映射到RC通道（RC控制参数调节）。
| 字段名 | 类型 | 值（枚举） | 描述 |
|---|---|---|---|
| target_system | uint8_t | - | 目标系统ID |
| target_component | uint8_t | - | 目标组件ID |
| param_id | char[16] | - | 参数ID字符串 |
| param_index | int16_t | - | 参数索引，设为-1使用param_id |
| parameter_rc_channel_index | uint8_t | - | RC通道索引（0-17，255为禁用） |
| param_value0 | float | - | RC值=0时的参数值 |
| scale | float | - | RC值范围映射比例 |
| param_value_min | float | - | 参数最小值 |
| param_value_max | float | - | 参数最大值 |

---

### MISSION_REQUEST_INT (ID: 51)
**描述：** 请求任务项（整数坐标版本，用于大范围任务）。
| 字段名 | 类型 | 描述 |
|---|---|---|
| target_system | uint8_t | 目标系统ID |
| target_component | uint8_t | 目标组件ID |
| seq | uint16_t | 任务序号 |

---

### COMMAND_LONG (ID: 76)
**描述：** 长命令（7个参数，如解锁、起飞等）。
| 字段名 | 类型 | 值（枚举） | 描述 |
|---|---|---|---|
| target_system | uint8_t | - | 目标系统ID |
| target_component | uint8_t | - | 目标组件ID |
| command | uint16_t | MAV_CMD | 命令ID |
| confirmation | uint8_t | - | 确认次数（0为首次发送） |
| param1 | float | - | 参数1 |
| param2 | float | - | 参数2 |
| param3 | float | - | 参数3 |
| param4 | float | - | 参数4 |
| param5 | float | - | 参数5 |
| param6 | float | - | 参数6 |
| param7 | float | - | 参数7 |

---

### COMMAND_ACK (ID: 77)
**描述：** 命令确认（命令执行结果）。
| 字段名 | 类型 | 值（枚举） | 描述 |
|---|---|---|---|
| command | uint16_t | MAV_CMD | 命令ID |
| result | uint8_t | MAV_RESULT | 命令执行结果 |
| progress | uint8_t | - | 进度百分比（0-100，255表示不适用） |
| result_param2 | int32_t | - | 附加结果参数 |
| target_system | uint8_t | - | 目标系统ID |
| target_component | uint8_t | - | 目标组件ID |

---

### COMMAND_CANCEL (ID: 80)
**描述：** 取消命令（取消长时间运行的命令）。
| 字段名 | 类型 | 值（枚举） | 描述 |
|---|---|---|---|
| command | uint16_t | MAV_CMD | 要取消的命令ID |
| target_system | uint8_t | - | 目标系统ID |
| target_component | uint8_t | - | 目标组件ID |

---

### RC_CHANNELS_OVERRIDE (ID: 70)
**描述：** 遥控通道覆盖（地面站发送虚拟遥控值）。
| 字段名 | 类型 | 单位 | 描述 |
|---|---|---|---|
| target_system | uint8_t | - | 目标系统ID |
| target_component | uint8_t | - | 目标组件ID |
| chan1_raw | uint16_t | μs | 通道1覆盖值 |
| chan2_raw | uint16_t | μs | 通道2覆盖值 |
| chan3_raw | uint16_t | μs | 通道3覆盖值 |
| chan4_raw | uint16_t | μs | 通道4覆盖值 |
| chan5_raw | uint16_t | μs | 通道5覆盖值 |
| chan6_raw | uint16_t | μs | 通道6覆盖值 |
| chan7_raw | uint16_t | μs | 通道7覆盖值 |
| chan8_raw | uint16_t | μs | 通道8覆盖值 |

---

### RC_CHANNELS (ID: 65)
**描述：** 遥控器通道原始值（更新版本，支持更多通道）。
| 字段名 | 类型 | 单位 | 描述 |
|---|---|---|---|
| time_boot_ms | uint32_t | ms | 启动时间戳 |
| chancount | uint8_t | - | 通道数量 |
| chan1_raw | uint16_t | μs | 通道1原始值 |
| chan2_raw | uint16_t | μs | 通道2原始值 |
| chan3_raw | uint16_t | μs | 通道3原始值 |
| chan4_raw | uint16_t | μs | 通道4原始值 |
| chan5_raw | uint16_t | μs | 通道5原始值 |
| chan6_raw | uint16_t | μs | 通道6原始值 |
| chan7_raw | uint16_t | μs | 通道7原始值 |
| chan8_raw | uint16_t | μs | 通道8原始值 |
| chan9_raw | uint16_t | μs | 通道9原始值 |
| chan10_raw | uint16_t | μs | 通道10原始值 |
| chan11_raw | uint16_t | μs | 通道11原始值 |
| chan12_raw | uint16_t | μs | 通道12原始值 |
| chan13_raw | uint16_t | μs | 通道13原始值 |
| chan14_raw | uint16_t | μs | 通道14原始值 |
| chan15_raw | uint16_t | μs | 通道15原始值 |
| chan16_raw | uint16_t | μs | 通道16原始值 |
| chan17_raw | uint16_t | μs | 通道17原始值（可选） |
| chan18_raw | uint16_t | μs | 通道18原始值（可选） |
| rssi | uint8_t | - | 接收信号强度指示 |

---

### BATTERY_STATUS (ID: 147)
**描述：** 电池详细状态（多电芯电池）。
| 字段名 | 类型 | 单位 | 描述 |
|---|---|---|---|
| id | uint8_t | - | 电池ID |
| battery_function | uint8_t | MAV_BATTERY_FUNCTION | 电池功能 |
| type | uint8_t | MAV_BATTERY_TYPE | 电池类型 |
| temperature | int16_t | cdegC | 电池温度 |
| voltages | uint16_t[10] | mV | 电芯电压数组 |
| current_battery | int16_t | cA | 电池电流 |
| current_consumed | int32_t | mAh | 已消耗电流 |
| energy_consumed | int32_t | hJ | 已消耗能量 |
| battery_remaining | int8_t | % | 剩余电量百分比 |
| time_remaining | uint32_t | s | 剩余使用时间 |
| charge_state | uint8_t | MAV_BATTERY_CHARGE_STATE | 充电状态 |

---

### AUTOPILOT_VERSION (ID: 181)
**描述：** 自动驾驶仪版本信息（固件版本、能力等）。
| 字段名 | 类型 | 描述 |
|---|---|---|---|
| capabilities | uint64_t | 自动驾驶仪能力位图 |
| flight_sw_version | uint32_t | 飞行软件版本 |
| middleware_sw_version | uint32_t | 中间件软件版本 |
| os_sw_version | uint32_t | 操作系统版本 |
| board_version | uint32_t | 板卡硬件版本 |
| flight_custom_version | uint8_t[8] | 飞行软件自定义版本 |
| middleware_custom_version | uint8_t[8] | 中间件自定义版本 |
| os_custom_version | uint8_t[8] | 操作系统自定义版本 |
| vendor_id | int16_t | 供应商ID |
| product_id | int16_t | 产品ID |
| uid | uint64_t | 硬件UID |
| uid2 | uint8_t[18] | 扩展硬件UID |

---

### HIGH_LATENCY2 (ID: 235)
**描述：** 高延迟链路精简遥测（替代旧版HIGH_LATENCY，适用于卫星或LoRa等低带宽链路）。
| 字段名 | 类型 | 单位 | 值（枚举） | 描述 |
|---|---|---|---|---|
| timestamp | uint32_t | s | 时间戳（自纪元以来的秒数） |
| type | uint8_t | MAV_TYPE | 载具类型 |
| autopilot | uint8_t | MAV_AUTOPILOT | 自动驾驶仪类型 |
| heading | uint16_t | cdeg | 航向角 |
| latitude | int32_t | degE7 | 纬度 |
| longitude | int32_t | degE7 | 经度 |
| altitude | int16_t | m | 海拔高度 |
| target_altitude | int16_t | m | 目标高度 |
| latitude_int | int32_t | degE7 | 目标纬度 |
| longitude_int | int32_t | degE7 | 目标经度 |
| target_heading | uint8_t | deg | 目标航向 |
| target_distance | uint16_t | dam | 到目标距离 |
| throttle | uint8_t | % | 油门百分比 |
| airspeed | uint8_t | m/s | 空速 |
| airspeed_sp | uint8_t | m/s | 空速设定点 |
| groundspeed | uint8_t | m/s | 地速 |
| windspeed | uint8_t | m/s | 风速 |
| wind_direction | uint8_t | deg | 风向 |
| eph | uint8_t | dm | GPS水平精度 |
| epv | uint8_t | dm | GPS垂直精度 |
| temperature_air | int8_t | degC | 空气温度 |
| climb_rate | int8_t | dm/s | 爬升率 |
| battery | int8_t | % | 电池百分比 |
| custom0 | int8_t | - | 自定义字段0 |
| custom1 | int8_t | - | 自定义字段1 |
| custom2 | uint8_t | - | 自定义字段2 |

---

### ACTUATOR_CONTROL_TARGET (ID: 140)
**描述：** 执行器控制目标（姿态+油门）。
| 字段名 | 类型 | 单位 | 描述 |
|---|---|---|---|
| time_usec | uint64_t | us | 时间戳 |
| group_mlx | uint8_t | - | 执行器组 ID（多实例区分） |
| controls | float[8] | - | 控制量（-1~+1：横滚、俯仰、偏航、油门、襟翼、扰流板、刹车、起落架） |

---

### ALTITUDE (ID: 141)
**描述：** 多维度高度数据（单调/绝对/相对/地形）。
| 字段名 | 类型 | 单位 | 描述 |
|---|---|---|---|
| time_usec | uint64_t | us | 时间戳 |
| altitude_monotonic | float | m | 单调高度（启动后不重置） |
| altitude_amsl | float | m | 绝对海拔（MSL） |
| altitude_local | float | m | 本地坐标系高度 |
| altitude_relative | float | m | 相对起飞点高度 |
| altitude_terrain | float | m | 相对地形高度（<-1000=无效） |
| bottom_clearance | float | m | 离地间隙（负=无效） |

---

### （其余消息速览，完整含全部 200+ 消息）
- **GPS_STATUS (ID: 25)**：GPS卫星状态
- **SERVO_OUTPUT_RAW (ID: 36)**：伺服输出原始脉冲宽度
- **MISSION_ITEM_INT (ID: 73)**：任务项（整数坐标版本）
- **VFR_HUD (ID: 74)**：虚拟HUD数据（空速、高度、爬升率等）
- **COMMAND_INT (ID: 75)**：整数命令（整数坐标版本）
- **LOCAL_POSITION_NED (ID: 32)**：本地坐标系位置（NED坐标系）
- **POSITION_TARGET_LOCAL_NED (ID: 64)**：本地坐标系位置目标
- **POSITION_TARGET_GLOBAL_INT (ID: 87)**：全球坐标系位置目标
- **ATTITUDE_TARGET (ID: 83)**：姿态目标
- **RC_CHANNELS_SCALED (ID: 34)**：缩放后的遥控通道值
- **VIBRATION (ID: 241)**：振动数据
- **HOME_POSITION (ID: 242)**：家位置（起飞点坐标）
- **STATUSTEXT (ID: 253)**：状态文本消息
- **DEBUG (ID: 254)**：调试数据
- **DEBUG_VECT (ID: 255)**：调试向量数据
- **SYSTEM_TIME (ID: 2)**：系统时间（Unix时间戳+启动时间）
- **TIMESYNC (ID: 111)**：时间同步消息
- **HIL_STATE (ID: 90)**：硬件在环仿真状态
- **HIL_CONTROLS (ID: 91)**：硬件在环仿真控制
- **OPTICAL_FLOW (ID: 100)**：光流数据
- **GLOBAL_VISION_POSITION_ESTIMATE (ID: 101)**：全局视觉位置估计
- **VISION_POSITION_ESTIMATE (ID: 102)**：视觉位置估计
- **VISION_SPEED_ESTIMATE (ID: 103)**：视觉速度估计
- **SCALED_IMU2 (ID: 107)**：第二套缩放IMU数据
- **SCALED_IMU3 (ID: 108)**：第三套缩放IMU数据
- **SCALED_PRESSURE2 (ID: 109)**：第二套缩放气压数据
- **SCALED_PRESSURE3 (ID: 129)**：第三套缩放气压数据
- **NAV_CONTROLLER_OUTPUT (ID: 62)**：导航控制器输出
- **GLOBAL_POSITION_INT_COV (ID: 93)**：带协方差的全局位置
- **LOCAL_POSITION_NED_COV (ID: 94)**：带协方差的本地位置
- **ATTITUDE_QUATERNION_COV (ID: 102)**：带协方差的姿态四元数
- **POSITION_TARGET_GLOBAL_INT (ID: 87)**：全球坐标系位置目标
- **MISSION_CHECKSUM (ID: 223)**：任务校验和
- **ESC_TELEMETRY_1_TO_4 (ID: 217)**：电调遥测数据（1-4号电调）
- **ESC_TELEMETRY_5_TO_8 (ID: 218)**：电调遥测数据（5-8号电调）
- **ESC_TELEMETRY_9_TO_12 (ID: 219)**：电调遥测数据（9-12号电调）
- **OBSTACLE_DISTANCE (ID: 330)**：障碍物距离
- **ODOMETRY (ID: 331)**：里程计数据
- **TRAJECTORY_REPRESENTATION_WAYPOINTS (ID: 332)**：航路点轨迹表示
- **TRAJECTORY_REPRESENTATION_BEZIER (ID: 333)**：贝塞尔曲线轨迹表示
- **CELLULAR_STATUS (ID: 334)**：蜂窝网络状态
- **ISBD_LINK_STATUS (ID: 335)**：铱星通信链路状态
- **CELLULAR_CONFIG (ID: 336)**：蜂窝网络配置
- **RAW_RPM (ID: 339)**：原始RPM数据
- **UTM_GLOBAL_POSITION (ID: 340)**：UTM全球位置
- **DATA_TRANSMISSION_HANDSHAKE (ID: 130)**：数据传输握手
- **ENCAPSULATED_DATA (ID: 131)**：封装数据
- **DISTANCE_SENSOR (ID: 132)**：距离传感器
- **TERRAIN_REQUEST (ID: 133)**：地形请求
- **TERRAIN_DATA (ID: 134)**：地形数据
- **TERRAIN_CHECK (ID: 135)**：地形检查
- **TERRAIN_REPORT (ID: 136)**：地形报告
- **SCALED_BATTERY (ID: 137)**：缩放电池数据
- **BATTERY_STATUS (ID: 147)**：电池状态（详细版本）
- **BATTERY_INFO (ID: 152)**：电池信息
- **ENERGY_STATUS (ID: 150)**：能量状态
- **CAN_FRAME (ID: 386)**：CAN总线帧
- **CANFD_FRAME (ID: 387)**：CANFD总线帧
- **CAN_FILTER_MODIFY (ID: 388)**：CAN过滤器修改
- **PLAY_TUNE (ID: 258)**：播放音调
- **PLAY_TUNE_V2 (ID: 400)**：播放音调（版本2）
- **GROUND_truth (ID: 143)**：地面真值（仿真数据）
- **PING (ID: 4)**：ping消息（已废弃，使用TIMESYNC代替）

---

## 枚举（Enums）

### MAV_TYPE（载具类型）
| 值 | 名称 | 描述 |
|---|---|---|
| 0 | MAV_TYPE_GENERIC | 通用未知类型 |
| 1 | MAV_TYPE_FIXED_WING | 固定翼 |
| 2 | MAV_TYPE_QUADROTOR | 四轴多旋翼 |
| 3 | MAV_TYPE_COAXIAL | 共轴双桨 |
| 4 | MAV_TYPE_HELICOPTER | 直升机 |
| 5 | MAV_TYPE_ANTENNA_TRACKER | 天线跟踪器 |
| 6 | MAV_TYPE_GCS | 地面站 |
| 7 | MAV_TYPE_AIRSHIP | 飞艇 |
| 8 | MAV_TYPE_FREE_BALLOON | 自由气球 |
| 9 | MAV_TYPE_ROCKET | 火箭 |
| 10 | MAV_TYPE_GROUND_ROVER | 地面无人车 |
| 11 | MAV_TYPE_SURFACE_BOAT | 水面无人船 |
| 12 | MAV_TYPE_SUBMARINE | 潜水器 |
| 13 | MAV_TYPE_HEXAROTOR | 六轴飞行器 |
| 14 | MAV_TYPE_OCTOROTOR | 八轴飞行器 |
| 15 | MAV_TYPE_TRICOPTER | 三轴飞行器 |
| 16 | MAV_TYPE_FLAPPING_WING | 扑翼机 |
| 17 | MAV_TYPE_KITE | 风筝 |
| 18 | MAV_TYPE_ONBOARD_CONTROLLER | 机载控制器 |
| 19 | MAV_TYPE_VTOL_DUOROTOR | VTOL双旋翼 |
| 20 | MAV_TYPE_VTOL_QUADROTOR | VTOL四旋翼 |
| 21 | MAV_TYPE_VTOL_TILTROTOR | VTOL倾转旋翼 |
| 22 | MAV_TYPE_VTOL_FIXEDROTOR | VTOL固定旋翼 |
| 23 | MAV_TYPE_VTOL_TAILSITTER | VTOL尾坐式 |
| 24 | MAV_TYPE_VTOL_TILTWING | VTOL倾转翼 |
| 25 | MAV_TYPE_VTOL_RESERVED5 | VTOL保留类型5 |
| 26 | MAV_TYPE_GIMBAL | 云台 |
| 27 | MAV_TYPE_ADSB | ADSB系统 |
| 28 | MAV_TYPE_PARAFOIL | 降落伞 |
| 29 | MAV_TYPE_DODECAROTOR | 十二轴飞行器 |
| 30 | MAV_TYPE_CAMERA | 相机 |
| 31 | MAV_TYPE_CHARGING_STATION | 充电站 |
| 32 | MAV_TYPE_FLARM | FLARM防撞系统 |
| 33 | MAV_TYPE_SERVO | 伺服机构 |
| 34 | MAV_TYPE_ODID | 广播式开放遥测ID |
| 35 | MAV_TYPE_DECAROTOR | 十轴飞行器 |
| 36 | MAV_TYPE_BATTERY | 电池 |
| 37 | MAV_TYPE_PARACHUTE | 降落伞 |
| 38 | MAV_TYPE_LOG | 日志 |
| 39 | MAV_TYPE_OSD | OSD叠加显示器 |
| 40 | MAV_TYPE_IMU | 惯性测量单元 |
| 41 | MAV_TYPE_GPS | GPS |
| 42 | MAV_TYPE_WINCH | 绞盘 |

### MAV_AUTOPILOT（自动驾驶仪类型）
| 值 | 名称 | 描述 |
|---|---|---|
| 0 | MAV_AUTOPILOT_GENERIC | 通用自动驾驶仪 |
| 1 | MAV_AUTOPILOT_RESERVED | 保留 |
| 2 | MAV_AUTOPILOT_SLUGS | SLUGS自动驾驶仪 |
| 3 | MAV_AUTOPILOT_ARDUPILOTMEGA | ArduPilotMega/AC3 |
| 4 | MAV_AUTOPILOT_OPENPILOT | OpenPilot |
| 5 | MAV_AUTOPILOT_GENERIC_WAYPOINTS_ONLY | 仅支持航点的通用自动驾驶仪 |
| 6 | MAV_AUTOPILOT_GENERIC_WAYPOINTS_AND_SIMPLE_NAVIGATION_ONLY | 仅支持航点和简单导航的通用自动驾驶仪 |
| 7 | MAV_AUTOPILOT_PIXHAWK | Pixhawk |
| 8 | MAV_AUTOPILOT_AUTOQUAD | AutoQuad |
| 9 | MAV_AUTOPILOT_ARMAZILA | ArmaZila |
| 10 | MAV_AUTOPILOT_PX4 | PX4 |
| 11 | MAV_AUTOPILOT_SENSEFLY | SenseFly |
| 12 | MAV_AUTOPILOT_TEST | 测试 |
| 13 | MAV_AUTOPILOT_UDB | UDB |
| 14 | MAV_AUTOPILOT_FP | FlexiPilot |
| 15 | MAV_AUTOPILOT_PX4_ADAPTIVE | PX4自适应控制 |
| 16 | MAV_AUTOPILOT_AVEX | AvEx |
| 17 | MAV_AUTOPILOT_ARGENTUM | Argentum |
| 18 | MAV_AUTOPILOT_URUS | Urus |
| 19 | MAV_AUTOPILOT_KK | KK |
| 20 | MAV_AUTOPILOT_ENUM_END | 枚举结束标志 |

### MAV_STATE（系统状态）
| 值 | 名称 | 描述 |
|---|---|---|
| 0 | MAV_STATE_UNINIT | 未初始化 |
| 1 | MAV_STATE_BOOT | 启动中 |
| 2 | MAV_STATE_CALIBRATING | 校准中 |
| 3 | MAV_STATE_STANDBY | 待机 |
| 4 | MAV_STATE_ACTIVE | 正常飞行 |
| 5 | MAV_STATE_CRITICAL | 临界故障 |
| 6 | MAV_STATE_EMERGENCY | 紧急故障 |
| 7 | MAV_STATE_POWEROFF | 关机 |
| 8 | MAV_STATE_FLIGHT_TERMINATION | 飞行终止 |

### MAV_MODE_FLAG（基础模式位）
| 位 | 名称 | 描述 |
|---|---|---|
| 0 | MAV_MODE_FLAG_SAFETY_ARMED | 安全解锁（可飞行） |
| 1 | MAV_MODE_FLAG_MANUAL_INPUT_ENABLED | 手动控制使能 |
| 2 | MAV_MODE_FLAG_HIL_ENABLED | 硬件在环仿真使能 |
| 3 | MAV_MODE_FLAG_STABILIZE_ENABLED | 增稳模式使能 |
| 4 | MAV_MODE_FLAG_GUIDED_ENABLED | 引导模式使能 |
| 5 | MAV_MODE_FLAG_AUTO_ENABLED | 自动模式使能 |
| 6 | MAV_MODE_FLAG_TEST_ENABLED | 测试模式使能 |
| 7 | MAV_MODE_FLAG_CUSTOM_MODE_ENABLED | 自定义模式使能 |

### MAV_MODE（预定义模式组合）
| 值 | 名称 | 描述 |
|---|---|---|
| 0 | MAV_MODE_PREFLIGHT | 预飞状态（未准备飞行） |
| 64 | MAV_MODE_MANUAL_DISARMED | 手动模式（未解锁） |
| 80 | MAV_MODE_STABILIZE_DISARMED | 增稳模式（未解锁） |
| 88 | MAV_MODE_GUIDED_DISARMED | 引导模式（未解锁） |
| 92 | MAV_MODE_AUTO_DISARMED | 自动模式（未解锁） |
| 192 | MAV_MODE_MANUAL_ARMED | 手动模式（已解锁） |
| 208 | MAV_MODE_STABILIZE_ARMED | 增稳模式（已解锁） |
| 216 | MAV_MODE_GUIDED_ARMED | 引导模式（已解锁） |
| 220 | MAV_MODE_AUTO_ARMED | 自动模式（已解锁） |
| 66 | MAV_MODE_TEST_DISARMED | 测试模式（未解锁） |
| 194 | MAV_MODE_TEST_ARMED | 测试模式（已解锁） |

### GPS_FIX_TYPE（GPS定位类型）
| 值 | 名称 | 描述 |
|---|---|---|
| 0 | GPS_FIX_TYPE_NO_GPS | 无GPS |
| 1 | GPS_FIX_TYPE_NO_FIX | 无定位 |
| 2 | GPS_FIX_TYPE_2D_FIX | 2D定位 |
| 3 | GPS_FIX_TYPE_3D_FIX | 3D定位 |
| 4 | GPS_FIX_TYPE_DGPS | 差分GPS |
| 5 | GPS_FIX_TYPE_RTK_FLOAT | RTK浮点解 |
| 6 | GPS_FIX_TYPE_RTK_FIXED | RTK固定解 |
| 7 | GPS_FIX_TYPE_STATIC | 静态定位 |
| 8 | GPS_FIX_TYPE_PPP | 精密单点定位 |

### MAV_MISSION_RESULT（任务操作结果）
| 值 | 名称 | 描述 |
|---|---|---|
| 0 | MAV_MISSION_ACCEPTED | 任务接受 |
| 1 | MAV_MISSION_ERROR | 任务错误 |
| 2 | MAV_MISSION_UNSUPPORTED_FRAME | 不支持的坐标系 |
| 3 | MAV_MISSION_UNSUPPORTED | 不支持的任务 |
| 4 | MAV_MISSION_NO_SPACE | 空间不足 |
| 5 | MAV_MISSION_INVALID | 无效任务 |
| 6 | MAV_MISSION_INVALID_PARAM1 | 参数1无效 |
| 7 | MAV_MISSION_INVALID_PARAM2 | 参数2无效 |
| 8 | MAV_MISSION_INVALID_PARAM3 | 参数3无效 |
| 9 | MAV_MISSION_INVALID_PARAM4 | 参数4无效 |
| 10 | MAV_MISSION_INVALID_PARAM5_X | 参数5(X)无效 |
| 11 | MAV_MISSION_INVALID_PARAM6_Y | 参数6(Y)无效 |
| 12 | MAV_MISSION_INVALID_PARAM7 | 参数7无效 |
| 13 | MAV_MISSION_INVALID_SEQUENCE | 序列无效 |
| 14 | MAV_MISSION_DENIED | 任务被拒绝 |
| 15 | MAV_MISSION_OPERATION_CANCELLED | 操作已取消 |

### MAV_FRAME（坐标系）
| 值 | 名称 | 描述 |
|---|---|---|
| 0 | MAV_FRAME_GLOBAL | 全球坐标系（WGS84） |
| 1 | MAV_FRAME_LOCAL_NED | 本地NED坐标系 |
| 2 | MAV_FRAME_MISSION | 任务坐标系 |
| 3 | MAV_FRAME_GLOBAL_RELATIVE_ALT | 全球相对高度坐标系 |
| 4 | MAV_FRAME_LOCAL_ENU | 本地ENU坐标系 |
| 5 | MAV_FRAME_GLOBAL_INT | 全球整数坐标系 |
| 6 | MAV_FRAME_GLOBAL_RELATIVE_ALT_INT | 全球相对高度整数坐标系 |
| 7 | MAV_FRAME_LOCAL_OFFSET_NED | 本地偏移NED坐标系 |
| 8 | MAV_FRAME_BODY_NED | 机体NED坐标系 |
| 9 | MAV_FRAME_BODY_OFFSET_NED | 机体偏移NED坐标系 |
| 10 | MAV_FRAME_GLOBAL_TERRAIN_ALT | 全球地形高度坐标系 |
| 11 | MAV_FRAME_GLOBAL_TERRAIN_ALT_INT | 全球地形高度整数坐标系 |
| 12 | MAV_FRAME_BODY_FRD | 机体FRD坐标系 |
| 13 | MAV_FRAME_RESERVED_13 | 保留 |
| 14 | MAV_FRAME_LOCAL_FRD | 本地FRD坐标系 |
| 15 | MAV_FRAME_LOCAL_FLU | 本地FLU坐标系 |

### MAV_CMD（命令ID枚举，部分示例）
| 值 | 名称 | 描述 |
|---|---|---|
| 1 | MAV_CMD_NAV_WAYPOINT | 导航：航路点 |
| 2 | MAV_CMD_NAV_LOITER_UNLIM | 导航：无限盘旋 |
| 16 | MAV_CMD_NAV_LAND | 导航：着陆 |
| 22 | MAV_CMD_NAV_TAKEOFF | 导航：起飞 |
| 400 | MAV_CMD_CONDITION_DELAY | 条件：延迟 |
| 420 | MAV_CMD_CONDITION_CHANGE_ALT | 条件：改变高度 |
| 500 | MAV_CMD_DO_SET_SERVO | 执行：设置伺服 |
| 510 | MAV_CMD_DO_REPEAT_SERVO | 执行：重复伺服 |
| 520 | MAV_CMD_DO_SET_ACTUATOR | 执行：设置执行器 |
| 501 | MAV_CMD_DO_SET_RELAY | 执行：设置继电器 |
| 511 | MAV_CMD_DO_REPEAT_RELAY | 执行：重复继电器 |
| 521 | MAV_CMD_DO_SET_ROI | 执行：设置兴趣区域 |
| 201 | MAV_CMD_DO_MOUNT_CONTROL | 执行：云台控制 |
| 241 | MAV_CMD_DO_FENCE_ENABLE | 执行：启用电子围栏 |
| 242 | MAV_CMD_DO_PARACHUTE | 执行：降落伞 |
| 245 | MAV_CMD_DO_MOTOR_TEST | 执行：电机测试 |
| 252 | MAV_CMD_DO_INVERTED_FLIGHT | 执行：倒飞 |
| 255 | MAV_CMD_DO_GRIPPER | 执行：机械爪 |
| 260 | MAV_CMD_DO_AUTOTUNE_ENABLE | 执行：启用自动调参 |
| 280 | MAV_CMD_DO_SET_CAM_TRIGG_DIST | 执行：设置相机触发距离 |
| 290 | MAV_CMD_DO_FLY_PATH_REVERSE | 执行：反转飞行路径 |
| 400 | MAV_CMD_CONDITION_GATE | 条件：门 |
| 500 | MAV_CMD_DO_JUMP | 执行：跳转 |
| 510 | MAV_CMD_DO_CHANGE_SPEED | 执行：改变速度 |
| 511 | MAV_CMD_DO_SET_HOME | 执行：设置家 |
| 512 | MAV_CMD_DO_SET_PARAMETER | 执行：设置参数 |
| 519 | MAV_CMD_DO_SET_RELAY | 执行：设置继电器 |
| 520 | MAV_CMD_DO_LAND_START | 执行：着陆开始 |
| 521 | MAV_CMD_DO_RALLY_LAND | 执行：集合点着陆 |
| 522 | MAV_CMD_DO_GO_AROUND | 执行：复飞 |
| 523 | MAV_CMD_DO_REPOSITION | 执行：重新定位 |
| 524 | MAV_CMD_DO_PAUSE_CONTINUE | 执行：暂停/继续 |
| 525 | MAV_CMD_DO_SET_REVERSE | 执行：设置反向 |
| 526 | MAV_CMD_DO_SET_ROI_LOCATION | 执行：设置ROI位置 |
| 527 | MAV_CMD_DO_SET_ROI_WPNEXT_OFFSET | 执行：设置ROI航点偏移 |
| 528 | MAV_CMD_DO_SET_ROI_NONE | 执行：取消ROI |
| 529 | MAV_CMD_DO_ORBIT | 执行：环绕 |
| 530 | MAV_CMD_NAV_ROI | 导航：ROI |
| 531 | MAV_CMD_NAV_FOLLOW | 导航：跟随 |
| 532 | MAV_CMD_NAV_CONTINUE_AND_CHANGE_ALT | 导航：继续并改变高度 |
| 533 | MAV_CMD_DO_SET_CAM_TRIGG_INTERVAL | 执行：设置相机触发间隔 |
| 534 | MAV_CMD_DO_MOUNT_CONTROL_QUAT | 执行：四元数云台控制 |
| 535 | MAV_CMD_DO_GUIDED_MASTER | 执行：引导主控 |
| 536 | MAV_CMD_DO_GUIDED_LIMITS | 执行：引导限制 |
| 537 | MAV_CMD_DO_ENGINE_CONTROL | 执行：发动机控制 |
| 538 | MAV_CMD_DO_SET_MISSION_CURRENT | 执行：设置当前任务 |
| 539 | MAV_CMD_DO_LAST | 执行：最后命令 |
| 540 | MAV_CMD_PREFLIGHT_CALIBRATION | 预飞：校准 |
| 550 | MAV_CMD_PREFLIGHT_SET_SENSOR_OFFSETS | 预飞：设置传感器偏移 |
| 551 | MAV_CMD_PREFLIGHT_UAVCAN | 预飞：UAVCAN |
| 552 | MAV_CMD_PREFLIGHT_STORAGE | 预飞：存储 |
| 553 | MAV_CMD_PREFLIGHT_REBOOT_SHUTDOWN | 预飞：重启/关机 |
| 554 | MAV_CMD_PREFLIGHT_AUTH_KEY | 预飞：认证密钥 |
| 555 | MAV_CMD_PREFLIGHT_SET_DEFAULT_PARAMETERS | 预飞：设置默认参数 |
| 556 | MAV_CMD_PREFLIGHT_UAVCAN_GET_NODE_INFO | 预飞：获取UAVCAN节点信息 |
| 560 | MAV_CMD_REQUEST_AUTOPILOT_CAPABILITIES | 请求：自驾仪能力 |
| 561 | MAV_CMD_REQUEST_CAMERA_INFORMATION | 请求：相机信息 |
| 562 | MAV_CMD_REQUEST_CAMERA_SETTINGS | 请求：相机设置 |
| 563 | MAV_CMD_REQUEST_STORAGE_INFORMATION | 请求：存储信息 |
| 564 | MAV_CMD_REQUEST_CAMERA_CAPTURE_STATUS | 请求：相机拍摄状态 |
| 565 | MAV_CMD_REQUEST_FLIGHT_INFORMATION | 请求：飞行信息 |
| 566 | MAV_CMD_REQUEST_AUTOPILOT_VERSION | 请求：自驾仪版本 |
| 567 | MAV_CMD_REQUEST_ORTHO_PHOTO_CAMERA_CONFIG | 请求：正射摄影相机配置 |
| 570 | MAV_CMD_IMAGE_START_CAPTURE | 图像：开始拍摄 |
| 571 | MAV_CMD_IMAGE_STOP_CAPTURE | 图像：停止拍摄 |
| 572 | MAV_CMD_DO_TRIGGER_CONTROL | 执行：触发控制 |
| 573 | MAV_CMD_CAMERA_TRACK_POINT | 相机：跟踪点 |
| 574 | MAV_CMD_CAMERA_TRACK_RECTANGLE | 相机：跟踪矩形 |
| 575 | MAV_CMD_CAMERA_STOP_TRACKING | 相机：停止跟踪 |
| 580 | MAV_CMD_VIDEO_START_CAPTURE | 视频：开始录制 |
| 581 | MAV_CMD_VIDEO_STOP_CAPTURE | 视频：停止录制 |
| 582 | MAV_CMD_VIDEO_START_STREAMING | 视频：开始流媒体 |
| 583 | MAV_CMD_VIDEO_STOP_STREAMING | 视频：停止流媒体 |
| 585 | MAV_CMD_REQUEST_VIDEO_STREAM_INFORMATION | 请求：视频流信息 |
| 586 | MAV_CMD_REQUEST_VIDEO_STREAM_STATUS | 请求：视频流状态 |
| 587 | MAV_CMD_VIDEO_SET_STREAM_STATUS_RATE | 视频：设置流状态速率 |
| 590 | MAV_CMD_LOGGING_START | 日志：开始记录 |
| 591 | MAV_CMD_LOGGING_STOP | 日志：停止记录 |
| 592 | MAV_CMD_AIRFRAME_CONFIGURATION | 空气动力构型 |
| 595 | MAV_CMD_CONTROL_HIGH_LATENCY | 控制：高延迟 |
| 600 | MAV_CMD_DO_SET_SERVO | 执行：设置伺服 |
| 601 | MAV_CMD_DO_SET_ACTUATOR | 执行：设置执行器 |
| 602 | MAV_CMD_DO_PROCEED_TO_ITEM | 执行：前往项目 |
| 603 | MAV_CMD_DO_FENCE_ENABLE | 执行：启用电子围栏 |
| 604 | MAV_CMD_DO_MOTOR_TEST_RATIO | 执行：电机测试比率 |
| 605 | MAV_CMD_DO_GRIPPER_WHEELS | 执行：机械爪轮 |
| 606 | MAV_CMD_DO_AUTOTUNE_AXIS_LEVEL | 执行：自动调参轴级 |
| 610 | MAV_CMD_DO_SET_REMOTEID_DATA | 执行：设置远程ID数据 |
| 620 | MAV_CMD_DO_RETURN_HOME | 执行：回家 |
| 630 | MAV_CMD_DO_GO_AROUND | 执行：复飞 |
| 640 | MAV_CMD_DO_REPOSITION | 执行：重新定位 |
| 650 | MAV_CMD_DO_SET_CAM_TRIGG_DIST | 执行：设置相机触发距离 |
| 660 | MAV_CMD_DO_SET_CAM_TRIGG_INTERVAL | 执行：设置相机触发间隔 |
| 670 | MAV_CMD_DO_SET_CAM_TRIGG_GENERAL | 执行：设置相机触发通用 |
| 680 | MAV_CMD_DO_ATTACH_DETACH | 执行：附着/分离 |
| 690 | MAV_CMD_DO_SPRAYER | 执行：喷洒器 |
| 700 | MAV_CMD_DO_SEND_SERIAL_CONTROL | 执行：发送串口控制 |
| 710 | MAV_CMD_DO_SET_SERVO_OUT_MASK | 执行：设置伺服输出掩码 |
| 720 | MAV_CMD_DO_SET_ESC_PWM | 执行：设置电调PWM |
| 730 | MAV_CMD_DO_SET_BOARD_MODE | 执行：设置板卡模式 |
| 740 | MAV_CMD_DO_SET_STANDARD_VEHICLE_MESSAGE | 执行：设置标准载具消息 |
| 750 | MAV_CMD_DO_SET_ROI_SYSID | 执行：设置ROI系统ID |
| 760 | MAV_CMD_DO_SET_HOME_OFFSET | 执行：设置家偏移 |
| 770 | MAV_CMD_DO_SET_MOTOR_TEST_ORDER | 执行：设置电机测试顺序 |
| 780 | MAV_CMD_DO_SET_FACTORY_TEST_MODE | 执行：设置工厂测试模式 |
| 790 | MAV_CMD_DO_SET_ESC_RTC | 执行：设置电调RTC |
| 800 | MAV_CMD_REQUEST_MESSAGE | 请求：消息 |
| 801 | MAV_CMD_SET_CAMERA_MODE | 设置：相机模式 |
| 802 | MAV_CMD_SET_CAMERA_ZOOM | 设置：相机变焦 |
| 803 | MAV_CMD_SET_CAMERA_FOCUS | 设置：相机对焦 |
| 804 | MAV_CMD_DO_GIMBAL_MANAGER_PITCHYAW | 执行：云台管理器俯仰偏航 |
| 805 | MAV_CMD_DO_GIMBAL_MANAGER_CONFIGURE | 执行：云台管理器配置 |
| 806 | MAV_CMD_DO_SET_VIDEO_STREAM_INDEX | 执行：设置视频流索引 |
| 807 | MAV_CMD_DO_SET_SUPPORTED_CAMERA_FUNCTIONS | 执行：设置支持的相机功能 |
| 808 | MAV_CMD_DO_SET_CAMERA_ZOOM_BY_STEP | 执行：设置相机变焦步进 |
| 809 | MAV_CMD_DO_SET_CAMERA_FOCUS_BY_STEP | 执行：设置相机对焦步进 |
| 810 | MAV_CMD_DO_ORBIT | 执行：环绕 |
| 900 | MAV_CMD_PARAM_TRANSACTION | 参数：事务 |
| 901 | MAV_CMD_PARAM_EXT_REQUEST_READ | 参数：扩展请求读取 |
| 902 | MAV_CMD_PARAM_EXT_REQUEST_LIST | 参数：扩展请求列表 |
| 903 | MAV_CMD_PARAM_EXT_SET | 参数：扩展设置 |
| 904 | MAV_CMD_PARAM_EXT_ACK | 参数：扩展确认 |
| 905 | MAV_CMD_PARAM_EXT_VALUE | 参数：扩展值 |
| 1000 | MAV_CMD_REQUEST_PROTOCOL_VERSION | 请求：协议版本 |

### MAV_RESULT（命令执行结果）
| 值 | 名称 | 描述 |
|---|---|---|
| 0 | MAV_RESULT_ACCEPTED | 接受 |
| 1 | MAV_RESULT_TEMPORARILY_REJECTED | 临时拒绝 |
| 2 | MAV_RESULT_DENIED | 拒绝 |
| 3 | MAV_RESULT_UNSUPPORTED | 不支持 |
| 4 | MAV_RESULT_FAILED | 失败 |
| 5 | MAV_RESULT_IN_PROGRESS | 进行中 |
| 6 | MAV_RESULT_CANCELLED | 已取消 |

### MAV_BATTERY_FUNCTION（电池功能）
| 值 | 名称 | 描述 |
|---|---|---|
| 0 | MAV_BATTERY_FUNCTION_UNKNOWN | 未知 |
| 1 | MAV_BATTERY_FUNCTION_ALL | 所有功能 |
| 2 | MAV_BATTERY_FUNCTION_PROPULSION | 推进 |
| 3 | MAV_BATTERY_FUNCTION_AVIONICS | 航电 |
| 4 | MAV_BATTERY_TYPE_HOT_SWAP | 热插拔（电池类型） |

### MAV_BATTERY_TYPE（电池类型）
| 值 | 名称 | 描述 |
|---|---|---|
| 0 | MAV_BATTERY_TYPE_UNKNOWN | 未知 |
| 1 | MAV_BATTERY_TYPE_LIPO | 锂聚合物 |
| 2 | MAV_BATTERY_TYPE_LIFE | 锂铁 |
| 3 | MAV_BATTERY_TYPE_LION | 锂离子 |
| 4 | MAV_BATTERY_TYPE_NIMH | 镍氢 |

### MAV_BATTERY_CHARGE_STATE（充电状态）
| 值 | 名称 | 描述 |
|---|---|---|
| 0 | MAV_BATTERY_CHARGE_STATE_UNDEFINED | 未定义 |
| 1 | MAV_BATTERY_CHARGE_STATE_OK | 正常 |
| 2 | MAV_BATTERY_CHARGE_STATE_LOW | 低电量 |
| 3 | MAV_BATTERY_CHARGE_STATE_CRITICAL | 危险 |
| 4 | MAV_BATTERY_CHARGE_STATE_EMERGENCY | 紧急 |
| 5 | MAV_BATTERY_CHARGE_STATE_FAILED | 故障 |
| 6 | MAV_BATTERY_CHARGE_STATE_UNHEALTHY | 不健康 |
| 7 | MAV_BATTERY_CHARGE_STATE_CHARGING | 充电中 |

### MAV_PARAM_TYPE（参数类型）
| 值 | 名称 | 描述 |
|---|---|---|
| 1 | MAV_PARAM_TYPE_UINT8 | 无符号8位整数 |
| 2 | MAV_PARAM_TYPE_INT8 | 有符号8位整数 |
| 3 | MAV_PARAM_TYPE_UINT16 | 无符号16位整数 |
| 4 | MAV_PARAM_TYPE_INT16 | 有符号16位整数 |
| 5 | MAV_PARAM_TYPE_UINT32 | 无符号32位整数 |
| 6 | MAV_PARAM_TYPE_INT32 | 有符号32位整数 |
| 7 | MAV_PARAM_TYPE_UINT64 | 无符号64位整数 |
| 8 | MAV_PARAM_TYPE_INT64 | 有符号64位整数 |
| 9 | MAV_PARAM_TYPE_REAL32 | 32位浮点数 |
| 10 | MAV_PARAM_TYPE_REAL64 | 64位浮点数 |

### HL_FAILURE_FLAG（高延迟故障标志）
| 值 | 名称 | 描述 |
|---|---|---|
| 1 | HL_FAILURE_FLAG_GPS | GPS故障 |
| 2 | HL_FAILURE_FLAG_DIFFERENTIAL_PRESSURE | 差分压力传感器故障 |
| 4 | HL_FAILURE_FLAG_ABSOLUTE_PRESSURE | 绝对压力传感器故障 |
| 8 | HL_FAILURE_FLAG_3D_ACCEL | 加速度计传感器故障 |
| 16 | HL_FAILURE_FLAG_3D_GYRO | 陀螺仪传感器故障 |
| 32 | HL_FAILURE_FLAG_3D_MAG | 磁力计传感器故障 |
| 64 | HL_FAILURE_FLAG_TERRAIN | 地形子系统故障 |
| 128 | HL_FAILURE_FLAG_BATTERY | 电池故障/严重低电量 |
| 256 | HL_FAILURE_FLAG_RC_RECEIVER | RC接收机故障/无RC连接 |
| 512 | HL_FAILURE_FLAG_OFFBOARD_LINK | 机外链路故障 |
| 1024 | HL_FAILURE_FLAG_ENGINE | 发动机故障 |
| 2048 | HL_FAILURE_FLAG_GEOFENCE | 地理围栏违规 |
| 4096 | HL_FAILURE_FLAG_ESTIMATOR | 估计器故障，例如测量拒绝或大偏差 |
| 8192 | HL_FAILURE_FLAG_MISSION | 任务故障 |

### MAV_GOTO（覆盖GOTO操作）
| 值 | 名称 | 描述 |
|---|---|---|
| 0 | MAV_GOTO_DO_HOLD | 在当前位置悬停 |
| 1 | MAV_GOTO_DO_CONTINUE | 继续执行任务中的下一项 |
| 2 | MAV_GOTO_HOLD_AT_CURRENT_POSITION | 在系统当前位置悬停 |
| 3 | MAV_GOTO_HOLD_AT_SPECIFIED_POSITION | 在DO_HOLD动作参数中指定的位置悬停 |

### MAV_MODE（预定义模式组合）
| 值 | 名称 | 描述 |
|---|---|---|
| 0 | MAV_MODE_PREFLIGHT | 系统未准备好飞行，正在启动、校准等。未设置任何标志 |
| 64 | MAV_MODE_MANUAL_DISARMED | 系统允许激活，处于手动（RC）控制，无稳定（MAV_MODE_FLAG_MANUAL_INPUT_ENABLED） |
| 80 | MAV_MODE_STABILIZE_DISARMED | 系统允许激活，处于辅助RC控制（MAV_MODE_FLAG_SAFETY_ARMED, MAV_MODE_FLAG_STABILIZE_ENABLED） |
| 88 | MAV_MODE_GUIDED_DISARMED | 系统允许激活，处于自主控制，手动设定点（MAV_MODE_FLAG_SAFETY_ARMED, MAV_MODE_FLAG_STABILIZE_ENABLED, MAV_MODE_FLAG_GUIDED_ENABLED） |
| 92 | MAV_MODE_AUTO_DISARMED | 系统允许激活，处于自主控制和导航（MAV_MODE_FLAG_SAFETY_ARMED, MAV_MODE_FLAG_STABILIZE_ENABLED, MAV_MODE_FLAG_GUIDED_ENABLED, MAV_MODE_FLAG_AUTO_ENABLED） |
| 192 | MAV_MODE_MANUAL_ARMED | 系统允许激活，处于手动（RC）控制，无稳定（MAV_MODE_FLAG_SAFETY_ARMED, MAV_MODE_FLAG_MANUAL_INPUT_ENABLED） |
| 194 | MAV_MODE_TEST_ARMED | 未定义模式。这完全取决于自动驾驶仪 - 谨慎使用，仅供开发人员使用（MAV_MODE_FLAG_SAFETY_ARMED, MAV_MODE_FLAG_MANUAL_INPUT_ENABLED, MAV_MODE_FLAG_TEST_ENABLED） |
| 208 | MAV_MODE_STABILIZE_ARMED | 系统允许激活，处于辅助RC控制（MAV_MODE_FLAG_SAFETY_ARMED, MAV_MODE_FLAG_MANUAL_INPUT_ENABLED, MAV_MODE_FLAG_STABILIZE_ENABLED） |
| 216 | MAV_MODE_GUIDED_ARMED | 系统允许激活，处于自主控制，手动设定点（MAV_MODE_FLAG_SAFETY_ARMED, MAV_MODE_FLAG_MANUAL_INPUT_ENABLED, MAV_MODE_FLAG_STABILIZE_ENABLED, MAV_MODE_FLAG_GUIDED_ENABLED） |
| 220 | MAV_MODE_AUTO_ARMED | 系统允许激活，处于自主控制和导航（MAV_MODE_FLAG_SAFETY_ARMED, MAV_MODE_FLAG_MANUAL_INPUT_ENABLED, MAV_MODE_FLAG_STABILIZE_ENABLED, MAV_MODE_FLAG_GUIDED_ENABLED, MAV_MODE_FLAG_AUTO_ENABLED） |
| 66 | MAV_MODE_TEST_DISARMED | 未定义模式。这完全取决于自动驾驶仪 - 谨慎使用，仅限开发人员（MAV_MODE_FLAG_MANUAL_INPUT_ENABLED, MAV_MODE_FLAG_TEST_ENABLED） |

### MAV_SYS_STATUS_SENSOR（传感器状态位图）
| 值 | 名称 | 描述 |
|---|---|---|
| 1 | MAV_SYS_STATUS_SENSOR_3D_GYRO | 0x01 3D陀螺仪 |
| 2 | MAV_SYS_STATUS_SENSOR_3D_ACCEL | 0x02 3D加速度计 |
| 4 | MAV_SYS_STATUS_SENSOR_3D_MAG | 0x04 3D磁力计 |
| 8 | MAV_SYS_STATUS_SENSOR_ABSOLUTE_PRESSURE | 0x08 绝对压力 |
| 16 | MAV_SYS_STATUS_SENSOR_DIFFERENTIAL_PRESSURE | 0x10 差分压力 |
| 32 | MAV_SYS_STATUS_SENSOR_GPS | 0x20 GPS |
| 64 | MAV_SYS_STATUS_SENSOR_OPTICAL_FLOW | 0x40 光流 |
| 128 | MAV_SYS_STATUS_SENSOR_VISION_POSITION | 0x80 计算机视觉位置 |
| 256 | MAV_SYS_STATUS_SENSOR_LASER_POSITION | 0x100 激光定位 |
| 512 | MAV_SYS_STATUS_SENSOR_EXTERNAL_GROUND_TRUTH | 0x200 外部地面真值（Vicon或Leica） |
| 1024 | MAV_SYS_STATUS_SENSOR_ANGULAR_RATE_CONTROL | 0x400 3D角速率控制 |
| 2048 | MAV_SYS_STATUS_SENSOR_ATTITUDE_STABILIZATION | 0x800 姿态稳定 |
| 4096 | MAV_SYS_STATUS_SENSOR_YAW_POSITION | 0x1000 偏航位置 |
| 8192 | MAV_SYS_STATUS_SENSOR_Z_ALTITUDE_CONTROL | 0x2000 Z/高度控制 |
| 16384 | MAV_SYS_STATUS_SENSOR_XY_POSITION_CONTROL | 0x4000 XY位置控制 |
| 32768 | MAV_SYS_STATUS_SENSOR_MOTOR_OUTPUTS | 0x8000 电机输出/控制 |
| 65536 | MAV_SYS_STATUS_SENSOR_RC_RECEIVER | 0x10000 RC接收机 |
| 131072 | MAV_SYS_STATUS_SENSOR_3D_GYRO2 | 0x20000 第二个3D陀螺仪 |
| 262144 | MAV_SYS_STATUS_SENSOR_3D_ACCEL2 | 0x40000 第二个3D加速度计 |
| 524288 | MAV_SYS_STATUS_SENSOR_3D_MAG2 | 0x80000 第二个3D磁力计 |
| 1048576 | MAV_SYS_STATUS_GEOFENCE | 0x100000 地理围栏 |
| 2097152 | MAV_SYS_STATUS_AHRS | 0x200000 AHRS子系统健康状况 |
| 4194304 | MAV_SYS_STATUS_TERRAIN | 0x400000 地形子系统健康状况 |
| 8388608 | MAV_SYS_STATUS_REVERSE_MOTOR | 0x800000 电机反转 |
| 16777216 | MAV_SYS_STATUS_LOGGING | 0x1000000 日志记录 |
| 33554432 | MAV_SYS_STATUS_SENSOR_BATTERY | 0x2000000 电池 |
| 67108864 | MAV_SYS_STATUS_SENSOR_PROXIMITY | 0x4000000 接近传感器 |
| 134217728 | MAV_SYS_STATUS_SENSOR_SATCOM | 0x8000000 卫星通信 |
| 268435456 | MAV_SYS_STATUS_PREARM_CHECK | 0x10000000 预解锁检查状态。解锁时始终健康 |
| 536870912 | MAV_SYS_STATUS_OBSTACLE_AVOIDANCE | 0x20000000 避障/碰撞预防 |
| 1073741824 | MAV_SYS_STATUS_SENSOR_PROPULSION | 0x40000000 推进（执行器、ESC、电机或螺旋桨） |
| 2147483648 | MAV_SYS_STATUS_EXTENSION_USED | 0x80000000 扩展位用于进一步的传感器状态位（仅在onboard_control_sensors_present中设置） |

### MAV_SYS_STATUS_SENSOR_EXTENDED（扩展传感器状态位图）
| 值 | 名称 | 描述 |
|---|---|---|
| 1 | MAV_SYS_STATUS_RECOVERY_SYSTEM | 0x01 回收系统（降落伞、气球、回收装置等） |
| 2 | MAV_SYS_STATUS_SENSOR_LEAK | 0x02 泄漏检测 |

### MAV_FRAME（坐标系）
| 值 | 名称 | 描述 |
|---|---|---|
| 0 | MAV_FRAME_GLOBAL | 全球（WGS84）坐标系 + 相对于平均海平面（MSL）的高度 |
| 1 | MAV_FRAME_LOCAL_NED | NED局部切线框架（x：北，y：东，z：下），原点相对于地球固定 |
| 2 | MAV_FRAME_MISSION | 非坐标系，表示任务命令 |
| 3 | MAV_FRAME_GLOBAL_RELATIVE_ALT | 全球（WGS84）坐标系 + 相对于家庭位置的高度 |
| 4 | MAV_FRAME_LOCAL_ENU | ENU局部切线框架（x：东，y：北，z：上），原点相对于地球固定 |
| 5 | MAV_FRAME_GLOBAL_INT | 全球（WGS84）坐标系（缩放）+ 相对于平均海平面（MSL）的高度 |
| 6 | MAV_FRAME_GLOBAL_RELATIVE_ALT_INT | 全球（WGS84）坐标系（缩放）+ 相对于家庭位置的高度 |
| 7 | MAV_FRAME_LOCAL_OFFSET_NED | NED局部切线框架（x：北，y：东，z：下），原点随载具移动 |
| 8 | MAV_FRAME_BODY_NED | 与MAV_FRAME_LOCAL_NED相同用于位置值。与MAV_FRAME_BODY_FRD相同用于速度/加速度值 |
| 9 | MAV_FRAME_BODY_OFFSET_NED | 与MAV_FRAME_BODY_FRD相同 |
| 10 | MAV_FRAME_GLOBAL_TERRAIN_ALT | 全球（WGS84）坐标系 + AGL高度（地面高度） |
| 11 | MAV_FRAME_GLOBAL_TERRAIN_ALT_INT | 全球（WGS84）坐标系（缩放）+ AGL高度（地面高度） |
| 12 | MAV_FRAME_BODY_FRD | FRD局部框架，与载具姿态对齐（x：前，y：右，z：下），原点随载具移动 |
| 13 | MAV_FRAME_RESERVED_13 | 机身固定参考系，Z向上（x：前，y：左，z：上） |
| 14 | MAV_FRAME_RESERVED_14 | MOCAP_NED - 运动捕捉系统给出的里程计局部坐标系，Z向下（x：北，y：东，z：下） |
| 15 | MAV_FRAME_RESERVED_15 | MOCAP_ENU - 运动捕捉系统给出的里程计局部坐标系，Z向上（x：东，y：北，z：上） |
| 16 | MAV_FRAME_RESERVED_16 | VISION_NED - 视觉估计系统给出的里程计局部坐标系，Z向下（x：北，y：东，z：下） |
| 17 | MAV_FRAME_RESERVED_17 | VISION_ENU - 视觉估计系统给出的里程计局部坐标系，Z向上（x：东，y：北，z：上） |
| 18 | MAV_FRAME_RESERVED_18 | ESTIM_NED - 载具上运行的估计器给出的里程计局部坐标系，Z向下（x：北，y：东，z：下） |
| 19 | MAV_FRAME_RESERVED_19 | ESTIM_ENU - 载具上运行的估计器给出的里程计局部坐标系，Z向上（x：东，y：北，z：上） |
| 20 | MAV_FRAME_LOCAL_FRD | FRD局部切线框架（x：前，y：右，z：下），原点相对于地球固定。前轴在水平面上与载具前方对齐 |
| 21 | MAV_FRAME_LOCAL_FLU | FLU局部切线框架（x：前，y：左，z：上），原点相对于地球固定。前轴在水平面上与载具前方对齐 |

### FENCE_BREACH（围栏违规类型）
| 值 | 名称 | 描述 |
|---|---|---|
| 0 | FENCE_BREACH_NONE | 无最后围栏违规 |
| 1 | FENCE_BREACH_MINALT | 违规最小高度 |
| 2 | FENCE_BREACH_MAXALT | 违规最大高度 |
| 3 | FENCE_BREACH_BOUNDARY | 违规围栏边界 |

### FENCE_MITIGATE（围栏缓解措施）
| 值 | 名称 | 描述 |
|---|---|---|
| 0 | FENCE_MITIGATE_UNKNOWN | 未知 |
| 1 | FENCE_MITIGATE_NONE | 未采取任何行动 |
| 2 | FENCE_MITIGATE_VEL_LIMIT | 激活速度限制以防止违规 |

### FENCE_TYPE（围栏类型）
| 值 | 名称 | 描述 |
|---|---|---|
| 1 | FENCE_TYPE_ALT_MAX | 最大高度围栏 |
| 2 | FENCE_TYPE_CIRCLE | 圆形围栏 |
| 4 | FENCE_TYPE_POLYGON | 多边形围栏 |
| 8 | FENCE_TYPE_ALT_MIN | 最小高度围栏 |

### MAV_MOUNT_MODE（云台模式）
| 值 | 名称 | 描述 |
|---|---|---|
| 0 | MAV_MOUNT_MODE_RETRACT | 从永久内存加载并保持安全位置（Roll,Pitch,Yaw）并停止稳定 |
| 1 | MAV_MOUNT_MODE_NEUTRAL | 从永久内存加载并保持中性位置（Roll,Pitch,Yaw） |
| 2 | MAV_MOUNT_MODE_MAVLINK_TARGETING | 加载中性位置并开始MAVLink Roll,Pitch,Yaw控制并稳定 |
| 3 | MAV_MOUNT_MODE_RC_TARGETING | 加载中性位置并开始RC Roll,Pitch,Yaw控制并稳定 |
| 4 | MAV_MOUNT_MODE_GPS_POINT | 加载中性位置并开始指向Lat,Lon,Alt |
| 5 | MAV_MOUNT_MODE_SYSID_TARGET | 云台跟踪具有指定系统ID的系统 |
| 6 | MAV_MOUNT_MODE_HOME_LOCATION | 云台跟踪家庭位置 |
| 7 | MAV_MOUNT_MODE_WPNEXT_OFFSET | 云台跟踪下一个航路点位置并带偏移 |

### GIMBAL_DEVICE_CAP_FLAGS（云台设备能力标志）
| 值 | 名称 | 描述 |
|---|---|---|
| 1 | GIMBAL_DEVICE_CAP_FLAGS_HAS_RETRACT | 云台设备支持收回位置 |

### MAVLINK_DATA_STREAM_TYPE（数据流类型）
| 值 | 名称 | 描述 |
|---|---|---|
| 0 | MAVLINK_DATA_STREAM_IMG_JPEG | JPEG图像 |
| 1 | MAVLINK_DATA_STREAM_IMG_BMP | BMP图像 |
| 2 | MAVLINK_DATA_STREAM_IMG_RAW8U | 8位原始图像 |
| 3 | MAVLINK_DATA_STREAM_IMG_RAW32U | 32位原始图像 |
| 4 | MAVLINK_DATA_STREAM_IMG_PGM | PGM图像 |
| 5 | MAVLINK_DATA_STREAM_IMG_PNG | PNG图像 |
