#!/usr/bin/env python3

from scipy.signal import butter, freqz
import matplotlib.pyplot as plt
import numpy as np
from enum import Enum

import signal



f_s = 360    # Sample frequency in Hz
f_c = 50     # Notch frequency in Hz

omega_c = 2 * np.pi * f_c       # Notch angular frequency
omega_c_d = omega_c / f_s    # Normalized notch frequency (digital)


def LowpassFilter(fs, fc, Q=1 / np.sqrt(2.0)):
    """ 低通滤波器(Low Pass Filter)
    LPF: H(s) = 1 / (s^2 + s/Q + 1)

    b0 = (1 - cos(w0))/2;
    b1 = 1 - cos(w0);
    b2 = (1 - cos(w0))/2;
    a0 = 1 + alpha;
    a1 = -2*cos(w0);
    a2 = 1 - alpha;
    """
    w0 = 2.0 * np.pi * fc / fs
    cos_w0 = np.cos(w0)
    sin_w0 = np.sin(w0)
    alpha = sin_w0 / (2.0 * Q)

    b0 = (1.0 - cos_w0) / 2.0
    b1 = 1.0 - cos_w0
    b2 = (1.0 - cos_w0) / 2.0
    a0 = 1.0 + alpha
    a1 = -2.0 * cos_w0
    a2 = 1.0 - alpha
    
    numerator_B = np.array([b0, b1, b2], dtype=np.float32)
    denominator_A = np.array([a0, a1, a2], dtype=np.float32)
    return numerator_B, denominator_A
def NotchFilter(fs, fc, Q=1 / np.sqrt(2)):
    """Notch滤波器
    notch: H(s) = (s^2 + 1) / (s^2 + s/Q + 1)$
        b0 =   1
        b1 =  -2*cos(w0)
        b2 =   1
        a0 =   1 + alpha
        a1 =  -2*cos(w0)
        a2 =   1 - alpha
    """

    w0 = 2.0 * np.pi * fc / fs
    cos_w0 = np.cos(w0)  # cos(w0)
    sin_w0 = np.sin(w0)
    alpha = sin_w0 / (2.0 * Q)

    b0 = 1.0
    b1 = -2.0 * cos_w0
    b2 = 1.0
    a0 = 1.0 + alpha
    a1 = -2.0 * cos_w0
    a2 = 1.0 - alpha
    numerator_B = np.array([b0, b1, b2], dtype=np.float32)
    denominator_A = np.array([a0, a1, a2], dtype=np.float32)
    return numerator_B, denominator_A

def AllpassFilter(fs, fc, Q=1 / np.sqrt(2)):
    """全通 All Pass Filter
    APF: H(s) = (s^2 - s/Q + 1) / (s^2 + s/Q + 1)$
        b0 =   1 - alpha
        b1 =  -2*cos(w0)
        b2 =   1 + alpha
        a0 =   1 + alpha
        a1 =  -2*cos(w0)
        a2 =   1 - alpha
    """

    w0 = 2.0 * np.pi * fc / fs
    cos_w0 = np.cos(w0)  # cos(w0)
    sin_w0 = np.sin(w0)
    alpha = sin_w0 / (2.0 * Q)

    b0 = 1.0 - alpha
    b1 = -2.0 * cos_w0
    b2 = 1.0 + alpha
    a0 = 1.0 + alpha
    a1 = -2.0 * cos_w0
    a2 = 1.0 - alpha
    numerator_B = np.array([b0, b1, b2], dtype=np.float32)
    denominator_A = np.array([a0, a1, a2], dtype=np.float32)
    return numerator_B, denominator_A

def LowshelfFilter(fs, fc, dBgain, Q=1 / np.sqrt(2)):
    """低切滤波器
    lowShelf: H(s) = A * (s^2 + (sqrt(A)/Q)*s + A)/(A*s^2 + (sqrt(A)/Q)*s + 1)
        b0 =    A*( (A+1) - (A-1)*cos(w0) + 2*sqrt(A)*alpha )
        b1 =  2*A*( (A-1) - (A+1)*cos(w0)                   )
        b2 =    A*( (A+1) - (A-1)*cos(w0) - 2*sqrt(A)*alpha )
        a0 =        (A+1) + (A-1)*cos(w0) + 2*sqrt(A)*alpha
        a1 =   -2*( (A-1) + (A+1)*cos(w0)                   )
        a2 =        (A+1) + (A-1)*cos(w0) - 2*sqrt(A)*alpha
    """

    w0 = 2.0 * np.pi * fc / fs
    cos_w0 = np.cos(w0)  # cos(w0)
    sin_w0 = np.sin(w0)
    alpha = sin_w0 / (2.0 * Q)

    dBgain = round(float(dBgain), 3)
    A = 10.0 ** (dBgain / 40.0)

    b0 = A * ((A + 1) - (A - 1) * cos_w0 + 2 * np.sqrt(A) * alpha)
    b1 = 2 * A * ((A - 1) - (A + 1) * cos_w0)
    b2 = A * ((A + 1) - (A - 1) * cos_w0 - 2 * np.sqrt(A) * alpha)
    a0 = (A + 1) + (A - 1) * cos_w0 + 2 * np.sqrt(A) * alpha
    a1 = -2 * ((A - 1) + (A + 1) * cos_w0)
    a2 = (A + 1) + (A - 1) * cos_w0 - 2 * np.sqrt(A) * alpha
    numerator_B = np.array([b0, b1, b2], dtype=np.float32)
    denominator_A = np.array([a0, a1, a2], dtype=np.float32)
    return numerator_B, denominator_A



class BiquadType(Enum):
    LOWPASS = 0
    HIGHPASS = 1
    BANDPASS = 2  # 可选扩展
    NOTCH = 3     # 可选扩展
    PEAKING = 4   # 可选扩展
    LOWSHELF = 5  # 可选扩展
    HIGHSHELF = 6 # 可选扩展
    ALLPASS = 7   # 全通滤波器
def BiquadFilter(fs, fc, Q=0.707, filter_type='lowpass'):
    """双二阶滤波器 Biquad Filter (修正版)"""
    
    # 数字角频率
    w0 = 2.0 * np.pi * fc / fs
    
    # 关键修正：正确的预翘曲公式
    # K = tan(w0/2) 而不是 tan(π * fc/fs)
    K = np.tan(w0 / 2.0)
    
    # 归一化因子
    norm = 1.0 / (1.0 + K / Q + K * K)
    
    # 根据滤波器类型计算系数
    if filter_type.lower() == 'lowpass':
        b0 = K * K * norm
        b1 = 2.0 * b0
        b2 = b0
        a0 = 1.0
        a1 = 2.0 * (K * K - 1.0) * norm
        a2 = (1.0 - K / Q + K * K) * norm
        
    elif filter_type.lower() == 'highpass':
        b0 = 1.0 * norm
        b1 = -2.0 * b0
        b2 = b0
        a0 = 1.0
        a1 = 2.0 * (K * K - 1.0) * norm
        a2 = (1.0 - K / Q + K * K) * norm
        
    elif filter_type.lower() == 'allpass':
        # 全通滤波器使用不同设计方法
        alpha = np.sin(w0) / (2.0 * Q)
        b0 = 1.0 - alpha
        b1 = -2.0 * np.cos(w0)
        b2 = 1.0 + alpha
        a0 = 1.0 + alpha
        a1 = -2.0 * np.cos(w0)
        a2 = 1.0 - alpha
    else:
        raise ValueError(f"不支持的滤波器类型: {filter_type}")
    
    # 归一化到a0=1
    b0, b1, b2 = b0/a0, b1/a0, b2/a0
    a0, a1, a2 = 1.0, a1/a0, a2/a0
    
    numerator_B = np.array([b0, b1, b2], dtype=np.float32)
    denominator_A = np.array([a0, a1, a2], dtype=np.float32)
    
    return numerator_B, denominator_A

# h_0 = 2 - 2 * cos(omega_c_d)
# b = np.array((1, -2 * cos(omega_c_d), 1))   # Calculate coefficients
# b /= h_0                                    # Normalize
# a = 1

test_freqs = [50, 100, 200, 300]
for f in test_freqs:
    b, a = BiquadFilter(1000, f, 0.707, 'lowpass')
    print("a =", a)                      # Print the coefficients
    print("b =", b)

    w, h = freqz(b, a, fs=1000)
    magnitude = 20 * np.log10(np.abs(h))
    # 在f处查找增益
    idx = np.argmin(np.abs(w*1000/(2*np.pi) - f))
    print(f"设计{f}Hz处增益: {magnitude[idx]:.2f} dB")

# b, a = LowshelfFilter(fs = 1000, fc = 100, dBgain = 10,Q = 0.707)
b, a = BiquadFilter(fs=1000, fc=200, Q=0.707, filter_type='lowpass')
print("a =", a)                      # Print the coefficients
print("b =", b)



w, h = freqz(b, a)                   # Calculate the frequency response
w *= f_s / (2 * np.pi)                  # Convert from rad/sample to Hz

plt.subplot(2, 1, 1)                 # Plot the amplitude response
plt.suptitle('Bode Plot')            
plt.plot(w, 20 * np.log10(abs(h)))   # Convert to dB
plt.ylabel('Magnitude [dB]')
plt.xlim(0, f_s / 2)
plt.ylim(-60, 20)
plt.axvline(f_c, color='red')

plt.subplot(2, 1, 2)                 # Plot the phase response
plt.plot(w, 180 * np.angle(h) / np.pi)  # Convert argument to degrees
plt.xlabel('Frequency [Hz]')
plt.ylabel('Phase [°]')
plt.xlim(0, f_s / 2)
plt.ylim(-90, 135)
plt.yticks([-90, -45, 0, 45, 90, 135])
plt.axvline(f_c, color='red')
plt.show()