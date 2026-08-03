import numpy as np
import matplotlib.pyplot as plt
import control as ctrl

# 定义 r
# r = 10.0  # rad/s
τ = 0.1

# 传递函数 H(s) = (2rs + r^2) / (s + r)^2
# 传递函数 H(s) = (2τs + 1) / (τs + 1)^2
# num = [2*r, r**2]    # 2r s + r^2
# zeta = 0.707
zeta = 1
num = [2*zeta *τ,1]    # 2r s + r^2
den = [τ ** 2, 2*zeta *τ, 1] # (s+r)^2 = s^2 + 2r s + r^2

sys = ctrl.TransferFunction(num, den)

# 生成伯德图
plt.figure(figsize=(10, 6))
mag, phase, omega = ctrl.bode(sys, dB=True, Hz=False, omega_limits=[0.1, 1000], omega_num=500, plot=False)

# 绘制幅频
plt.subplot(2, 1, 1)
plt.semilogx(omega, 20*np.log10(mag))
plt.grid(True, which='both')
plt.ylabel('Magnitude (dB)')
plt.title(f'Bode plot of H(s) = (2rs + r^2)/(s+r)^2, r={1/τ} rad/s')

# 绘制相频
plt.subplot(2, 1, 2)
plt.semilogx(omega, phase * 180/np.pi)
plt.grid(True, which='both')
plt.ylabel('Phase (deg)')
plt.xlabel('Frequency (rad/s)')
plt.tight_layout()
plt.show()