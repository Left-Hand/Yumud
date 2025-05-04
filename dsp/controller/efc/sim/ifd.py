import numpy as np
import matplotlib.pyplot as plt
import control as ctl

#IFD 系统为AD系统和IFC系统的串联
# AD系统为高通RC滤波器
# ICF系统为低通RC滤波器链之和

# 定义参数
K_IFD = 1.0      # 增益
T_IFD = 0.1      # 时间常数
n = 3            # ICF的阶数 (n-1个低通滤波器叠加)
T = 0.0001

# 创建传递函数 AD(s) = K_IFD * (T_IFD*s) / (1 + T_IFD*s)
s = ctl.TransferFunction(ctl.tf('s'))
AD = K_IFD * (T_IFD * s) / (1 + T_IFD * s)

# 创建传递函数 ICF(s) = Σ [1/(1 + T_IFD*s)]^i
ICF = 0
for i in range(1, n):  # i从1到n-1
    ICF += (1 / (1 + T_IFD * s)) ** i

# 总传递函数 IFD(s) = AD(s) * ICF(s)
IFD = AD * ICF
IFC_INV = 1 / ICF

IFD_Z = ctl.c2d(AD * ICF, T)
ICF_INV = 1 / ICF
EFLO = 1 / ICF
EFPI = 1 / (1 - ICF)
EFC = EFLO * EFPI
EFC_Z = ctl.c2d(EFC, T, 'tustin')
# ICF_INV_Z = ctl.c2d(ICF_INV, T)

# 打印传递函数
print("AD(s):", AD)
print("ICF(s):", ICF)
print("IFD(s):", IFD)
print("IFD(z):", IFD_Z)
print("IFC_INV(s):", IFC_INV)
# print("IFC(s):", IFD)
# print("IFC_INV(z):", IFC_INV_Z)
print("EFLO(s):", EFLO)
print("EFPI(s):", EFPI)
print("EFC(s):", EFC)
print("EFC(z):", EFC_Z)

# # 绘制伯德图
# plt.figure(figsize=(12, 6))
# ctl.bode_plot(IFD, dB=True, Hz=False, margins=True)
# ctl.bode_plot(IFD_Z, dB=True, Hz=False, margins=True)
# plt.suptitle('Bode Plot of IFD(s) = AD(s) * ICF(s)')
# plt.tight_layout()
# plt.show()

# # 绘制阶跃响应
# plt.figure(figsize=(8, 4))
# t, y = ctl.step_response(IFD)
# plt.plot(t, y)
# plt.title('Step Response of IFD(s)')
# plt.xlabel('Time [s]')
# plt.ylabel('Amplitude')
# plt.grid()
# plt.show()

# # 绘制阶跃响应
# plt.figure(figsize=(8, 4))
# t, y = ctl.step_response(IFC_INV)
# plt.plot(t, y)
# plt.title('Step Response of IFC_INV(s)')
# plt.xlabel('Time [s]')
# plt.ylabel('Amplitude')
# plt.grid()
# plt.show()