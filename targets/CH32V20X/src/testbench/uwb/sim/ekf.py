import numpy as np
import matplotlib.pyplot as plt
from typing import Tuple, List, Callable, Optional, Dict, Protocol
from dataclasses import dataclass
from scipy.linalg import block_diag

@dataclass
class EKFConfig:
    """EKF配置参数"""
    dt: float
    d: float
    q_x: float
    q_y: float
    sigma_l1: float
    sigma_l2: float
    sigma_q: float

class ExtendedKalmanFilter:
    def __init__(self, config: EKFConfig):
        self.config = config
        
        # 状态向量: [x, y, vx, vy]
        self.state = np.zeros(4)
        self.cov = np.eye(4) * 0.01
        
        # 过程噪声协方差
        self.Q = self._compute_process_noise()
        
        # 观测噪声协方差
        self.R = np.diag([config.sigma_l1**2, config.sigma_l2**2, config.sigma_q**2])
        
        # 状态转移矩阵
        self.F = np.array([
            [1, 0, config.dt, 0],
            [0, 1, 0, config.dt],
            [0, 0, 1, 0],
            [0, 0, 0, 1]
        ])
    
    def _compute_process_noise(self) -> np.ndarray:
        """计算离散过程噪声协方差矩阵"""
        dt = self.config.dt
        qx, qy = self.config.q_x, self.config.q_y
        
        Q = np.array([
            [dt**3/3 * qx, 0, dt**2/2 * qx, 0],
            [0, dt**3/3 * qy, 0, dt**2/2 * qy],
            [dt**2/2 * qx, 0, dt * qx, 0],
            [0, dt**2/2 * qy, 0, dt * qy]
        ])
        return Q
    
    def _observation_model(self, state: np.ndarray) -> np.ndarray:
        """观测模型 h(x)"""
        x, y, _, _ = state
        d = self.config.d
        
        l1 = np.sqrt(x**2 + (y - d)**2)
        l2 = np.sqrt(x**2 + (y + d)**2)
        q = np.arctan2(y - d, x)
        
        return np.array([l1, l2, q])
    
    def _observation_jacobian(self, state: np.ndarray) -> np.ndarray:
        """观测雅可比矩阵 H = dh/dx"""
        x, y, _, _ = state
        d = self.config.d
        
        l1 = np.sqrt(x**2 + (y - d)**2)
        l2 = np.sqrt(x**2 + (y + d)**2)
        
        # 避免除零
        l1_safe = l1 if l1 > 1e-6 else 1e-6
        l2_safe = l2 if l2 > 1e-6 else 1e-6
        
        H = np.array([
            [x/l1_safe, (y-d)/l1_safe, 0, 0],
            [x/l2_safe, (y+d)/l2_safe, 0, 0],
            [-(y-d)/(l1_safe**2), x/(l1_safe**2), 0, 0]
        ])
        
        return H
    
    def predict(self) -> None:
        """预测步骤"""
        self.state = self.F @ self.state
        self.cov = self.F @ self.cov @ self.F.T + self.Q
    
    def update(self, l1: float, l2: float, q: float) -> Tuple[float, float, float, float]:
        """更新步骤
        
        Args:
            l1: 雷达A测量的距离
            l2: 雷达B测量的距离  
            q: 雷达A测量的角度
            
        Returns:
            更新后的状态 [x, y, vx, vy]
        """
        # 预测
        self.predict()
        
        # 观测向量
        z = np.array([l1, l2, q])
        
        # 计算观测雅可比
        H = self._observation_jacobian(self.state)
        
        # 预测观测
        z_pred = self._observation_model(self.state)
        
        # 创新协方差
        S = H @ self.cov @ H.T + self.R
        
        # 卡尔曼增益
        K = self.cov @ H.T @ np.linalg.inv(S)
        
        # 更新状态和协方差
        self.state = self.state + K @ (z - z_pred)
        self.cov = self.cov - K @ H @ self.cov
        
        return tuple(self.state)

class KinematicSolver:
    """正运动学解算器，只使用l1和q计算位置"""
    
    def __init__(self, config: EKFConfig):
        self.config = config
        # 为了保持相同的API，我们也维护一个状态向量
        self.state = np.zeros(4)  # [x, y, vx, vy]
    
    def update(self, l1: float, l2: float, q: float) -> Tuple[float, float, float, float]:
        """使用正运动学直接从l1和q解算位置
        
        Args:
            l1: 雷达A测量的距离
            l2: 雷达B测量的距离 (不使用)
            q: 雷达A测量的角度
            
        Returns:
            解算的位置和零速度 [x, y, 0, 0]
        """
        # 直接从观测值计算位置
        x = l1 * np.cos(q)
        y = self.config.d + l1 * np.sin(q)
        
        # 速度设为0（因为没有滤波，无法估计速度）
        vx, vy = 0.0, 0.0
        
        self.state = np.array([x, y, vx, vy])
        return tuple(self.state)

class TrajectoryGenerator:
    """轨迹生成器基类"""
    def __init__(self, config: EKFConfig):
        self.config = config
    
    def generate_state(self, t: float) -> np.ndarray:
        """生成在时间t的状态 [x, y, vx, vy]"""
        raise NotImplementedError

class CircularTrajectoryGenerator(TrajectoryGenerator):
    """围绕 (cos(t), sin(2t)) 移动的轨迹生成器"""
    
    def generate_state(self, t: float) -> np.ndarray:
        """生成在时间t的状态 [x, y, vx, vy]"""
        # 位置: (cos(t), sin(2t))
        x = 6 * np.cos(t)
        y = 4 * np.sin(2 * t) + 5
        
        # 速度: 对位置求导 (-sin(t), 2cos(2t))
        vx = -np.sin(t)
        vy = 2 * np.cos(2 * t)
        
        return np.array([x, y, vx, vy])

def generate_sample_data(
    config: EKFConfig,
    duration: float = 3.0,  # 改为3秒
    trajectory_generator: Optional[TrajectoryGenerator] = None
) -> Tuple[List[float], List[np.ndarray], List[np.ndarray]]:
    """生成样本数据
    
    Args:
        config: EKF配置
        duration: 仿真时长(秒)
        trajectory_generator: 轨迹生成器
        
    Returns:
        time_steps: 时间序列
        true_states: 真实状态序列
        measurements: 观测序列
    """
    if trajectory_generator is None:
        trajectory_generator = CircularTrajectoryGenerator(config)
    
    dt = config.dt
    n_steps = int(duration / dt)
    
    time_steps = [i * dt for i in range(n_steps)]
    true_states = []
    measurements = []
    
    # 使用轨迹生成器生成真实轨迹
    for t in time_steps:
        state = trajectory_generator.generate_state(t)
        true_states.append(state)
    
    # 生成带噪声的观测
    for state in true_states:
        x, y, _, _ = state
        
        # 真实观测
        l1_true = np.sqrt(x**2 + (y - config.d)**2)
        l2_true = np.sqrt(x**2 + (y + config.d)**2)
        q_true = np.arctan2(y - config.d, x)
        
        # 添加噪声
        l1_meas = l1_true + np.random.normal(0, config.sigma_l1)
        l2_meas = l2_true + np.random.normal(0, config.sigma_l2)
        q_meas = q_true + np.random.normal(0, config.sigma_q)
        
        measurements.append(np.array([l1_meas, l2_meas, q_meas]))
    
    return time_steps, true_states, measurements

def plot_results(
    time_steps: List[float],
    true_states: List[np.ndarray],
    measurements: List[np.ndarray],
    dict_estimated_states: Dict[str, List[np.ndarray]],
    config: EKFConfig
) -> None:
    """绘制结果图表"""
    fig, axes = plt.subplots(2, 2, figsize=(12, 10))
    
    # 提取数据
    true_x = [state[0] for state in true_states]
    true_y = [state[1] for state in true_states]

    # 从观测数据反算位置 (用于显示带噪观测位置)
    obs_x = []
    obs_y = []
    for meas in measurements:
        l1, l2, q = meas
        # 从观测值反算位置 (近似)
        x_obs = l1 * np.cos(q)
        y_obs = config.d + l1 * np.sin(q)
        obs_x.append(x_obs)
        obs_y.append(y_obs)
    
    # 定义颜色和线型
    colors = ['b', 'm', 'c', 'orange']
    linestyles = ['--', ':', '-.', '-']
    
    # 1. 轨迹图
    axes[0, 0].plot(true_x, true_y, 'g-', label='real', linewidth=2)
    for i, (title, estimated_states) in enumerate(dict_estimated_states.items()):
        est_x = [state[0] for state in estimated_states]
        est_y = [state[1] for state in estimated_states]
        color = colors[i % len(colors)]
        linestyle = linestyles[i % len(linestyles)]
        axes[0, 0].plot(est_x, est_y, color=color, linestyle=linestyle, label=title, linewidth=2)
    
    axes[0, 0].scatter(obs_x, obs_y, c='r', s=10, alpha=0.5, label='meas')
    axes[0, 0].scatter([0], [config.d], c='k', s=100, marker='^', label='A')
    axes[0, 0].scatter([0], [-config.d], c='k', s=100, marker='v', label='B')
    axes[0, 0].set_xlabel('x')
    axes[0, 0].set_ylabel('y')
    axes[0, 0].set_title('target trajectory')
    axes[0, 0].legend()
    axes[0, 0].grid(True)
    
    # 2. X坐标随时间变化
    axes[0, 1].plot(time_steps, true_x, 'g-', label='real_x', linewidth=2)
    for i, (title, estimated_states) in enumerate(dict_estimated_states.items()):
        est_x = [state[0] for state in estimated_states]
        color = colors[i % len(colors)]
        linestyle = linestyles[i % len(linestyles)]
        axes[0, 1].plot(time_steps, est_x, color=color, linestyle=linestyle, label=f'{title}_x', linewidth=2)
    
    axes[0, 1].scatter(time_steps, obs_x, c='r', s=10, alpha=0.5, label='meas_x')
    axes[0, 1].set_xlabel('time(s)')
    axes[0, 1].set_ylabel('x')
    axes[0, 1].set_title('X coordinate over time')
    axes[0, 1].legend()
    axes[0, 1].grid(True)
    
    # 3. Y坐标随时间变化
    axes[1, 0].plot(time_steps, true_y, 'g-', label='real_y', linewidth=2)
    for i, (title, estimated_states) in enumerate(dict_estimated_states.items()):
        est_y = [state[1] for state in estimated_states]
        color = colors[i % len(colors)]
        linestyle = linestyles[i % len(linestyles)]
        axes[1, 0].plot(time_steps, est_y, color=color, linestyle=linestyle, label=f'{title}_y', linewidth=2)
    
    axes[1, 0].scatter(time_steps, obs_y, c='r', s=10, alpha=0.5, label='meas_y')
    axes[1, 0].set_xlabel('time(s)')
    axes[1, 0].set_ylabel('y')
    axes[1, 0].set_title('Y coordinate over time')
    axes[1, 0].legend()
    axes[1, 0].grid(True)
    
    # 4. 位置误差
    for i, (title, estimated_states) in enumerate(dict_estimated_states.items()):
        est_x = [state[0] for state in estimated_states]
        est_y = [state[1] for state in estimated_states]
        color = colors[i % len(colors)]
        
        pos_errors = [
            np.sqrt((true_x[i]-est_x[i])**2 + (true_y[i]-est_y[i])**2)
            for i in range(len(true_x))
        ]
        axes[1, 1].plot(time_steps, pos_errors, color=color, linestyle='-', label=title, linewidth=2)
    
    axes[1, 1].set_xlabel('time(s)')
    axes[1, 1].set_ylabel('position error')
    axes[1, 1].set_title('Position estimation error')
    axes[1, 1].legend()
    axes[1, 1].grid(True)
    
    plt.tight_layout()
    plt.show()

class Estimator(Protocol):
    """观测器协议，定义统一的接口"""
    def update(self, l1: float, l2: float, q: float) -> Tuple[float, float, float, float]:
        ...
    state: np.ndarray

def create_estimator_trajectory(
    estimator_factory,
    measurements: List[np.ndarray],
    initial_state: np.ndarray
) -> List[np.ndarray]:
    """工厂函数：传入观测器和测量值，返回观测的状态向量
    
    Args:
        estimator_factory: 观测器工厂函数
        measurements: 测量值列表
        initial_state: 初始状态
        
    Returns:
        估计的状态向量列表
    """
    estimator = estimator_factory()
    estimator.state = initial_state.copy()
    
    estimated_states = [initial_state.copy()]
    for meas in measurements[1:]:
        l1, l2, q = meas
        estimated_state = estimator.update(l1, l2, q)
        estimated_states.append(np.array(estimated_state))
    
    return estimated_states

def run_simulation() -> None:
    """运行完整仿真"""
    # 配置参数
    config = EKFConfig(
        dt=0.1,
        d=0.8,
        q_x=0.1,
        q_y=0.1,
        sigma_l1=0.007,
        sigma_l2=0.007,
        # sigma_q=0.08
        sigma_q=0.05
    )
    
    # 创建轨迹生成器
    trajectory_generator = CircularTrajectoryGenerator(config)
    
    # 生成样本数据 (3秒)
    time_steps, true_states, measurements = generate_sample_data(
        config, duration=8.0, trajectory_generator=trajectory_generator
    )
    

    # 使用工厂函数创建不同观测器的轨迹
    dict_estimated_states = {}
    
    # EKF估计器
    def create_ekf():
        return ExtendedKalmanFilter(config)
    
    dict_estimated_states["EKF"] = create_estimator_trajectory(
        create_ekf, measurements, true_states[0]
    )
    
    # 运动学解算器
    def create_kinematic():
        return KinematicSolver(config)
    
    dict_estimated_states["Kinematic"] = create_estimator_trajectory(
        create_kinematic, measurements, true_states[0]
    )
    
    # 绘制结果
    plot_results(time_steps, true_states, measurements, dict_estimated_states, config)

if __name__ == "__main__":
    run_simulation()