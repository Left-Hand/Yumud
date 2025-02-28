
import matplotlib.pyplot as plt

from iqtable import *


def __mpyf_l(x:np.int32, y:np.int32) -> np.int32:
        return np.int32((x * y) >> 31)

def __mpyf_u32(x:np.uint32, y:np.uint32) -> np.uint32:
        return np.int32((x * y) >> 32)
def iq_cos(uiq31Input:np.int32) -> np.int32:
    # Calculate index for sin and cos lookup using bits 31:26
    index = (uiq31Input >> 25) & 0x003f
    # Lookup S(k) and C(k) values.
    iq31Sin = iq31_sin_table[index]
    iq31Cos = iq31_cos_table[index]

    # Calculated x (the remainder) by subtracting the index from the unsigned
    # iq31 input. This can be accomplished by masking out the bits used for
    # the index.
    iq31X = np.int32(uiq31Input & 0x01ffffff)

    # 0.333*x*S(k)
    iq31Res = __mpyf_l(0x2aaaaaab,iq31X)  # Simulating __mpyf_l with integer multiplication and right shift
    iq31Res = __mpyf_l(iq31Sin,iq31Res)

    # -C(k) + 0.333*x*S(k)
    iq31Res = np.int32(iq31Res - iq31Cos)

    # 0.5*x*(-C(k) + 0.333*x*S(k))
    iq31Res = np.int32(iq31Res >> 1)
    iq31Res = __mpyf_l(iq31X,iq31Res)

    # -S(k) + 0.5*x*(-C(k) + 0.333*x*S(k))
    iq31Res = np.int32(iq31Res - iq31Sin)

    # x*(-S(k) + 0.5*x*(-C(k) + 0.333*x*S(k)))
    iq31Res = __mpyf_l(iq31X,iq31Res)

    # cos(Radian) = C(k) + x*(-S(k) + 0.5*x*(-C(k) + 0.333*x*S(k)))
    return np.int32(iq31Cos + iq31Res)
    # print(iq31X, "iq31X")
    
    # return np.int32(iq31Res)


def iq_cos2(uiq31Input:np.int32) -> np.int32:
    # Calculate index for sin and cos lookup using bits 31:26
    index = (uiq31Input >> 25) & 0x003f
    # Lookup S(k) and C(k) values.
    uiq32Sin = uiq32_sin_table[index]
    uiq32Cos = uiq32_cos_table[index]

    # Calculated x (the remainder) by subtracting the index from the unsigned
    # iq31 input. This can be accomplished by masking out the bits used for
    # the index.
    uiq32X = np.uint32(uiq31Input & 0x01ffffff)

    # 0.333*x*S(k)
    uiq32Res = __mpyf_u32(0x2aaaaaab * 2,uiq32X)  # Simulating __mpyf_u32 with integer multiplication and right shift
    uiq32Res = __mpyf_u32(uiq32Sin,uiq32Res)

    # -C(k) + 0.333*x*S(k)
    uiq32Res = np.uint32(uiq32Res - uiq32Cos)

    # 0.5*x*(-C(k) + 0.333*x*S(k))
    uiq32Res = np.uint32(uiq32Res >> 1)
    uiq32Res = __mpyf_u32(uiq32X,uiq32Res)

    # -S(k) + 0.5*x*(-C(k) + 0.333*x*S(k))
    uiq32Res = np.uint32(uiq32Res - uiq32Sin)

    # x*(-S(k) + 0.5*x*(-C(k) + 0.333*x*S(k)))
    uiq32Res = __mpyf_u32(uiq32X,uiq32Res)

    # cos(Radian) = C(k) + x*(-S(k) + 0.5*x*(-C(k) + 0.333*x*S(k)))
    return np.uint32(uiq32Cos + uiq32Res)
    # print(iq31X, "iq31X")
    
    # return np.int32(iq31Res)

def plot_array(array:np.int32):
    x_indices = range(len(array))
        
    # 绘制图形
    plt.plot(x_indices, array, marker='o')

    # 添加标题和标签
    plt.title('数组数据绘制')
    plt.xlabel('索引')
    plt.ylabel('数值')

    # 显示图形
    plt.grid(True)
    plt.show()


# plot_array(iq31_cos_table)

# 生成从 0 到 pi/2 之间的角度
angles = np.linspace(0, np.pi/4, 1000)

# 将角度转换为 uiq31Input 格式的整数
# 假设 uiq31Input 的范围是 0 到 2*pi 对应 0 到 2^31

# 计算这些角度的余弦值
cos_values = iq_cos(np.int32(angles * (2**31))) / (2**31)
cos_values2 = iq_cos2(np.int32(angles * (2**31))) / (2**32)

# 绘制角度与计算的余弦值之间的关系
plt.plot(angles, cos_values, label='iq_cos 计算的余弦值')
plt.plot(angles, cos_values2, label='iq_cos2 计算的余弦值')
plt.plot(angles, np.cos(angles), label='numpy 的余弦值', linestyle='--')
# print(index)
# plt.plot(angles, np.float32(index), label='序列', linestyle='--')

# 添加标题和标签
plt.title('iq_cos 函数输出与 numpy 余弦函数的比较')
plt.xlabel('角度 (弧度)')
plt.ylabel('余弦值')

# 添加图例
plt.legend()

# 显示图形
plt.grid(True)
plt.show()

# print(np.uint32(iq31_cos_table) * 2)
# print(np.uint32(iq31_sin_table) * 2)