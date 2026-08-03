import numpy as np
import matplotlib.pyplot as plt
from scipy import signal
import control

#plt设置合理的中文字体
plt.rcParams['font.sans-serif'] = ['SimHei']

def backward_difference_lpf_coefficients(fs, fc):
    """
    计算后向差分法的一阶低通滤波器系数
    
    参数:
    fc: 截止频率 (Hz)
    fs: 采样频率 (Hz)
    
    返回:
    b, a: 离散滤波器系数
    """
    # 计算数字域截止频率
    omega_c = 2 * np.pi * fc
    T = 1 / fs
    
    # 模拟滤波器传递函数: H(s) = ω_c / (s + ω_c)
    # 使用后向差分法: s = (1 - z^(-1)) / T
    
    # 离散化
    b = np.array([omega_c * T])  # 分子系数
    a = np.array([omega_c * T + 1, -1])  # 分母系数
    
    # 重新排列为标准的b, a格式 (升幂排列)
    b = np.array([0, b[0]])  # b0, b1
    a = np.array([a[0], a[1]])  # a0, a1
    
    return b, a

def theoretical_discrete_lpf(fs, fc):
    """
    理论上的一阶低通滤波器离散传递函数
    H(z) = (1 - α) / (1 - α*z^(-1)), 其中 α = exp(-ω_c*T)
    """
    omega_c = 2 * np.pi * fc
    T = 1 / fs
    
    alpha = np.exp(-omega_c * T)
    
    b = np.array([1 - alpha])  # 分子系数
    a = np.array([1, -alpha])  # 分母系数
    
    return b, a

def plot_bode_comparison(fs, fc):
    """
    绘制伯德图比较不同方法得到的滤波器
    """
    # 生成频率范围
    freqs = np.logspace(0, np.log10(fs/2), 1000)
    w = 2 * np.pi * freqs
    
    # 1. 模拟滤波器（理想情况）
    s = 1j * w
    H_analog = (2 * np.pi * fc) / (s + 2 * np.pi * fc)
    
    # 2. 后向差分法得到的数字滤波器
    b_backward, a_backward = backward_difference_lpf_coefficients(fs, fc)
    H_backward = signal.freqz(b_backward, a_backward, worN=w, fs=fs)[1]
    
    # 3. 理论上精确的数字滤波器
    b_theory, a_theory = theoretical_discrete_lpf(fs, fc)
    H_theory = signal.freqz(b_theory, a_theory, worN=w, fs=fs)[1]
    
    # 4. 使用双线性变换作为参考
    b_bilinear, a_bilinear = signal.bilinear([2 * np.pi * fc], 
                                           [1, 2 * np.pi * fc], 
                                           fs=fs)
    H_bilinear = signal.freqz(b_bilinear, a_bilinear, worN=w, fs=fs)[1]
    
    # 创建图形
    fig, ((ax1, ax2), (ax3, ax4)) = plt.subplots(2, 2, figsize=(14, 10))
    
    # 绘制幅度响应
    ax1.semilogx(freqs, 20 * np.log10(np.abs(H_analog)), 'k-', linewidth=2, label='模拟滤波器')
    ax1.semilogx(freqs, 20 * np.log10(np.abs(H_backward)), 'r--', linewidth=2, label='后向差分')
    ax1.semilogx(freqs, 20 * np.log10(np.abs(H_theory)), 'b:', linewidth=2, label='理论离散')
    ax1.semilogx(freqs, 20 * np.log10(np.abs(H_bilinear)), 'g-.', linewidth=1.5, label='双线性变换')
    
    ax1.set_xlabel('频率 (Hz)')
    ax1.set_ylabel('幅度 (dB)')
    ax1.set_title('幅度响应比较')
    ax1.grid(True, which='both', alpha=0.3)
    ax1.legend()
    ax1.axvline(fc, color='gray', linestyle='--', alpha=0.5)
    ax1.set_ylim(-40, 5)
    
    # 绘制相位响应
    ax2.semilogx(freqs, np.angle(H_analog, deg=True), 'k-', linewidth=2, label='模拟滤波器')
    ax2.semilogx(freqs, np.angle(H_backward, deg=True), 'r--', linewidth=2, label='后向差分')
    ax2.semilogx(freqs, np.angle(H_theory, deg=True), 'b:', linewidth=2, label='理论离散')
    ax2.semilogx(freqs, np.angle(H_bilinear, deg=True), 'g-.', linewidth=1.5, label='双线性变换')
    
    ax2.set_xlabel('频率 (Hz)')
    ax2.set_ylabel('相位 (度)')
    ax2.set_title('相位响应比较')
    ax2.grid(True, which='both', alpha=0.3)
    ax2.legend()
    ax2.axvline(fc, color='gray', linestyle='--', alpha=0.5)
    
    # 绘制误差分析
    ax3.semilogx(freqs, 20 * np.log10(np.abs(H_backward)) - 20 * np.log10(np.abs(H_theory)), 
                'r-', linewidth=2, label='后向差分误差')
    ax3.semilogx(freqs, 20 * np.log10(np.abs(H_bilinear)) - 20 * np.log10(np.abs(H_theory)), 
                'g-', linewidth=1.5, label='双线性变换误差')
    
    ax3.set_xlabel('频率 (Hz)')
    ax3.set_ylabel('幅度误差 (dB)')
    ax3.set_title('幅度响应误差（相对于理论离散）')
    ax3.grid(True, which='both', alpha=0.3)
    ax3.legend()
    ax3.axvline(fc, color='gray', linestyle='--', alpha=0.5)
    ax3.axhline(0, color='k', linestyle='-', alpha=0.3)
    
    # 绘制脉冲响应
    t = np.arange(0, 5/fc, 1/fs)
    
    # 创建单位脉冲
    impulse = np.zeros_like(t)
    impulse[0] = 1
    
    # 计算脉冲响应
    response_backward = signal.lfilter(b_backward, a_backward, impulse)
    response_theory = signal.lfilter(b_theory, a_theory, impulse)
    
    ax4.plot(t, response_backward, 'r--', linewidth=2, label='后向差分')
    ax4.plot(t, response_theory, 'b:', linewidth=2, label='理论离散')
    ax4.plot(t, (2 * np.pi * fc) * np.exp(-2 * np.pi * fc * t), 
            'k-', alpha=0.5, linewidth=1, label='模拟响应')
    
    ax4.set_xlabel('时间 (秒)')
    ax4.set_ylabel('幅度')
    ax4.set_title('脉冲响应比较')
    ax4.grid(True, alpha=0.3)
    ax4.legend()
    
    plt.tight_layout()
    return fig

def main():
    # 设置滤波器参数
    fs = 10000 
    
    # 分析系数
    # analyze_coefficients(fs, fc)
    
    # # 绘制伯德图比较
    # fig = plot_bode_comparison(fs, fc)
    
    # 测试不同频率比的情况
    fig2, axes = plt.subplots(2, 2, figsize=(12, 8))
    frequency_ratios = [0.01, 0.1, 0.2, 0.5]  # 截止频率/采样频率 的比值
    
    for idx, ratio in enumerate(frequency_ratios):
        row = idx // 2
        col = idx % 2
        
        fc = fs * ratio
        freqs = np.logspace(0, np.log10(fs/2), 500)
        w = 2 * np.pi * freqs

        b_backward, a_backward = backward_difference_lpf_coefficients(fs, fc)
        b_theory, a_theory = theoretical_discrete_lpf(fs, fc)
        

        
        H_backward = signal.freqz(b_backward, a_backward, worN=w, fs=fs)[1]
        H_theory = signal.freqz(b_theory, a_theory, worN=w, fs=fs)[1]
        
        axes[row, col].semilogx(freqs, 20*np.log10(np.abs(H_backward)), 'r--', label='后向差分')
        axes[row, col].semilogx(freqs, 20*np.log10(np.abs(H_theory)), 'b:', label='理论离散')
        axes[row, col].axvline(fc, color='gray', linestyle='--', alpha=0.5)
        axes[row, col].set_title(f'f_c/fs = {ratio:.2f} (f_c={fc:.0f}Hz)')
        axes[row, col].set_xlabel('频率 (Hz)')
        axes[row, col].set_ylabel('幅度 (dB)')
        axes[row, col].grid(True, alpha=0.3)
        axes[row, col].legend(fontsize='small')
        axes[row, col].set_ylim(-40, 5)
    
    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    main()