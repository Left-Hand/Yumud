import numpy as np
import matplotlib.pyplot as plt

def visualize_filter_design():
    """可视化代码的设计过程"""
    
    # 参数
    bits = 7          # 2^5 = 32 taps
    taps = 1 << bits
    f_break = 0.2    # 截止频率（归一化）
    
    # 步骤1：理想频率响应
    H_ideal = np.zeros(taps)
    cutoff_idx = int(taps * f_break)
    H_ideal[:cutoff_idx] = 1
    # 对称化
    for i in range(taps//2):
        H_ideal[taps - i - 1] = H_ideal[i]
    
    # 步骤2：IFFT得到时域响应
    # 注意：代码中的filter_fft(1)应该是逆FFT
    h_time = np.fft.ifft(H_ideal).real
    
    # 步骤3：fftshift（循环移位）
    h_shifted = np.roll(h_time, taps//2)
    
    # 步骤4：加汉明窗
    window = 0.54 - 0.46 * np.cos(2 * np.pi * np.arange(taps) / (taps - 1))
    h_windowed = h_shifted
    # h_windowed = h_shifted * window
    
    # 绘图
    fig, axes = plt.subplots(3, 2, figsize=(12, 10))
    
    # 频率响应
    axes[0,0].plot(H_ideal)
    axes[0,0].set_title('理想频率响应 (对称后)')
    axes[0,0].axvline(cutoff_idx, color='r', linestyle='--')
    axes[0,0].grid(True)
    
    # 时域响应（IFFT后）
    axes[0,1].plot(h_time)
    axes[0,1].set_title('IFFT得到的脉冲响应')
    axes[0,1].grid(True)
    
    # 移位后
    axes[1,0].plot(h_shifted)
    axes[1,0].set_title('fftshift后（居中）')
    axes[1,0].grid(True)
    
    # 汉明窗
    axes[1,1].plot(window)
    axes[1,1].set_title('汉明窗')
    axes[1,1].grid(True)
    
    # 加窗后
    axes[2,0].plot(h_windowed)
    axes[2,0].set_title('加窗后的FIR系数')
    axes[2,0].grid(True)
    
    # 最终频率响应
    H_final = np.fft.fft(h_windowed)
    axes[2,1].plot(np.abs(H_final[:taps]))
    axes[2,1].set_title('最终滤波器频率响应')
    axes[2,1].axvline(cutoff_idx//2, color='r', linestyle='--')
    axes[2,1].grid(True)
    
    plt.tight_layout()
    plt.show()
    
    return h_windowed

# 运行可视化
fir_coeffs = visualize_filter_design()