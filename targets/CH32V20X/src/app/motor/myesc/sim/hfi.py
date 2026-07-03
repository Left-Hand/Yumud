import numpy as np
import matplotlib.pyplot as plt

# https://dgjsxb.ces-transaction.com/fileup/HTML/2021-4-801.htm

TAU = 2 * np.pi
DURATION = 0.2
FS = 36000              # 采样率 (Hz)
FH = FS / 32
FSPIN = 12
NOISE_STDVAR = 0.20

N = DURATION * FS
Lq = 66e-6
Ld = 40e-6

L0 = (Ld + Lq) / 2
L1 = (Ld - Lq) / 2

K = 1/(L0*L0 - L1 * L1)

# 1. 设置随机种子，让每次运行结果一致（方便对比）
np.random.seed(42)

# 2. 生成采样时间和纯净信号


t = np.linspace(0, DURATION, FS)  # 采样1秒钟

hfi_sine_modu = np.sin(TAU * FH * t)
hfi_k = K / (TAU * FH)
hfi_factor = hfi_k * hfi_sine_modu

# print(L0, L1, K)


pure_d = hfi_factor * (L0 - L1 * np.cos(TAU * FSPIN * t) )
pure_q = hfi_factor * (- L1 * np.sin(TAU * FSPIN * t) )

def create_noise(noise_amplitude):
        # 3. 添加白噪声 (信噪比约 20dB)
    noise = noise_amplitude * np.random.randn(len(t))  # 高斯白噪声
    return noise


noise_d = pure_d + create_noise(NOISE_STDVAR)
noise_q = pure_q + create_noise(NOISE_STDVAR)
frac = noise_q / noise_d
frac = np.clip(frac, -1, 1)


# 4. 绘图
plt.figure(figsize=(12, 5))

# plt.plot(t, pure_d, 'b--', linewidth=1.5, alpha=0.7, label = "pure_d")
plt.plot(t, noise_d, 'r-', linewidth=0.8, alpha=0.9, label='noise_d')

# plt.plot(t, pure_q, 'y--', linewidth=1.5, alpha=0.7, label = "pure_q")
plt.plot(t, noise_q, 'g-', linewidth=0.8, alpha=0.9, label='noise_q')
plt.plot(t, frac, 'y-', linewidth=0.8, alpha=0.9, label='frac')

plt.legend()
plt.grid(True, alpha=0.3)
plt.tight_layout()
plt.show()