import numpy as np
import matplotlib.pyplot as plt


#设置matplot的中文字体
plt.rcParams['font.sans-serif'] = ['SimHei']
plt.rcParams['mathtext.fontset'] = 'stix'  # Use STIX font for math symbols

def inverse_sqrt_newton(x, initial_guess, iterations=3):
    """使用牛顿法计算逆平方根"""
    g = initial_guess
    history = [g]
    
    print(f"计算 1/√({x})")
    print(f"初始猜测: g₀ = {g:.6f}")
    print("-" * 50)
    
    for i in range(iterations):
        g_new = 0.5 * g * (3 - g**2 * x)
        error = abs(g_new - 1/np.sqrt(x))
        
        print(f"迭代 {i+1}:")
        print(f"  g_{i} = {g:.8f}")
        print(f"  g_{i}² × x = {(g**2 * x):.8f}")
        print(f"  3 - g_{i}² × x = {(3 - g**2 * x):.8f}")
        print(f"  g_{i+1} = 0.5 × {g:.8f} × {3 - g**2 * x:.8f}")
        print(f"        = {g_new:.8f}")
        print(f"  真实值: {1/np.sqrt(x):.8f}")
        print(f"  误差: {error:.8f}")
        print("-" * 50)
        
        g = g_new
        history.append(g)
    
    return history

def plot_newton_iteration(x, history):
    """绘制牛顿迭代过程的可视化"""
    # 创建函数 f(g) = 1/g^2 - x
    g_vals = np.linspace(0.1, 2, 400)
    f_vals = 1/(g_vals**2) - x
    
    # 真实解
    true_solution = 1/np.sqrt(x)
    
    # 创建图形
    fig, axes = plt.subplots(1, 2, figsize=(14, 6))
    
    # 左图：函数曲线和切线
    ax1 = axes[0]
    ax1.plot(g_vals, f_vals, 'b-', label='$f(g) = 1/g^2 - x$', linewidth=2)
    ax1.axhline(y=0, color='k', linestyle='-', alpha=0.3)
    ax1.axvline(x=true_solution, color='r', linestyle='--', alpha=0.5, label=f'真实解: {true_solution:.4f}')
    
    # 标记迭代点
    colors = ['r', 'g', 'm', 'c', 'y']
    for i, (g, g_next) in enumerate(zip(history[:-1], history[1:])):
        # 计算切线
        f_g = 1/(g**2) - x
        f_prime_g = -2/(g**3)
        tangent_line = f_prime_g * (g_vals - g) + f_g
        
        ax1.plot(g_vals, tangent_line, color=colors[i], linestyle='--', alpha=0.7, linewidth=1.5)
        ax1.plot(g, 0, 'o', color=colors[i], markersize=8, label=f'$g_{i}$ = {g:.4f}')
        ax1.plot(g, f_g, 'o', color=colors[i], markersize=8)
        
        # 添加垂直线显示迭代过程
        ax1.plot([g, g_next], [f_g, 0], ':', color=colors[i], alpha=0.7)
        ax1.annotate(f'$g_{i+1}$', xy=(g_next, 0), xytext=(g_next, -0.2 if i%2==0 else 0.2),
                    arrowprops=dict(arrowstyle='->', color=colors[i], alpha=0.7),
                    color=colors[i], fontsize=10)
    
    ax1.set_xlabel('$g$', fontsize=12)
    ax1.set_ylabel('$f(g)$', fontsize=12)
    ax1.set_title('牛顿迭代过程可视化 (寻找 $f(g)=0$)', fontsize=14)
    ax1.legend(loc='upper right')
    ax1.grid(True, alpha=0.3)
    ax1.set_xlim(0.3, 1.2)
    ax1.set_ylim(-3, 3)
    
    # 右图：误差收敛过程
    ax2 = axes[1]
    errors = [abs(g - true_solution) for g in history]
    iterations = range(len(history))
    
    ax2.semilogy(iterations, errors, 'bo-', linewidth=2, markersize=8, label='误差')
    
    # 添加二次收敛参考线
    if len(errors) > 1:
        ref_x = np.array(iterations[1:])
        # 二次收敛：误差按平方减小
        quadratic_ref = errors[1] * (errors[1]/errors[0])**(np.array(ref_x)-1)
        ax2.semilogy(ref_x, quadratic_ref, 'r--', alpha=0.7, label='二次收敛参考')
    
    ax2.set_xlabel('迭代次数', fontsize=12)
    ax2.set_ylabel('误差 (对数尺度)', fontsize=12)
    ax2.set_title('误差收敛过程', fontsize=14)
    ax2.legend()
    ax2.grid(True, alpha=0.3, which='both')
    
    plt.tight_layout()
    plt.show()
    
    # 打印收敛统计
    print("\n" + "="*60)
    print("收敛统计:")
    print("="*60)
    for i, g in enumerate(history):
        error = abs(g - true_solution)
        if i > 0:
            ratio = errors[i-1] / error if error > 0 else float('inf')
            print(f"迭代 {i}: 值 = {g:.10f}, 误差 = {error:.2e}, 误差比 = {ratio:.2f}")
        else:
            print(f"初始值: 值 = {g:.10f}, 误差 = {error:.2e}")


# 示例计算：计算 1/√(2)
x_value = 2.0
initial_guess = 0.3  # 故意选一个不是特别好的初始值，以展示收敛过程

# 执行迭代
history = inverse_sqrt_newton(x_value, initial_guess, iterations=5)

# 绘制可视化
plot_newton_iteration(x_value, history)
