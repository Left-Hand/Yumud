import numpy as np
from scipy import signal
import matplotlib.pyplot as plt

def analyze_tap_bound():
    """分析不同类型FIR滤波器的系数范围"""
    
    fs = 1000
    fc = 200
    taps = 101
    
    # 不同类型的FIR滤波器
    filters = {
        '低通-矩形窗': signal.firwin(taps, fc, fs=fs, window='boxcar'),
        '低通-汉明窗': signal.firwin(taps, fc, fs=fs, window='hamming'),
        # '低通-凯泽窗': signal.firwin(taps, fc, fs=fs, window=('kaiser', 8.0)),
        '高通-汉明窗': signal.firwin(taps, fc, fs=fs, pass_zero=False, window='hamming'),
        '带通-汉明窗': signal.firwin(taps, [100, 300], fs=fs, window='hamming')
    }
    
    results = {}
    for name, coeffs in filters.items():
        results[name] = {
            'min': np.min(coeffs),
            'max': np.max(coeffs),
            'max_abs': np.max(np.abs(coeffs)),
            'mean_abs': np.mean(np.abs(coeffs)),
            'std': np.std(coeffs)
        }
    
    # 显示结果
    print("FIR系数统计分析:")
    print("=" * 70)
    for name, stats in results.items():
        print(f"{name:20} | 范围: [{stats['min']:.4f}, {stats['max']:.4f}]")
        print(f"{'':20} | 最大绝对值: {stats['max_abs']:.4f}")
        print(f"{'':20} | 平均绝对值: {stats['mean_abs']:.4f}")
        print("-" * 70)
    
    return results, filters

# # 运行分析
# results, coeffs_dict = analyze_tap_bound()

import numpy as np
from scipy import signal
import matplotlib.pyplot as plt

#plt设置正常中文显示
plt.rcParams['font.sans-serif'] = ['SimHei']
def analyze_common_fir_coefficients():
    """分析常用FIR滤波器的系数范围"""
    
    fs = 1000
    taps = 101
    
    test_cases = []
    
    # 1. 各种截止频率的低通滤波器
    for fc in [50, 100, 200, 300, 400]:
        coeffs = signal.firwin(taps, fc, fs=fs, window='hamming')
        test_cases.append((f'低通{fc}Hz', coeffs))
    
    # 2. 高通滤波器
    for fc in [100, 200, 300]:
        coeffs = signal.firwin(taps, fc, fs=fs, pass_zero=False, window='hamming')
        test_cases.append((f'高通{fc}Hz', coeffs))
    
    # 3. 带通滤波器
    coeffs = signal.firwin(taps, [100, 300], fs=fs, window='hamming')
    test_cases.append(('带通[100,300]Hz', coeffs))
    
    # 4. 带阻滤波器
    coeffs = signal.firwin(taps, [200, 400], fs=fs, pass_zero=False, window='hamming')
    test_cases.append(('带阻[200,400]Hz', coeffs))
    
    # 5. 不同窗函数
    windows = ['hamming', 'hann', 'blackman', 'kaiser']
    for win in windows:
        if win == 'kaiser':
            coeffs = signal.firwin(taps, 200, fs=fs, window=(win, 8.0))
        else:
            coeffs = signal.firwin(taps, 200, fs=fs, window=win)
        test_cases.append((f'低通200Hz({win})', coeffs))
    
    # 分析结果
    results = []
    for name, coeffs in test_cases:
        max_abs = np.max(np.abs(coeffs))
        exceeds_1 = max_abs > 1.0 + 1e-10
        
        results.append({
            'name': name,
            'max_abs': max_abs,
            'exceeds_1': exceeds_1,
            'min': np.min(coeffs),
            'max': np.max(coeffs),
            'mean_abs': np.mean(np.abs(coeffs)),
            'coeffs': coeffs
        })
    
    # 显示结果
    print("常用FIR滤波器系数范围分析:")
    print("=" * 80)
    
    exceed_count = 0
    for r in results:
        status = "✓" if not r['exceeds_1'] else "✗"
        print(f"{status} {r['name']:25} | max|coeff| = {r['max_abs']:.6f}")
        print(f"   范围: [{r['min']:.6f}, {r['max']:.6f}]")
        if r['exceeds_1']:
            exceed_count += 1
            print(f"   ⚠️ 注意：超出1.0!")
        print("-" * 80)
    
    print(f"\n总结：{len(results)}个测试用例中，{exceed_count}个超出[-1,1]范围")
    
    # 可视化所有系数的分布
    plt.figure(figsize=(12, 6))
    
    all_coeffs = []
    labels = []
    for r in results:
        all_coeffs.extend(r['coeffs'])
        labels.extend([r['name']] * len(r['coeffs']))
    
    plt.boxplot([r['coeffs'] for r in results[:10]], labels=[r['name'] for r in results[:10]])
    plt.axhline(1.0, color='r', linestyle='--', alpha=0.5, label='上界+1')
    plt.axhline(-1.0, color='r', linestyle='--', alpha=0.5, label='下界-1')
    plt.xticks(rotation=45)
    plt.ylabel('系数值')
    plt.title('常用FIR滤波器系数分布')
    plt.grid(True, alpha=0.3)
    plt.tight_layout()
    plt.show()
    
    return results

# 运行分析
results = analyze_common_fir_coefficients()