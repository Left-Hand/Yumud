import numpy as np
import matplotlib.pyplot as plt


class ModifiedL1AdaptiveController:
    def __init__(self, param_Ts, param_tau):
        self.param_Ts = param_Ts  # 采样时间
        self.param_tau = param_tau  # 滤波器时间常数
        self.u_prev = 0  # 初始化滤波器前一次的控制输入
        self.x_hat_in = 0

        # 在线辨识的初始参数
        self.m_hat = 1.0
        self.b_hat = 1.0
        self.k_hat = 1.0

        # PID控制器参数
        self.kp = 10.0
        self.ki = 100.0
        self.kd = 10.0
        self.integral = 0
        self.prev_error = 0

    def online_system_identification(self, theta, theta_dot, theta_ddot, F):
        # 简单的递推最小二乘法进行在线辨识
        # 注意：这是一个简化的实现，可能需要更复杂的算法来获得更好的结果
        phi = np.array([theta_ddot, theta_dot, theta])
        y = F

        # 更新参数估计
        self.m_hat += 0.01 * (y - self.m_hat * phi[0] - self.b_hat * phi[1] - self.k_hat * phi[2]) * phi[0]
        self.b_hat += 0.01 * (y - self.m_hat * phi[0] - self.b_hat * phi[1] - self.k_hat * phi[2]) * phi[1]
        self.k_hat += 0.01 * (y - self.m_hat * phi[0] - self.b_hat * phi[1] - self.k_hat * phi[2]) * phi[2]

    def predict_core(self, theta, theta_dot, F):
        # 使用辨识得到的参数预测下一步的系统状态
        theta_ddot = (F - self.b_hat * theta_dot - self.k_hat * theta) / self.m_hat
        theta_next = theta + theta_dot * self.param_Ts + 0.5 * theta_ddot * self.param_Ts ** 2
        theta_dot_next = theta_dot + theta_ddot * self.param_Ts
        return theta_next, theta_dot_next

    def pid_control(self, error):
        self.integral += error * self.param_Ts
        derivative = (error - self.prev_error) / self.param_Ts
        u = self.kp * error + self.ki * self.integral + self.kd * derivative
        self.prev_error = error
        return u

    def control_core(self, desired_angle, actual_angle, actual_velocity):
        error = desired_angle - actual_angle
        u = self.pid_control(error)

        # 应用滤波器
        u_f = (self.param_Ts / self.param_tau) * u + (1 - self.param_Ts / self.param_tau) * self.u_prev
        self.u_prev = u_f

        return u_f


# 单机械臂模型
class SingleLinkArm:
    def __init__(self, length, dt):
        self.length = length
        self.angle = 0
        self.angular_velocity = 0
        self.dt = dt

    def update(self, torque):
        # 这里假设机械臂的转动惯量和阻尼系数为1
        inertia = 1
        damping = 1
        angular_acceleration = (torque - damping * self.angular_velocity) / inertia
        self.angular_velocity += angular_acceleration * self.dt
        self.angle += self.angular_velocity * self.dt
        return angular_acceleration

    def get_position(self):
        x = self.length * np.cos(self.angle)
        y = self.length * np.sin(self.angle)
        return x, y


# 模拟参数
dt = 0.01
length = 1.0
time = np.arange(0, 50, dt)

# 正弦轨迹参数
amplitude = 0.5
frequency = 0.2
desired_angle = amplitude * np.sin(2 * np.pi * frequency * time)

# 初始化控制器和机械臂
controller = ModifiedL1AdaptiveController(param_Ts=dt, param_tau=0.008)
arm = SingleLinkArm(length, dt)

# 记录机械臂的轨迹
actual_angle = []
error_angle = []
m_hat_values = []
b_hat_values = []
k_hat_values = []

for t in time:
    error = desired_angle[int(t / dt)] - arm.angle
    error_angle.append(error)
    actual_angle.append(arm.angle)

    torque = controller.control_core(desired_angle[int(t / dt)], arm.angle, arm.angular_velocity)
    angular_acceleration = arm.update(torque)

    # 进行在线辨识
    controller.online_system_identification(arm.angle, arm.angular_velocity, angular_acceleration, torque)

    # 记录辨识参数
    m_hat_values.append(controller.m_hat)
    b_hat_values.append(controller.b_hat)
    k_hat_values.append(controller.k_hat)

# 绘图
plt.figure(figsize=(12, 10))

plt.subplot(2, 1, 1)
plt.plot(time, desired_angle, label='Desired Angle (rad)')
plt.plot(time, actual_angle, label='Actual Angle (rad)')
plt.plot(time, error_angle, label='Error (rad)')
plt.xlabel('Time (s)')
plt.ylabel('Angle (rad)')
plt.title('Modified L1AC Control of Single Link Arm')
plt.legend()
plt.grid(True)

plt.subplot(2, 1, 2)
plt.plot(time, m_hat_values, label='m_hat')
plt.plot(time, b_hat_values, label='b_hat')
plt.plot(time, k_hat_values, label='k_hat')
plt.xlabel('Time (s)')
plt.ylabel('Parameter Value')
plt.title('Online System Identification')
plt.legend()
plt.grid(True)

plt.tight_layout()
plt.show()