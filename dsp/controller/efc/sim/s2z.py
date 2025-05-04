import control as ctl
import numpy as np

s = ctl.TransferFunction(ctl.tf('s'))
def s2z(H_s, T, method='tustin', epsilon=1e-5):
    """
    将 s 域传递函数转换为 z 域传递函数，支持非真分式（如纯微分器 s）。
    
    参数：
        H_s      : s 域传递函数 (control.TransferFunction)
        T        : 采样时间 (float)
        method   : 离散化方法 ('tustin', 'zoh', 'backward_diff')
        epsilon  : 用于正则化非真分式的小参数 (默认 1e-5)
    
    返回：
        H_z      : z 域传递函数 (control.TransferFunction)
    """
    
    global s
    # 检查是否为纯微分器（如 s, s^2, ...）
    if True:
        print(f"警告: H(s) 是非真分式（分子阶数={H_s.zeros().size} > 分母阶数={H_s.poles().size}），将自动正则化。")
        
        # 构造正则化分母 (s + epsilon)^k
        k = H_s.zeros().size - H_s.poles().size
        den_regularized = (s + epsilon)**k
        
        # 正则化传递函数：H_s_reg = H_s / (s + epsilon)^k
        H_s_reg = H_s / den_regularized
    else:
        H_s_reg = H_s
    
    # 离散化
    if method == 'backward_diff':
        # 后向差分法 (s = (1 - z^-1)/T)
        z = ctl.tf('z', dt=T)
        s_to_z = (1 - z**-1) / T
        H_z = ctl.subsys(H_s_reg, s_to_z)
    else:
        # 默认使用双线性变换或 ZOH
        H_z = ctl.c2d(H_s_reg, T, method=method)
    
    return H_z

# ================= 测试用例 =================
if __name__ == "__main__":
    # 示例 1: 纯微分器 s
    s = ctl.tf('s')
    H_s = s
    T = 0.01
    
    print("="*50)
    print("测试纯微分器 H(s) = s:")
    H_z = s2z(H_s, T, method='tustin')
    print("双线性变换结果:\n", H_z)
    
    H_z_backward = s2z(H_s, T, method='backward_diff')
    print("后向差分结果:\n", H_z_backward)
    
    # 示例 2: 非真分式 (s^2)/(s + 1)
    H_s_nonproper = s**2 / (s + 1)
    print("\n" + "="*50)
    print("测试非真分式 H(s) = s^2/(s + 1):")
    H_z_nonproper = s2z(H_s_nonproper, T, method='tustin')
    print("正则化后离散结果:\n", H_z_nonproper)
    
    # 示例 3: 真分式 (s + 1)/(s^2 + 2s + 5)
    H_s_proper = (s + 1)/(s**2 + 2*s + 5)
    print("\n" + "="*50)
    print("测试真分式 H(s) = (s + 1)/(s^2 + 2s + 5):")
    H_z_proper = s2z(H_s_proper, T, method='tustin')
    print("直接离散结果:\n", H_z_proper)