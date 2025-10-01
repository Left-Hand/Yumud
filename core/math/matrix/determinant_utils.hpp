#pragma once

namespace ymd{

#if 0
// 辅助函数：计算跳过指定行和列的行列式
[[nodiscard]] __fast_inline constexpr T compute_determinant_without_row_col(size_t skip_row, size_t skip_col) const {
    constexpr size_t N = R - 1;
    
    if constexpr (N == 1) {
        // 找到唯一剩余的元素
        const size_t row = (skip_row == 0) ? 1 : 0;
        const size_t col = (skip_col == 0) ? 1 : 0;
        return this->at(row, col);
    } else if constexpr (N == 2) {
        // 2x2 子矩阵的行列式
        // 找到剩余的行和列索引
        size_t rows[2], cols[2];
        size_t r_idx = 0;
        for (size_t i = 0; i < R; ++i) {
            if (i != skip_row) rows[r_idx++] = i;
        }
        size_t c_idx = 0;
        for (size_t j = 0; j < C; ++j) {
            if (j != skip_col) cols[c_idx++] = j;
        }
        
        return this->at(rows[0], cols[0]) * this->at(rows[1], cols[1]) 
            - this->at(rows[0], cols[1]) * this->at(rows[1], cols[0]);
    } else {
        // 对于更大的矩阵，使用拉普拉斯展开
        T det = T(0);
        int sign = 1;
        
        // 选择第一行进行展开（跳过skip_row和skip_col后剩余的第一行）
        size_t expand_row = 0;
        while (expand_row == skip_row && expand_row < R) {
            expand_row++;
        }
        
        size_t col_idx = 0;
        for (size_t j = 0; j < C; ++j) {
            if (j == skip_col) continue;
            
            T cofactor = sign * this->at(expand_row, j) 
                    * compute_determinant_without_row_col_col(expand_row, j, skip_row, skip_col);
            det += cofactor;
            sign = -sign;
            col_idx++;
        }
        
        return det;
    }
}

// 辅助函数：计算跳过两行两列的行列式
[[nodiscard]] __fast_inline constexpr T compute_determinant_without_row_col_col(
    size_t skip_row1, size_t skip_col1, size_t skip_row2, size_t skip_col2) const {
    
    constexpr size_t N = R - 2;
    
    if constexpr (N == 1) {
        // 找到唯一剩余的元素
        for (size_t i = 0; i < R; ++i) {
            if (i != skip_row1 && i != skip_row2) {
                for (size_t j = 0; j < C; ++j) {
                    if (j != skip_col1 && j != skip_col2) {
                        return this->at(i, j);
                    }
                }
            }
        }
        return T(0); // 不应该到达这里
    } else {
        // 递归计算更小的子矩阵
        Matrix<T, N, N> submatrix;
        size_t dst_row = 0;
        
        for (size_t src_row = 0; src_row < R; ++src_row) {
            if (src_row == skip_row1 || src_row == skip_row2) continue;
            
            size_t dst_col = 0;
            for (size_t src_col = 0; src_col < C; ++src_col) {
                if (src_col == skip_col1 || src_col == skip_col2) continue;
                
                submatrix.at(dst_row, dst_col) = this->at(src_row, src_col);
                dst_col++;
            }
            dst_row++;
        }
        
        return submatrix.determinant();
    }
}


[[nodiscard]] __fast_inline constexpr T compute_4x4_minor_determinant(size_t i, size_t j) const {
    // 手动展开4x4矩阵的3x3子矩阵行列式计算
    // 选择展开行（跳过第i行）
    const size_t rows[3];
    size_t r_idx = 0;
    for (size_t r = 0; r < 4; ++r) {
        if (r != i) rows[r_idx++] = r;
    }
    
    // 拉普拉斯展开计算3x3行列式
    return this->at(rows[0], (j+1)%4) * (this->at(rows[1], (j+2)%4) * this->at(rows[2], (j+3)%4) - 
                                        this->at(rows[1], (j+3)%4) * this->at(rows[2], (j+2)%4))
        - this->at(rows[0], (j+2)%4) * (this->at(rows[1], (j+1)%4) * this->at(rows[2], (j+3)%4) - 
                                        this->at(rows[1], (j+3)%4) * this->at(rows[2], (j+1)%4))
        + this->at(rows[0], (j+3)%4) * (this->at(rows[1], (j+1)%4) * this->at(rows[2], (j+2)%4) - 
                                        this->at(rows[1], (j+2)%4) * this->at(rows[2], (j+1)%4));
}
#endif


}