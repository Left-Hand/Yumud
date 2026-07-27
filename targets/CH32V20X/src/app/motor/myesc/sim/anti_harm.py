import numpy as np
import matplotlib.pyplot as plt

# ========== 仿真参数 ==========
dt = 1/ 25000                     # 采样步长 (s)
T_total = 1.0                   # 总时间
t = np.arange(0, T_total, dt)
N = len(t)

# ---------- 运动参数（梯形速度） ----------
v_max = 12.0                     # 最大速度 (rad/s)
a_max = 100.0                     # 加速度 (rad/s²)
t_acc = v_max / a_max           # 加速时间 = 1s
t_const = T_total - 2*t_acc     # 匀速时间 = 4s (若为正)

# ---------- 扰动参数 ----------
n = 84
b = 0.002
# phi = np.pi / 6                 # 任意相位
phi = np.pi/2                 # 任意相位
b1_true = b * np.cos(phi)
b2_true = b * np.sin(phi)

# ---------- 观测器增益 ----------
l1 = 1115.0                       # 自适应观测器位置增益
l2 = 7.3                        # 参数增益

# LESO 带宽 (rad/s) —— 高于运动频率即可
wo = 800.0
beta1 = 2.0 * wo
beta2 = wo**2

# ========== 初始化状态 ==========
theta_true = 0.0
# 自适应观测器
hat_theta = 0.0
hat_b1 = 0.0
hat_b2 = 0.0
# LESO (用于滤波后信号)
z1_f = 0.0
z2_f = 0.0
# LESO (用于原始信号，对比)
z1_raw = 0.0
z2_raw = 0.0

# 存储变量
theta_true_vec = np.zeros(N)
theta_meas_vec = np.zeros(N)
theta_hat_vec  = np.zeros(N)
b1_hat_vec     = np.zeros(N)
b2_hat_vec     = np.zeros(N)
z1_f_vec       = np.zeros(N)   # LESO位置估计 (滤波后)
z2_f_vec       = np.zeros(N)   # LESO速度估计 (滤波后)
z2_raw_vec     = np.zeros(N)   # LESO速度估计 (原始)

# ========== 梯形运动函数 ==========
def get_trapezoid(t, v_max, a_max, T_total):
    t_acc = v_max / a_max
    if T_total < 2*t_acc:
        t_acc = T_total / 2
        v_max = a_max * t_acc
    t_const = T_total - 2*t_acc
    if t < 0:
        return 0.0, 0.0
    elif t < t_acc:
        v = a_max * t
        theta = 0.5 * a_max * t**2
    elif t < t_acc + t_const:
        v = v_max
        theta = 0.5 * a_max * t_acc**2 + v_max * (t - t_acc)
    elif t < T_total:
        t_dec = t - (t_acc + t_const)
        v = v_max - a_max * t_dec
        theta = (0.5 * a_max * t_acc**2 + v_max * t_const +
                 v_max * t_dec - 0.5 * a_max * t_dec**2)
    else:
        v = 0.0
        theta = 0.5 * a_max * t_acc**2 + v_max * t_const + 0.5 * v_max**2 / a_max
    return theta, v

def lerp(a, b, r):
    return a + (b - a) * r

# ========== 主循环 ==========
for i, ti in enumerate(t):
    # ----- 真实轨迹 -----
    theta_true, omega_true = get_trapezoid(ti, v_max, a_max, T_total)

    # ----- 测量值（含谐波扰动）-----
    theta_meas = theta_true + b1_true * np.sin(n * theta_true) + b2_true * np.cos(n * theta_true)

    # ===== 自适应观测器 (已知真实速度作为前馈) =====
    s = np.sin(n * hat_theta)
    c = np.cos(n * hat_theta)
    harm = hat_b2 * c
    # y_hat = hat_theta + 
    e = theta_meas - hat_theta - harm
    # hat_theta += (z2_f + l1 * e) * dt
    # hat_theta += (z2_f + l1 * e) * dt
    hat_theta += (z2_f + l1 * e) * dt
    l2dt = l2 * dt
    # hat_b1 = lerp(hat_b1, hat_b1 + e * (s), 0.0025)
    hat_b2 = lerp(hat_b2, hat_b2 + e * (c), 0.0025)

    # ===== LESO 1: 输入为滤波后的 hat_theta =====
    e1 = z1_f - hat_theta
    z1_f += (z2_f - beta1 * e1) * dt
    z2_f += (- beta2 * e1) * dt     # u=0

    # ===== LESO 2: 输入为原始 theta_meas (对比) =====
    e2 = z1_raw - theta_meas + harm
    z1_raw += (z2_raw - beta1 * e2) * dt
    z2_raw += (- beta2 * e2) * dt

    # ----- 存储 -----
    theta_true_vec[i] = theta_true
    theta_meas_vec[i] = theta_meas
    theta_hat_vec[i]  = hat_theta
    b1_hat_vec[i]     = hat_b1
    b2_hat_vec[i]     = hat_b2
    z1_f_vec[i]       = z1_f
    z2_f_vec[i]       = z2_f
    z2_raw_vec[i]     = z2_raw

# ========== 绘图 ==========
plt.figure(figsize=(15, 12))

# 1. 位置：真实，测量，自适应滤波输出
plt.subplot(3, 2, 1)
plt.plot(t, theta_true_vec, 'k-', linewidth=2, label='True θ')
plt.plot(t, theta_meas_vec, 'r--', alpha=0.4, label='Measured θ')
plt.plot(t, theta_hat_vec, 'b-', linewidth=1.5, label='Adaptive filtered θ')
plt.xlabel('Time (s)'); plt.ylabel('θ (rad)')
plt.legend(); plt.grid(True); plt.title('Position: True vs Measured vs Filtered')

# 2. 局部放大（显示滤波效果）
plt.subplot(3, 2, 2)
plt.plot(t, theta_true_vec, 'k-', linewidth=2, label='True')
plt.plot(t, theta_meas_vec, 'r--', alpha=0.4, label='Measured')
plt.plot(t, theta_hat_vec, 'b-', linewidth=1.5, label='Filtered')
plt.xlim(2.0, 2.5)  # 匀速段局部
plt.xlabel('Time (s)'); plt.ylabel('θ (rad)')
plt.legend(); plt.grid(True); plt.title('Zoom-in (2.0~2.5s)')

# 3. 速度估计：真实速度 vs LESO(滤波后) vs LESO(原始)
plt.subplot(3, 2, 3)
# 计算真实速度（已存 omega_true? 我们没存，重新计算或用差分）
omega_true_vec = np.zeros(N)
for i in range(1, N):
    omega_true_vec[i] = (theta_true_vec[i] - theta_true_vec[i-1]) / dt
omega_true_vec[0] = omega_true_vec[1]
plt.plot(t, omega_true_vec, 'k-', linewidth=2, label='True speed')
plt.plot(t, z2_f_vec, 'b-', linewidth=1.5, label='LESO (filtered input)')
plt.plot(t, z2_raw_vec, 'r--', alpha=0.5, label='LESO (raw input)')
plt.xlabel('Time (s)'); plt.ylabel('Speed (rad/s)')
plt.legend(); plt.grid(True); plt.title('Speed Estimation Comparison')

# 4. 速度误差（滤波后输入 vs 原始输入）
plt.subplot(3, 2, 4)
err_f = z2_f_vec - omega_true_vec
err_raw = z2_raw_vec - omega_true_vec
plt.plot(t, err_f, 'b-', label='Error (filtered input)')
plt.plot(t, err_raw, 'r--', alpha=0.5, label='Error (raw input)')
plt.xlabel('Time (s)'); plt.ylabel('Speed error (rad/s)')
plt.legend(); plt.grid(True); plt.title('Speed Estimation Errors')

# 5. 自适应参数估计 b1, b2
plt.subplot(3, 2, 5)
plt.plot(t, b1_true * np.ones_like(t), 'k--', label='True b1')
plt.plot(t, b1_hat_vec, 'b-', label='Estimated b1')
plt.plot(t, b2_true * np.ones_like(t), 'k--', label='True b2')
plt.plot(t, b2_hat_vec, 'g-', label='Estimated b2')
plt.xlabel('Time (s)'); plt.ylabel('b1, b2')
plt.legend(); plt.grid(True); plt.title('Disturbance Parameter Estimation')

# 6. 重构的扰动幅值和相位
plt.subplot(3, 2, 6)
mag_est = np.sqrt(b1_hat_vec**2 + b2_hat_vec**2)
phi_est = np.arctan2(b2_hat_vec, b1_hat_vec)
plt.plot(t, b * np.ones_like(t), 'k--', label='True magnitude')
plt.plot(t, mag_est, 'b-', label='Estimated magnitude')
plt.plot(t, phi * np.ones_like(t), 'r--', label='True phase')
plt.plot(t, phi_est, 'g-', alpha=0.6, label='Estimated phase')
plt.xlabel('Time (s)')
plt.legend(); plt.grid(True); plt.title('Reconstructed Amplitude & Phase')

plt.tight_layout()
plt.show()

# 打印稳态误差（最后 1 秒）
steady_idx = int(0.85 * N)
err_f_steady = np.mean(np.abs(z2_f_vec[steady_idx:] - omega_true_vec[steady_idx:]))
err_raw_steady = np.mean(np.abs(z2_raw_vec[steady_idx:] - omega_true_vec[steady_idx:]))
print(f"LESO(滤波后输入) 速度稳态误差均值: {err_f_steady:.4f} rad/s")
print(f"LESO(原始输入)   速度稳态误差均值: {err_raw_steady:.4f} rad/s")