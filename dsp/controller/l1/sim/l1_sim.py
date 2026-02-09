import numpy as np
import matplotlib.pyplot as plt
from dataclasses import dataclass

@dataclass
class State:
    """存储控制器的动态状态变量"""
    state_u_prev: float = 0.0      # 上一次的控制输入
    state_x_hat_in: float = 0.0    # 内部预测状态


class Precomputed:
    """存储预计算的参数并实现控制器操作"""
    
    def __init__(self, param_am: float, param_b: float, param_Ts: float, param_tau: float):
        """
        :param param_am: 理想模型的动态参数，决定输出增益
        :param param_b: 系统参数b，影响控制输入对系统输出的作用
        :param param_Ts: 基本可以设定成时间步长
        :param param_tau: 学习率
        """
        self.param_am: float = param_am  # 理想模型的动态参数
        self.param_b: float = param_b   # 系统参数b，影响控制输入对系统输出的作用
        self.param_Ts: float = param_Ts  # 自适应采样时间

        self.kr: float = self.param_am / self.param_b  # 计算理想控制器的增益
        self.Phi_Ts: float = (1 - np.exp(-self.param_am * self.param_Ts)) / self.param_am  # 计算状态更新公式
        self.exp_factor: float = np.exp(-self.param_am * self.param_Ts)
        self.k_u: float = (self.param_Ts / param_tau)
        self.k_u_prev: float = 1.0 - self.k_u
        self.k_inv_muy: float = 1/(self.Phi_Ts * self.param_b)

    def predict_core(self, x_hat_prev: float, u: float, sigma_hat: float) -> float:
        # 预测器的动态方程，用于估计下一步的系统状态
        # 状态变化率
        dx_hat: float = -self.param_am * x_hat_prev + self.param_b * (u + sigma_hat)
        # 更新预测状态
        x_hat: float = x_hat_prev + dx_hat * self.param_Ts
        return x_hat

    def control_core(self, state: State, desired_x1: float, now_x1: float) -> tuple[float, State]:
        x_tilde: float = state.state_x_hat_in - now_x1  # 计算状态误差

        sigma_hat: float = self.estimator(x_tilde)  # 估计系统不确定性

        # 控制器设计，基于L1自适应控制原理
        # 控制输入由理想控制器输出减去系统不确定性估计值
        # 控制策略：这里随便给了一个u = self.kr * EXP_POS，这里必须根据不同被控对象进行调整
        u: float = self.kr * desired_x1 - sigma_hat
        # 应用滤波器：对u进行一个光顺
        u_f: float = self.k_u * u + self.k_u_prev * state.state_u_prev
        # 更新上一次控制输入
        
        # 创建新的状态对象
        new_state = State(
            state_u_prev=u_f,
            state_x_hat_in=self.predict_core(state.state_x_hat_in, u_f, sigma_hat)
        )

        return u_f, new_state

    def estimator(self, x_tilde: float) -> float:
        # 不确定性估计器，用于估计系统的不确定性
        # 通过状态误差计算出不确定性估计值
        # 计算状态误差的加权
        muy: float = self.exp_factor * x_tilde
        # 计算不确定性估计值
        sigma_hat: float = -muy * self.k_inv_muy
        return sigma_hat


# 使用示例
class L1AdaptiveController:
    def __init__(self, param_am: float, param_b: float, param_Ts: float, param_tau: float):
        self.precomputed = Precomputed(param_am, param_b, param_Ts, param_tau)
        self.state = State()

    def control_core(self, desired_x1: float, now_x1: float) -> float:
        u_f, self.state = self.precomputed.control_core(self.state, desired_x1, now_x1)
        return u_f
# 参数设置
dt = 0.001  # 时间步长

# 初始化控制器
controller = L1AdaptiveController(param_am =19,
                                  param_b =1.1,
                                  param_Ts = dt,
                                  param_tau =0.001)


# 

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
        self.angular_velocity += (torque - damping * self.angular_velocity) / inertia * self.dt
        self.angle += self.angular_velocity * self.dt

    def get_position(self):
        x = self.length * np.cos(self.angle)
        y = self.length * np.sin(self.angle)
        return x, y

# 模拟参数
length = 1.0  # 机械臂长度

time = np.arange(0, 50, dt)  # 模拟时间

# 正弦轨迹参数
amplitude = 0.55  # 正弦轨迹振幅
frequency = 0.2  # 正弦轨迹频率
desired_angle = amplitude * np.sin(2 * np.pi * frequency * time)

arm = SingleLinkArm(length, dt)

# 记录机械臂的轨迹
actual_angle = []
errpr_angle = []
for t in time:
    error = desired_angle[int(t / dt)] - arm.angle
    errpr_angle.append(error)
    actual_angle.append(arm.angle)

    torque = controller.control_core(desired_angle[int(t / dt)], arm.angle)
    arm.update(torque)


# 画图
plt.figure()
plt.plot(time, desired_angle, label='Desired Angle (rad)')
plt.plot(time, actual_angle, label='Actual Angle (rad)')
plt.plot(time, errpr_angle, label='error (rad)')


plt.xlabel('Time (s)')
plt.ylabel('Angle (rad)')
plt.title('L1AC Control of Single Link Arm')
plt.legend()
plt.grid(True)
plt.show()