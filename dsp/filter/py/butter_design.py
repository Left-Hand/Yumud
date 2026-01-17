import numpy as np
from scipy.signal import butter, freqz
from scipy import signal
import matplotlib.pyplot as plt

# https://zhuanlan.zhihu.com/p/24342046101

def plot_filter_response(b, a, title, subplot, cutoff=None, lowcut=None, highcut=None):
    w, h = freqz(b, a, worN=8000)
    subplot.plot(0.5 * fs * w / np.pi, np.abs(h), 'b')
    if cutoff:
        subplot.axvline(cutoff, color='r', ls='--')
    if lowcut:
        subplot.axvline(lowcut, color='r', ls='--')
    if highcut:
        subplot.axvline(highcut, color='r', ls='--')
    subplot.set_title(title)
    subplot.set_xlabel('Frequency (Hz)')
    subplot.set_ylabel('Gain')
    subplot.grid(True)


fs = 32000.0  # Sample rate (Hz)
cutoff = 1000.0  # Desired cutoff frequency (Hz)

fig, axs = plt.subplots(1, 4, figsize=(20, 5))

##################
# Lowpass filter
##################
# method 1:
b, a = butter(2, cutoff / (0.5 * fs), btype='low', output="ba") or (np.array([]), np.array([]))
print(b,a)
# method 2: (equals method 1)
sos = butter(2, cutoff / (0.5 * fs), btype='low', output="sos")
b, a = signal.sos2tf(sos)

plot_filter_response(b, a, 'Lowpass filter', axs[0], cutoff=cutoff)

##################
# Highpass filter
##################
b, a = butter(2, cutoff / (0.5 * fs), btype='high')
plot_filter_response(b, a, 'Highpass filter', axs[1], cutoff=cutoff)

##################
# Bandpass filter
##################
lowcut = 500.0
highcut = 1500.0
b, a = butter(2, [lowcut / (0.5 * fs), highcut / (0.5 * fs)], btype='band')
plot_filter_response(b, a, 'Bandpass filter', axs[2], lowcut=lowcut, highcut=highcut)

##################
# Bandstop filter
##################
b, a = butter(2, [lowcut / (0.5 * fs), highcut / (0.5 * fs)], btype='bandstop')

plot_filter_response(b, a, 'Bandstop filter', axs[3], lowcut=lowcut, highcut=highcut)

plt.tight_layout()
plt.show()