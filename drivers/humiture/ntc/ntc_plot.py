import numpy as np
import matplotlib.pyplot as plt

# ==================== 参数配置 ====================
# NTC参数 (B=3950, R0=100kΩ)
B0 = 3950.0
R0_KOHMS = 10.0
RDIV_KOHMS = 100.0  # 分压电阻值 (根据你的实际电路调整)
TABLE_SIZE = 32
ADC_MAX = 4096



# ==================== 方法1：通用解决方案（推荐）====================
def setup_chinese_font():
    """设置matplotlib支持中文显示"""
    # 尝试设置中文字体
    try:
        # Windows系统
        plt.rcParams['font.sans-serif'] = ['SimHei', 'Microsoft YaHei', 'SimSun']
        # macOS系统（如果需要）
        # plt.rcParams['font.sans-serif'] = ['PingFang SC', 'Heiti SC', 'STHeiti']
        # Linux系统（如果需要）
        # plt.rcParams['font.sans-serif'] = ['WenQuanYi Micro Hei', 'Noto Sans CJK SC']
        
        plt.rcParams['axes.unicode_minus'] = False  # 解决负号显示问题
        print("✅ 中文字体设置成功")
    except Exception as e:
        print(f"⚠️ 中文字体设置失败: {e}")

# 在绘图前调用
setup_chinese_font()

# ==================== 方法2：临时设置（针对单次绘图）====================
def plot_with_chinese():
    """带中文支持的绘图示例"""
    # 临时设置
    plt.rcParams['font.sans-serif'] = ['SimHei']
    plt.rcParams['axes.unicode_minus'] = False
    
    # 你的绘图代码
    fig, ax = plt.subplots()
    ax.plot([1, 2, 3], [1, 4, 9])
    ax.set_xlabel('横坐标 (ADC值)')
    ax.set_title('NTC温度曲线对比')
    plt.show()

# ==================== 方法3：检查可用字体（调试用）====================
def list_available_fonts():
    """列出系统可用的中文字体（用于调试）"""
    from matplotlib.font_manager import FontManager
    fm = FontManager()
    chinese_fonts = [f.name for f in fm.ttflist if any(keyword in f.name.lower() for keyword in ['sim', 'hei', 'song', 'kai', 'ming', 'yahei', 'pingfang', 'noto'])]
    print("可用的中文字体:")
    for font in chinese_fonts[:10]:  # 只显示前10个
        print(f"  - {font}")
    return chinese_fonts


# ==================== B参数公式实现 ====================
def b_param_to_celsius(rt_kohms, b0=B0, r0_kohms=R0_KOHMS):
    """B参数公式: 电阻→温度(°C)"""
    T0 = 273.15 + 25.0  # 298.15K
    if rt_kohms <= 0:
        return -273.15  # 防止log(负数)
    inv_kelvin = 1.0 / T0 + (1.0 / b0) * np.log(rt_kohms / r0_kohms)
    kelvin = 1.0 / inv_kelvin
    return kelvin - 273.15

def voltage_ratio_to_resistance(ratio, rdiv_kohms=RDIV_KOHMS):
    """分压比 → NTC电阻值 (kΩ)"""
    if ratio <= 0 or ratio >= 1:
        return float('inf')
    return (rdiv_kohms / ratio) - rdiv_kohms

def adc_to_celsius_bparam(adc_value, adc_max=ADC_MAX, rdiv_kohms=RDIV_KOHMS):
    """ADC值 → 温度(°C) - B参数公式直接计算"""
    if adc_value <= 0 or adc_value >= adc_max:
        return float('inf')
    ratio = adc_value / adc_max
    rt_kohms = voltage_ratio_to_resistance(ratio, rdiv_kohms)
    return b_param_to_celsius(rt_kohms)

# ==================== 32点插值表实现 ====================
def make_thermistor_table(b0, r0_kohms, rdiv_kohms, table_size=TABLE_SIZE):
    """生成32点温度插值表"""
    table = np.zeros(table_size)

    
    for i in range(1, table_size):
        norm_v = i / table_size  # 归一化电压 (不是ADC值!)
        if norm_v <= 0 or norm_v >= 1:
            rt_kohms = float('inf')
        else:
            rt_kohms = (rdiv_kohms / norm_v) - rdiv_kohms
        table[i] = b_param_to_celsius(rt_kohms, b0, r0_kohms)
    
    table[0] = table[1]
    return table

def adc_to_celsius_interp(adc_value, table, adc_max=ADC_MAX, table_size=TABLE_SIZE):
    """ADC值 → 温度(°C) - 插值表查表"""
    adc_step = adc_max // table_size
    
    # 边界处理
    if adc_value <= 0:
        return table[0]
    if adc_value >= adc_max - 1:
        return table[-1]
    
    # 找到索引 (与C++代码逻辑一致)
    offset_idx = int(adc_value / adc_step)
    offset_idx = max(1, min(table_size - 2, offset_idx))
    
    this_adc_value = offset_idx * adc_step
    temp1 = table[offset_idx]
    temp2 = table[offset_idx + 1]
    lerp_ratio = (adc_value - this_adc_value) / adc_step
    
    return temp1 + (temp2 - temp1) * lerp_ratio

# ==================== 生成对比数据 ====================
def main():
        # 设置中文支持
    plt.rcParams['font.sans-serif'] = ['SimHei']  # 或者 'Microsoft YaHei'
    plt.rcParams['axes.unicode_minus'] = False
    
    # 生成插值表
    table = make_thermistor_table(B0, R0_KOHMS, RDIV_KOHMS, TABLE_SIZE)
    
    # ADC值范围 (0~4095)
    adc_values = np.arange(0, ADC_MAX, 1)
    
    # 计算两种方法的温度
    temps_bparam = np.zeros_like(adc_values, dtype=float)
    temps_interp = np.zeros_like(adc_values, dtype=float)
    
    for i, adc in enumerate(adc_values):
        # B参数公式 (直接计算)
        try:
            temps_bparam[i] = adc_to_celsius_bparam(adc)
        except:
            temps_bparam[i] = np.nan
            
        # 插值表查表
        try:
            temps_interp[i] = adc_to_celsius_interp(adc, table)
        except:
            temps_interp[i] = np.nan
    
    # 计算误差
    valid_mask = ~np.isnan(temps_bparam) & ~np.isnan(temps_interp)
    temps_diff = temps_interp - temps_bparam
    temps_diff[~valid_mask] = np.nan
    
    # ==================== 绘图 ====================
    fig, (ax1) = plt.subplots(1, 1, figsize=(12, 10), sharex=True)
    
    # 图1: 温度曲线对比
    ax1.plot(adc_values, temps_bparam, 'b-', label='B参数公式 (直接计算)', linewidth=1.5)
    ax1.plot(adc_values, temps_interp, 'r--', label=f'{TABLE_SIZE}点插值表', linewidth=1.5, alpha=0.7)
    
    # 标记插值点位置
    for i in range(TABLE_SIZE):
        adc_pos = i * (ADC_MAX // TABLE_SIZE)
        if i < len(table):
            ax1.plot(adc_pos, table[i], 'ko', markersize=3)
    
    ax1.set_ylabel('温度 (°C)')
    ax1.set_title(f'NTC温度曲线对比 (B={B0}, R0={R0_KOHMS}kΩ, Rdiv={RDIV_KOHMS}kΩ)')
    ax1.legend()
    ax1.grid(True, alpha=0.3)
    
    # # 图2: 绝对误差
    # ax2.plot(adc_values, temps_diff, 'g-', linewidth=1)
    # ax2.axhline(y=0, color='k', linestyle='--', linewidth=0.5)
    # ax2.set_ylabel('绝对误差 (°C)')
    # ax2.set_title('插值表 vs B参数公式 的绝对误差')
    # ax2.grid(True, alpha=0.3)
    
    # # 标出最大误差
    # max_error_idx = np.nanargmax(np.abs(temps_diff))
    # max_error_val = temps_diff[max_error_idx]
    # ax2.plot(adc_values[max_error_idx], max_error_val, 'ro', markersize=6)
    # ax2.annotate(f'Max Error: {max_error_val:.3f}°C',
    #              xy=(adc_values[max_error_idx], max_error_val),
    #              xytext=(adc_values[max_error_idx]+200, max_error_val+2),
    #              arrowprops=dict(arrowstyle='->', color='red'))
    
    # # 图3: 相对误差分布 (百分比)
    # rel_error = np.abs(temps_diff / temps_bparam) * 100
    # rel_error[~valid_mask] = np.nan
    # ax3.plot(adc_values, rel_error, 'm-', linewidth=1)
    # ax3.set_ylabel('相对误差 (%)')
    # ax3.set_xlabel('ADC值 (0~4095)')
    # ax3.set_title('插值表 vs B参数公式 的相对误差')
    # ax3.grid(True, alpha=0.3)
    
    # 统计信息
    if valid_mask.any():
        mean_error = np.nanmean(np.abs(temps_diff[valid_mask]))
        max_error_abs = np.nanmax(np.abs(temps_diff[valid_mask]))
        print(f"📊 统计信息 (有效ADC范围: {adc_values[valid_mask][0]} ~ {adc_values[valid_mask][-1]}):")
        print(f"  - 平均绝对误差: {mean_error:.4f} °C")
        print(f"  - 最大绝对误差: {max_error_abs:.4f} °C")
        print(f"  - 温度范围: {np.nanmin(temps_bparam):.1f}°C ~ {np.nanmax(temps_bparam):.1f}°C")
    
    plt.tight_layout()
    # plt.savefig('ntc_comparison.png', dpi=150)
    plt.show()
    
    # # 打印插值表内容 (调试用)
    # print("\n📋 插值表 (ADC索引 → 温度):")
    # for i, temp in enumerate(table):
    #     adc_pos = i * (ADC_MAX // TABLE_SIZE)
    #     print(f"  ADC[{adc_pos:4d}] → {temp:8.3f}°C")

if __name__ == "__main__":
    main()