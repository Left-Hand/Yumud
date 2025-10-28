#pyright strict

import numpy as np
import matplotlib.pyplot as plt
from scipy.integrate import solve_ivp

class PendulumSMC:
    def __init__(self, m=1.0, l=1.0, b=0.1, g=9.81):
        self.m = m  # 质量 kg
        self.l = l  # 绳长 m
        self.b = b  # 阻尼系数 N·s/m
        self.g = g  # 重力加速度 m/s²
        
        # 控制器参数
        self.lambda_ = 2.0  # 滑模面参数
        self.eta = 5.0      # 切换增益
        self.k = 1.0        # 趋近律增益
        
        self.theta_d = 0.0  # 期望角度
    
    def smc_control(self, x, theta_d):
        """滑模控制器"""
        x1, x2 = x
        
        # 误差和滑模面
        e = x1 - theta_d
        s = x2 + self.lambda_ * e
        
        # 控制律
        u = self.m * self.l * (
            (self.g / self.l) * np.sin(x1) + 
            (self.b / self.m - self.lambda_) * x2 - 
            self.k * s - 
            self.eta * np.sign(s)
        )
        
        # 控制输入饱和（可选）
        u_max = 20.0
        u = np.clip(u, -u_max, u_max)
        
        return u, s
    
    def dynamics(self, t, x, theta_d):
        """系统动力学"""
        x1, x2 = x
        
        # 计算控制输入
        u, _ = self.smc_control(x, theta_d)
        
        # 状态方程
        dx1 = x2
        dx2 = - (self.g / self.l) * np.sin(x1) - (self.b / self.m) * x2 + u / (self.m * self.l)
        
        return [dx1, dx2]
    
    def simulate(self, x0, t_span, theta_d, t_eval=None):
        """仿真系统"""
        self.theta_d = theta_d
        # 求解微分方程
        sol = solve_ivp(
            lambda t, x: self.dynamics(t, x, theta_d),
            t_span,
            x0,
            t_eval=t_eval,
            method='RK23',  # Less computationally expensive than RK45
            rtol=1e-3,      # Slightly relaxed tolerance
            atol=1e-6
        )
                
        return sol

def plot_results(t, x1, x2, u, theta_d):
    """绘制结果"""
    fig, axes = plt.subplots(3, 1, figsize=(10, 8))
    
    # 角度曲线
    axes[0].plot(t, x1, 'b-', linewidth=2, label='实际角度 $x_1$')
    axes[0].plot(t, np.ones_like(t) * theta_d, 'r--', linewidth=2, label='期望角度 $\\theta_d$')
    axes[0].set_ylabel('角度 (rad)')
    axes[0].legend()
    axes[0].grid(True)
    axes[0].set_title('滑模控制 - 单摆系统响应')
    
    # 角速度曲线
    axes[1].plot(t, x2, 'g-', linewidth=2, label='角速度 $x_2$')
    axes[1].set_ylabel('角速度 (rad/s)')
    axes[1].legend()
    axes[1].grid(True)
    
    # 控制输入曲线
    axes[2].plot(t, u, 'm-', linewidth=2, label='控制输入 $u$')
    axes[2].set_ylabel('控制力 (N)')
    axes[2].set_xlabel('时间 (s)')
    axes[2].legend()
    axes[2].grid(True)
    
    plt.tight_layout()
    plt.show()

def calculate_control_history(controller, t, x_trajectory, theta_d):
    """计算控制输入历史"""
    u_history = []
    s_history = []
    print(len(t))
    
    for i in range(len(t)):
        x = x_trajectory[:, i]
        u, s = controller.smc_control(x, theta_d)
        u_history.append(u)
        s_history.append(s)
    
    return np.array(u_history), np.array(s_history)

# 主程序
if __name__ == "__main__":
    # 创建控制器实例
    controller = PendulumSMC(m=1.0, l=1.0, b=0.1, g=9.81)
    
    # 仿真参数
    x0 = [np.pi/2, 0.0]  # 初始状态 [theta, omega] = [90°, 0 rad/s]
    theta_d = 0.0        # 期望角度 (竖直向下)
    t_span = [0, 10]     # 仿真时间
    t_eval = np.linspace(0, 10, 1000)  # 时间点
    
    # 运行仿真
    print("开始仿真...")
    sol = controller.simulate(x0, t_span, theta_d, t_eval)
    print("仿真完成！")
    
    # 计算控制输入历史
    u_history, s_history = calculate_control_history(controller, sol.t, sol.y, theta_d)
    
    # # 绘制结果
    # plot_results(sol.t, sol.y[0], sol.y[1], u_history, theta_d)
    
    # # 额外绘制滑模面
    # plt.figure(figsize=(10, 4))
    # plt.plot(sol.t, s_history, 'r-', linewidth=2)
    # plt.xlabel('时间 (s)')
    # plt.ylabel('滑模面 $s$')
    # plt.title('滑模面随时间变化')
    # plt.grid(True)
    # plt.show()
    
    # # 打印一些统计信息
    # print(f"最终角度: {sol.y[0,-1]:.4f} rad")
    # print(f"最终角速度: {sol.y[1,-1]:.4f} rad/s")
    # print(f"控制输入范围: [{u_history.min():.2f}, {u_history.max():.2f}] N")
    # print(f"稳态误差: {abs(sol.y[0,-1] - theta_d):.6f} rad")