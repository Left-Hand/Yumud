import cv2
import numpy as np
import os
import math
from scipy.spatial import ConvexHull


def divide_by_top_left(matrix):
    """
    将给定的矩阵除以其左上角的元素。
    
    参数:
    matrix (np.ndarray): 输入的二维矩阵。
    
    返回:
    np.ndarray: 新的矩阵，其中每个元素都除以原矩阵左上角的元素。
    """
    # 确保矩阵不是空的
    if matrix.size == 0:
        raise ValueError("Input matrix is empty.")
    
    # 确保左上角元素不为零
    if matrix[0, 0] == 0:
        raise ValueError("Top-left element is zero.")
    
    # 计算除法
    result_matrix = matrix / matrix[0, 0]
    
    return result_matrix

def matrix_to_c_struct_text(matrix):
    # 容易与模板字符串混淆的占位符名称修改
    template = """\
struct InvPerspectiveConfig{{ 
    real_t H1 = real_t({H_1:.5f});
    real_t H2 = real_t({H_2:.5f});
    real_t H3 = real_t({H_3:.5f});
    real_t H4 = real_t({H_4:.5f});
    real_t H5 = real_t({H_5:.5f});
    real_t H6 = real_t({H_6:.5f});
    real_t H7 = real_t({H_7:.5f});
    real_t H8 = real_t({H_8:.5f});
}};
"""

    # 填充结构体模板
    config_text = template.format(
        H_1=matrix[0][0],
        H_2=matrix[0][1],
        H_3=matrix[0][2],
        H_4=matrix[1][0],
        H_5=matrix[1][1],
        H_6=matrix[1][2],
        H_7=matrix[2][0],
        H_8=matrix[2][1]
    )

    return config_text

size = (94, 60)
scale = 0.82
# 使用索引访问元组的元素
pts1 = np.float32([[0, 0], [size[0], 0], [size[0], size[1]], [0, size[1]]])
pts2 = np.float32([[0, 0], [size[0], 0], [size[0] * scale, size[1]], [size[0] * (1 - scale), size[1]]])

print(pts1, pts2)
M = cv2.getPerspectiveTransform(pts1, pts2)

print(M)


# 使用 NumPy 计算 M 的逆矩阵
M_inv = np.linalg.inv(M)
M_inv = divide_by_top_left(M_inv)

print(matrix_to_c_struct_text(M_inv))
