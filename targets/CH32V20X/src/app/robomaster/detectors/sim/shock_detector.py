import numpy as np
import matplotlib.pyplot as plt
from scipy import signal


# https://blog.csdn.net/u010634139/article/details/143801055



fs:float = 20000
fl:float = 70
fh:float = 180

dur:float = 10

# 生成测试信号
t = np.linspace(0, dur, 1000, endpoint=False)
x = np.sin(2*np.pi*10*t) + 0.7*np.sin(2*np.pi*80*t) + 20*np.sin(2*np.pi*500*t)
# x = np.sin(2*np.pi*10*t) + 20*np.sin(2*np.pi*500*t)

b, a  =   signal.butter( 2, [ fl/fs , fh/fs],  'bandpass' )    #配置滤波器 8 表示滤波器的阶数
filtered_bandpass  =   signal.filtfilt(b, a, x)   #data为要过滤的信号

# 绘制原始信号和滤波后的信号的时域图和频域图
fig, axs = plt.subplots(2, 2, figsize=(10, 10))
fig.suptitle('Signal Filtering')

axs[0, 0].plot(t, x)
axs[0, 0].set_title('bandpass Filtered Signal (Time Domain)')
axs[0, 0].set_xlabel('Time')
axs[0, 0].set_ylabel('Amplitude')

axs[0, 1].magnitude_spectrum(x, Fs=fs)
axs[0, 1].set_title('bandpass Filtered Signal (Frequency Domain)')
axs[0, 1].set_xlabel('Frequency')
axs[0, 1].set_ylabel('Magnitude')

axs[1, 0].plot(t, filtered_bandpass)
axs[1, 0].set_title('bandpass Filtered Signal (Time Domain)')
axs[1, 0].set_xlabel('Time')
axs[1, 0].set_ylabel('Amplitude')

axs[1, 1].magnitude_spectrum(filtered_bandpass, Fs=fs)
axs[1, 1].set_title('bandpass Filtered Signal (Frequency Domain)')
axs[1, 1].set_xlabel('Frequency')
axs[1, 1].set_ylabel('Magnitude')

plt.tight_layout()
plt.show()
