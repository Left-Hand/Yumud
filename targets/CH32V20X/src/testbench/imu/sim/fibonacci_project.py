import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

def project_idx_to_v3(i, N):
    """从索引映射到三维球面点 (x, y, z)"""
    z = ((2 * i) / (N-1)) - 1
    x = np.sqrt(1-z * z) * np.cos( 2 * np.pi * i * 0.617)
    y = np.sqrt(1-z * z) * np.sin( 2 * np.pi * i * 0.617)
    return x, y, z


def fibonacci_sphere_sampling(N):
    # """Fibonacci 球面采样"""
    # indices = np.arange(N, dtype=float)
    # phi, theta = project_idx_to_v2(indices, N)
    # x, y, z = project_v2_to_v3(theta, phi)
    # return np.vstack((x, y, z)).T

    """Fibonacci 球面采样"""
    # i = np.arange(N, dtype=float)
    i = np.linspace(0, N-1, N, dtype=float)
    # phi = np.arccos(1 - 2 * indices / (N - 1))  # 极角
    # theta = 2 * np.pi * indices / ((1 + np.sqrt(5)) / 2)  # 方位角
    # theta, phi = project_idx_to_v2(indices, N)
    # print(theta, phi)
    # x, y, z = project_v2_to_v3(theta, phi)
    x,y,z = project_idx_to_v3(i, N)
    # print(i,z)
    return np.vstack((x, y, z)).T



def project_idx_to_v2(i, N):
    """从索引映射到二维平面点 (theta, phi)"""
    phi = np.arccos(1 - 2 * i / (N - 1))  # 极角
    theta = 2 * np.pi * i / ((1 + np.sqrt(5)) / 2)  # 方位角，基于黄金比例
    return theta, phi

def project_v2_to_v3(theta, phi):
    """从二维平面点映射到三维球面点 (x, y, z)"""
    x = np.cos(theta) * np.sin(phi)
    y = np.sin(theta) * np.sin(phi)
    z = np.cos(phi)
    return x, y, z

def project_v3_to_v2(x, y, z):
    """从三维球面点映射回二维平面点 (theta, phi)"""
    phi = np.arccos(z)  # 极角
    theta = np.arctan2(y, x)  # 方位角
    return theta, phi

# def project_v2_to_idx(theta, phi, N):
#     """从二维平面点映射回索引 i"""
#     i = (1 - np.cos(phi)) * (N - 1) / 2  # 计算索引
#     return int(i)


def plot_sphere(points):
    """绘制球面上的点"""
    fig = plt.figure()
    ax = fig.add_subplot(111, projection='3d')
    ax.scatter(points[:, 0], points[:, 1], points[:, 2], s=10)
    ax.set_xlabel('X')
    ax.set_ylabel('Y')
    ax.set_zlabel('Z')
    plt.show()

def plot_2d(points):
    """绘制二维平面上的点"""
    plt.scatter(points[:, 0], points[:, 1], s=10)
    plt.xlabel('X')
    plt.ylabel('Y')
    plt.title('2D Uniform Points')
    plt.show()

def main():
    # 生成二维平面上的均匀分布点
    N = 270
    x = np.linspace(0, 1, int(np.sqrt(N)))
    y = np.linspace(0, 1, int(np.sqrt(N)))
    xx, yy = np.meshgrid(x, y)
    points_2d = np.vstack((xx.flatten(), yy.flatten())).T

    # 绘制二维平面上的点
    plot_2d(points_2d)

    # 将二维点映射到球面上
    points_sphere = fibonacci_sphere_sampling(N)
    
    points_2d_inv_theta, points_2d_inv_phi = project_v3_to_v2(points_sphere[:, 0], points_sphere[:, 1], points_sphere[:, 2])
    points_2d_inv = np.vstack((points_2d_inv_theta, points_2d_inv_phi)).T

    # 绘制球面上的点
    plot_sphere(points_sphere)
    
    plot_2d(points_2d_inv)

if __name__ == "__main__":
    main()

