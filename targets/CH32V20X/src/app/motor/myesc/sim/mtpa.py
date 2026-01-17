import numpy as np
import matplotlib.pyplot as plt
from typing import Tuple, List, Optional
from dataclasses import dataclass
import matplotlib.pyplot as plt
import numpy as np
from typing import List, Tuple, Union

# 全局常量定义
FLUX_LINKAGE: float = 0.1  # 永磁体磁链 (Wb)
LD: float = 20e-4           # d轴电感 (H)
LQ: float = 40e-4           # q轴电感 (H)
MAX_CURRENT: float = 10.0  # 最大电流 (A)
SEGMENT_NUM: int = 100      # 分段数量
CURRENT_STEP: float = MAX_CURRENT / 100  # 电流步长

@dataclass
class MTPAParams:
    """MTPA参数数据类"""
    flux_linkage: float
    ld: float
    lq: float
    max_current: float
    segment_num: int


def compute_mtpa_id_analytical(params: MTPAParams, iq: float) -> Union[float, complex]:
    """
    计算MTPA理论id值（解析解法）
    
    根据MTPA条件：∂Te/∂id = 0 推导得到：
    id = (ψpm / (Lq - Ld)) ± sqrt((ψpm / (Lq - Ld))² + iq²)
    对于IPMSM (Lq > Ld)，取负号；对于反凸极(Lq < Ld)，取正号
    
    Args:
        params: MTPA参数
        iq: q轴电流 (A)
        
    Returns:
        d轴电流参考值 (A)，如果根号内为负数返回复数
    """
    # 处理表面贴装PMSM情况
    if abs(params.lq - params.ld) < 1e-10:
        # 表面贴装PMSM: Ld ≈ Lq，MTPA操作点为id=0
        return 0.0
    
    # 计算MTPA解析解
    delta_l = params.lq - params.ld
    psi_term = params.flux_linkage / delta_l
    
    # 根号内的项
    sqrt_term = psi_term**2 + iq**2
    
    if sqrt_term >= 0:
        sqrt_val = np.sqrt(sqrt_term)
    else:
        # 根号内为负数，返回复数（通常不会发生，除非参数错误）
        sqrt_val = np.sqrt(-sqrt_term) * 1j
    
    # 根据电感关系选择符号
    if delta_l > 0:
        # IPMSM: Lq > Ld，取负号
        return psi_term - sqrt_val
    else:
        # 反凸极: Lq < Ld，取正号
        return psi_term + sqrt_val


def compute_mtpa_id_numerical(params: MTPAParams, iq: float) -> float:
    """
    计算MTPA理论id值（数值稳定版本）
    
    Args:
        params: MTPA参数
        iq: q轴电流 (A)
        
    Returns:
        d轴电流参考值 (A)，确保在电流极限内
    """
    # 处理表面贴装PMSM
    if abs(params.lq - params.ld) < 1e-10:
        return 0.0
    
    delta_l = params.lq - params.ld
    psi = params.flux_linkage
    
    # 更稳定的计算方法
    if delta_l > 0:
        # IPMSM情况
        if abs(iq) < 1e-10:
            # 当iq接近0时的特例
            id_mtpa = -psi / (2 * delta_l)
        else:
            # 标准MTPA公式
            term = (psi / (2 * delta_l))**2 + iq**2
            if term >= 0:
                id_mtpa = -psi / (2 * delta_l) - np.sqrt(term)
            else:
                # 数值处理：使用近似
                id_mtpa = -psi / delta_l
    else:
        # 反凸极情况
        term = (psi / (2 * delta_l))**2 + iq**2
        if term >= 0:
            id_mtpa = -psi / (2 * delta_l) + np.sqrt(term)
        else:
            id_mtpa = 0.0
    
    # 确保电流在极限范围内
    i_total = np.sqrt(id_mtpa**2 + iq**2)
    if i_total > params.max_current:
        # 按比例缩放
        scale = params.max_current / i_total
        id_mtpa *= scale
    
    return float(id_mtpa)


def compute_mtpa_trajectory(params: MTPAParams) -> tuple[np.ndarray, np.ndarray]:
    """
    计算完整的MTPA轨迹
    
    Args:
        params: MTPA参数
        
    Returns:
        (id_array, iq_array): MTPA轨迹上的电流点
    """
    iq_range = np.linspace(0, params.max_current, params.segment_num)
    id_array = np.zeros_like(iq_range)
    
    for i, iq_val in enumerate(iq_range):
        id_array[i] = compute_mtpa_id_numerical(params, iq_val)
    
    return id_array, iq_range

def plot_mtpa_relationship(params: MTPAParams, 
                          current_step: float = 0.1) -> plt.Figure:
    """
    绘制MTPA的iq-id关系图
    
    Args:
        params: MTPA参数
        current_step: 电流步长 (A)，用于绘制连续曲线
        
    Returns:
        matplotlib图形对象
    """
    # 生成连续的iq值用于理论曲线绘制
    iq_values: np.ndarray = np.arange(0, params.max_current, current_step)
    
    # 计算理论id值（使用数值稳定版本）
    id_theoretical: List[float] = []
    for iq in iq_values:
        id_val = compute_mtpa_id_numerical(params, iq)
        # 只保留实数部分（复数情况通常为参数错误）
        id_theoretical.append(id_val.real if isinstance(id_val, complex) else id_val)
    
    # 创建图形
    fig, ax = plt.subplots(figsize=(12, 10))
    
    # 绘制理论曲线
    ax.plot(iq_values, id_theoretical, 'b-', linewidth=2.5, 
            label='Theoretical MTPA Curve', zorder=2)
    
    # 添加分段标记
    seg_points_x: List[float] = []
    seg_points_y: List[float] = []
    
    for i in range(params.segment_num + 1):
        iq_seg: float = i * params.max_current / params.segment_num
        id_seg = compute_mtpa_id_numerical(params, iq_seg)
        seg_points_x.append(iq_seg)
        # 处理可能返回的复数
        seg_points_y.append(id_seg.real if isinstance(id_seg, complex) else id_seg)
    
    ax.scatter(seg_points_x, seg_points_y, color='red', s=100, 
               marker='o', label=f'Segment Points (N={params.segment_num})', 
               zorder=4, edgecolors='black', linewidth=1.5)
    
    # 绘制电流极限圆（最大电流限制）
    theta = np.linspace(0, 2*np.pi, 200)
    i_max = params.max_current
    id_circle = i_max * np.cos(theta)
    iq_circle = i_max * np.sin(theta)
    
    ax.plot(iq_circle, id_circle, 'r--', linewidth=1.5, alpha=0.7, 
            label=f'Current Limit ({i_max}A)')
    
    # 标记MTPA曲线的起点和终点
    if len(id_theoretical) > 0:
        ax.scatter(0, id_theoretical[0], color='green', s=150, 
                  marker='^', label='Start (iq=0)', zorder=5)
        ax.scatter(iq_values[-1], id_theoretical[-1], color='purple', s=150,
                  marker='v', label=f'End (iq={iq_values[-1]:.1f}A)', zorder=5)
    
    # 设置图形属性
    ax.set_xlabel('q-axis Current ($i_q$) [A]', fontsize=13, fontweight='bold')
    ax.set_ylabel('d-axis Current ($i_d$) [A]', fontsize=13, fontweight='bold')
    
    # 添加电机类型标识
    if abs(params.lq - params.ld) < 1e-10:
        motor_type = "Surface PMSM"
    elif params.lq > params.ld:
        motor_type = "IPMSM (Interior PMSM)"
    else:
        motor_type = "Reverse Saliency PMSM"
    
    ax.set_title(f'Maximum Torque Per Ampere (MTPA) Relationship\n'
                 f'{motor_type} | '
                 f'$ψ_{{pm}}$={params.flux_linkage:.3f}Wb, '
                 f'$L_d$={params.ld*1000:.2f}mH, $L_q$={params.lq*1000:.2f}mH', 
                 fontsize=14, fontweight='bold', pad=20)
    
    ax.grid(True, linestyle='--', alpha=0.6, zorder=1)
    ax.legend(loc='upper right', fontsize=11, framealpha=0.9)
    
    # 设置坐标轴范围
    ax.set_xlim(0, params.max_current * 1.05)
    
    # 自动确定y轴范围（id通常为负值）
    if id_theoretical:
        id_min = min(id_theoretical)
        id_max = max(id_theoretical)
        ax.set_ylim(id_min * 1.1, max(id_max * 1.1, 0.1))
    
    # 添加网格线和零线
    ax.axhline(y=0, color='gray', linestyle=':', linewidth=1, alpha=0.5)
    ax.axvline(x=0, color='gray', linestyle=':', linewidth=1, alpha=0.5)
    
    # 添加注释说明
    annotation_text = (f'Parameters:\n'
                      f'• ψ_pm = {params.flux_linkage:.3f} Wb\n'
                      f'• L_d = {params.ld*1000:.2f} mH\n'
                      f'• L_q = {params.lq*1000:.2f} mH\n'
                      f'• I_max = {params.max_current} A\n'
                      f'• Segments: {params.segment_num}\n'
                      f'• ΔIq = {params.max_current/params.segment_num:.2f} A')
    
    ax.annotate(annotation_text, 
                xy=(0.02, 0.98), xycoords='axes fraction',
                bbox=dict(boxstyle='round,pad=0.3', facecolor='lightblue', 
                         alpha=0.8, edgecolor='blue'),
                verticalalignment='top', fontsize=10, fontfamily='monospace')
    
    # 添加当前曲线信息
    curve_info = (f'MTPA Curve Info:\n'
                  f'• Start: id={id_theoretical[0]:.2f}A @ iq=0A\n'
                  f'• End: id={id_theoretical[-1]:.2f}A @ iq={iq_values[-1]:.1f}A\n'
                  f'• Curve length: {len(iq_values)} points')
    
    ax.annotate(curve_info,
                xy=(0.98, 0.02), xycoords='axes fraction',
                bbox=dict(boxstyle='round,pad=0.3', facecolor='lightgreen',
                         alpha=0.8, edgecolor='green'),
                verticalalignment='bottom', horizontalalignment='right',
                fontsize=10, fontfamily='monospace')
    
    plt.tight_layout()
    return fig


def plot_mtpa_comparison(params: MTPAParams) -> None:
    """
    对比分析法和数值法计算的MTPA曲线
    
    Args:
        params: MTPA参数
    """
    fig, axes = plt.subplots(1, 2, figsize=(16, 7))
    
    # 生成iq值
    iq_values = np.linspace(0, params.max_current, 500)
    
    # 计算方法1: 分析法
    id_analytical = []
    for iq in iq_values:
        id_val = compute_mtpa_id_analytical(params, iq)
        id_analytical.append(id_val.real if isinstance(id_val, complex) else id_val)
    
    # 计算方法2: 数值法
    id_numerical = []
    for iq in iq_values:
        id_val = compute_mtpa_id_numerical(params, iq)
        id_numerical.append(id_val.real if isinstance(id_val, complex) else id_val)
    
    # 绘制分析法结果
    ax1 = axes[0]
    ax1.plot(iq_values, id_analytical, 'b-', linewidth=2, label='Analytical Method')
    ax1.set_xlabel('q-axis Current ($i_q$) [A]', fontsize=12)
    ax1.set_ylabel('d-axis Current ($i_d$) [A]', fontsize=12)
    ax1.set_title('Analytical MTPA Calculation', fontsize=14, fontweight='bold')
    ax1.grid(True, alpha=0.3)
    ax1.legend()
    
    # 绘制数值法结果
    ax2 = axes[1]
    ax2.plot(iq_values, id_numerical, 'r-', linewidth=2, label='Numerical Method')
    ax2.set_xlabel('q-axis Current ($i_q$) [A]', fontsize=12)
    ax2.set_ylabel('d-axis Current ($i_d$) [A]', fontsize=12)
    ax2.set_title('Numerical MTPA Calculation', fontsize=14, fontweight='bold')
    ax2.grid(True, alpha=0.3)
    ax2.legend()
    
    # 添加整体标题
    motor_type = "SPMSM" if abs(params.lq - params.ld) < 1e-10 else "IPMSM"
    fig.suptitle(f'MTPA Calculation Comparison - {motor_type}\n'
                 f'ψ_pm={params.flux_linkage:.3f}Wb, '
                 f'Ld={params.ld*1000:.2f}mH, Lq={params.lq*1000:.2f}mH',
                 fontsize=16, fontweight='bold')
    
    plt.tight_layout()
    plt.show()

    
def main() -> None:
    """主函数"""
    # 创建MTPA参数对象
    params: MTPAParams = MTPAParams(
        flux_linkage=FLUX_LINKAGE,
        ld=LD,
        lq=LQ,
        max_current=MAX_CURRENT,
        segment_num=SEGMENT_NUM
    )
    


    # 绘制MTPA关系图
    fig = plot_mtpa_relationship(params, current_step=0.5)
    plt.savefig('mtpa_curve.png', dpi=300, bbox_inches='tight')
    plt.show()

    # 绘制对比图
    plot_mtpa_comparison(params)

if __name__ == "__main__":
    main()