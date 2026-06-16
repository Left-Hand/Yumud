我来用规范的伪代码解读这个流体模拟和粒子系统的处理流程。## 砂粒流体模拟系统 - 规范伪代码解读

这是一个基于 **MPS（Moving Particle Semi-implicit）方法** 的 2D 流体/砂粒模拟系统。以下是核心处理流程的规范伪代码表示：

### **整体流程架构**

```
MAIN_SIMULATION_LOOP:
  REPEAT:
    1. InitParticles()              // 初始化阶段
    2. ParticleIntegrate()          // 粒子速度和位置更新
    3. PushParticlesApart()         // 粒子间碰撞分离
    4. density_update()             // 密度计算
    5. particles_to_grid()          // 粒子→网格信息映射
    6. compute_grid_forces()        // 网格压力/力场计算
    7. grid_to_particles()          // 网格→粒子速度更新（FLIP/PIC混合）
    8. screen_update()              // 结果可视化
```

---

### **详细函数伪代码**

#### **1️⃣ InitParticles() - 初始化**

```pseudocode
PROCEDURE InitParticles()
  // 清零所有物理量
  RESET uVel, vVel, uPrev, vPrev, particleVel ← 0
  RESET particleDensity ← 0
  
  // 建立固体边界掩码（边界为0，内部为1）
  FOR x ← 0 TO CellNumX-1 DO
    FOR y ← 0 TO CellNumY-1 DO
      IF (x == 0 OR x == CellNumX-1 OR y == 0 OR y == CellNumY-1) THEN
        solidMask[x,y] ← 0      // 边界为固体
      ELSE
        solidMask[x,y] ← 1      // 内部为流体区域
      END IF
    END FOR
  END FOR
  
  // 以蜂窝形阵列初始化粒子位置（紧密堆积）
  p_num ← 0
  dx ← 2 × ParticleRadius
  dy ← √3/2 × dx              // 六边形堆积的 y 间距
  
  FOR i ← 0 TO CellNumX-1 DO
    FOR j ← 0 TO CellNumY-1 DO
      IF p_num < NumberOfParticles THEN
        // 奇偶行错位
        px ← Spacing + ParticleRadius + dx×i + (j%2 == 1 ? ParticleRadius : 0)
        py ← Spacing + ParticleRadius + dy×j
        
        // 检查是否超出网格边界
        IF px <= max_x AND py <= max_y THEN
          particlePos[p_num] ← (px, py)
          p_num ← p_num + 1
        END IF
      END IF
    END FOR
  END FOR
  
  // 剩余粒子初始化在角落
  FOR p_num TO NumberOfParticles-1 DO
    particlePos[p_num] ← (Spacing + ParticleRadius, Spacing + ParticleRadius)
  END FOR
END PROCEDURE
```

---

#### **2️⃣ ParticleIntegrate() - 粒子物理积分**

```pseudocode
PROCEDURE ParticleIntegrate(xAccel, yAccel)
  FOR i ← 0 TO NumberOfParticles-1 DO
    // 速度更新：v ← v + a×dt
    particleVel[i].x ← particleVel[i].x + xAccel × dt
    particleVel[i].y ← particleVel[i].y + yAccel × dt
    
    // 位置更新：x ← x + v×dt（显式欧拉法）
    particlePos[i].x ← particlePos[i].x + particleVel[i].x × dt
    particlePos[i].y ← particlePos[i].y + particleVel[i].y × dt
    
    // 边界碰撞处理（罚函数法）
    IF particlePos[i].x < minX THEN
      particlePos[i].x ← minX
      particleVel[i].x ← particleVel[i].x × BOUNCYNESS  // 反弹速度衰减
    END IF
    
    IF particlePos[i].x > maxX THEN
      particlePos[i].x ← maxX
      particleVel[i].x ← particleVel[i].x × BOUNCYNESS
    END IF
    
    // y 方向同理
    SIMILAR_FOR_Y_DIRECTION(...)
  END FOR
END PROCEDURE
```

---

#### **3️⃣ PushParticlesApart() - 粒子碰撞分离**

```pseudocode
PROCEDURE PushParticlesApart(nIters)
  // 第一步：建立空间哈希表（加速邻域查询）
  // 统计每个网格单元的粒子数
  FOR i ← 0 TO NumberOfParticles-1 DO
    grid_x ← particlePos[i].x / Spacing
    grid_y ← particlePos[i].y / Spacing
    cellIndex ← grid_x + grid_y × CellNumX
    cellParticleCountPrefix[cellIndex]++
  END FOR
  
  // 前缀和扫描（生成粒子在数组中的起始位置）
  prefix ← 0
  FOR cellIndex ← 0 TO CellCount-1 DO
    temp ← cellParticleCountPrefix[cellIndex]
    cellParticleCountPrefix[cellIndex] ← prefix
    prefix ← prefix + temp
  END FOR
  
  // 重排粒子ID到粒子数组
  FOR i ← 0 TO NumberOfParticles-1 DO
    cellIndex ← CELL_INDEX(particlePos[i])
    gridIndex ← --cellParticleCountPrefix[cellIndex]
    particlePosId[gridIndex] ← i
  END FOR
  
  minDist ← 2 × ParticleRadius
  minDist² ← minDist²
  
  // 第二步：迭代分离相邻粒子
  FOR iter ← 0 TO nIters-1 DO
    FOR i ← 0 TO NumberOfParticles-1 DO
      px, py ← particlePos[i]
      
      // 查询 3×3 邻域网格
      FOR xi ∈ [floor(px/Spacing)-1, floor(px/Spacing)+1] DO
        FOR yi ∈ [floor(py/Spacing)-1, floor(py/Spacing)+1] DO
          cellIndex ← xi + yi × CellNumX
          
          // 遍历该网格中的所有粒子
          FOR j ← cellParticleCountPrefix[cellIndex] TO cellParticleCountPrefix[cellIndex+1]-1 DO
            id ← particlePosId[j]
            IF id == i THEN CONTINUE END IF
            
            qx, qy ← particlePos[id]
            dx ← qx - px
            dy ← qy - py
            d² ← dx² + dy²
            
            // 如果粒子重叠（d < minDist）
            IF d² < minDist² AND d² ≠ 0 THEN
              d ← √d²
              s ← 0.5 × (minDist - d) / d
              
              // 推动粒子分离
              particlePos[i] ← particlePos[i] - (dx×s, dy×s)
              particlePos[id] ← particlePos[id] + (dx×s, dy×s)
            END IF
          END FOR
        END FOR
      END FOR
    END FOR
  END FOR
  
  // 再次边界碰撞处理
  FOR i ← 0 TO NumberOfParticles-1 DO
    CLAMP_TO_BOUNDARY(particlePos[i], particleVel[i])
  END FOR
END PROCEDURE
```

---

#### **4️⃣ density_update() - 密度计算**

```pseudocode
PROCEDURE density_update()
  RESET particleDensity ← 0
  
  // 使用双线性插值分布粒子质量到 2×2 网格
  FOR i ← 0 TO NumberOfParticles-1 DO
    px, py ← particlePos[i]  （限制在网格内）
    
    // 找到左下角网格点
    x0 ← floor(px / Spacing)
    y0 ← floor(py / Spacing)
    x1 ← x0 + 1
    y1 ← y0 + 1
    
    // 计算双线性权重
    tx ← (px - x0×Spacing) / Spacing   // [0,1]
    ty ← (py - y0×Spacing) / Spacing
    
    sx ← 1 - tx
    sy ← 1 - ty
    
    // 分布到 4 个网格点
    particleDensity[x0,y0] += sx × sy
    particleDensity[x1,y0] += tx × sy
    particleDensity[x1,y1] += tx × ty
    particleDensity[x0,y1] += sx × ty
  END FOR
  
  // 初始化时计算静息密度
  IF particleRestDensity == 0 THEN
    sum ← 0
    fluidCellCount ← 0
    FOR each cell DO
      IF cellType[cell] == FLUID THEN
        sum ← sum + particleDensity[cell]
        fluidCellCount++
      END IF
    END FOR
    particleRestDensity ← sum / fluidCellCount
  END IF
END PROCEDURE
```

---

#### **5️⃣ particles_to_grid() - 粒子→网格映射**

```pseudocode
PROCEDURE particles_to_grid()
  // 初始化网格状态
  RESET uVel, vVel, uWeight, vWeight ← 0
  
  // 初始化网格单元类型
  FOR each cell DO
    cellType[cell] ← (solidMask[cell] == 0) ? SOLID : AIR
  END FOR
  
  // 标记包含粒子的网格为流体
  FOR i ← 0 TO NumberOfParticles-1 DO
    cellIndex ← CELL_INDEX(particlePos[i])
    cellType[cellIndex] ← FLUID
  END FOR
  
  // 对 x 和 y 速度分量分别处理
  FOR component ← 0 TO 1 DO
    // component=0: u速度（x方向），偏移量 dx=0, dy=h/2
    // component=1: v速度（y方向），偏移量 dx=h/2, dy=0
    
    RESET uVel ← 0, uWeight ← 0    (或 vVel, vWeight)
    
    // 将粒子速度映射到网格
    FOR i ← 0 TO NumberOfParticles-1 DO
      // 根据速度分量的采样位置，计算双线性权重
      x0, y0, x1, y1 ← GRID_CELLS(particlePos[i], component)
      tx, ty ← INTERPOLATION_WEIGHTS(...)
      sx ← 1 - tx, sy ← 1 - ty
      
      w0 ← sx × sy
      w1 ← tx × sy
      w2 ← tx × ty
      w3 ← sx × ty
      
      pv ← particleVel[i][component]
      
      // 累积加权速度
      uVel[x0,y0] += pv × w0,  uWeight[x0,y0] += w0
      uVel[x1,y0] += pv × w1,  uWeight[x1,y0] += w1
      uVel[x1,y1] += pv × w2,  uWeight[x1,y1] += w2
      uVel[x0,y1] += pv × w3,  uWeight[x0,y1] += w3
    END FOR
    
    // 计算加权平均速度
    FOR each cell DO
      IF uWeight[cell] > 0 THEN
        uVel[cell] ← uVel[cell] / uWeight[cell]
      END IF
    END FOR
    
    // 在固体边界处重置速度为前一帧值
    FOR x ← 0 TO CellNumX-1 DO
      FOR y ← 0 TO CellNumY-1 DO
        IF cellType[x,y] == SOLID THEN
          uVel[x,y] ← uPrev[x,y]
        ELSE IF component == 0 AND cellType[x-1,y] == SOLID THEN
          uVel[x,y] ← uPrev[x,y]    // u 速度在左边界受影响
        ELSE IF component == 1 AND cellType[x,y-1] == SOLID THEN
          vVel[x,y] ← vPrev[x,y]    // v 速度在下边界受影响
        END IF
      END FOR
    END FOR
  END FOR
END PROCEDURE
```

---

#### **6️⃣ compute_grid_forces() - 压力求解**

```pseudocode
PROCEDURE compute_grid_forces(nIters)
  // 备份当前速度用于 FLIP 修正
  uPrev ← COPY(uVel)
  vPrev ← COPY(vVel)
  
  // Gauss-Seidel 迭代求解泊松方程
  FOR iteration ← 0 TO nIters-1 DO
    FOR x ← 1 TO CellNumX-2 DO
      FOR y ← 1 TO CellNumY-2 DO
        IF cellType[x,y] ≠ FLUID THEN CONTINUE END IF
        
        // 获取邻域网格
        center ← (x, y)
        left ← (x-1, y)
        right ← (x+1, y)
        bottom ← (x, y-1)
        top ← (x, y+1)
        
        // 固体掩码（边界条件）
        sx0 ← solidMask[left]      // 左边界
        sx1 ← solidMask[right]     // 右边界
        sy0 ← solidMask[bottom]    // 下边界
        sy1 ← solidMask[top]       // 上边界
        s ← sx0 + sx1 + sy0 + sy1
        
        IF s == 0 THEN CONTINUE END IF   // 被固体包围，无法求解
        
        // 计算速度散度（质量守恒）
        div ← uVel[right] - uVel[center] + vVel[top] - vVel[center]
        
        // 密度约束修正
        IF particleRestDensity > 0 THEN
          compression ← particleDensity[center] - particleRestDensity
          IF compression > 0 THEN
            compression ← compression × stiffnessCoefficient
            div ← div - compression     // 减少散度以压缩密度
          END IF
        END IF
        
        // 泊松方程求解：∇·p = div
        p ← -div / s
        p ← p × overRelaxation      // 松弛因子加速收敛
        
        // 压力梯度修正速度
        uVel[center] ← uVel[center] - sx0 × p
        uVel[right] ← uVel[right] + sx1 × p
        vVel[center] ← vVel[center] - sy0 × p
        vVel[top] ← vVel[top] + sy1 × p
      END FOR
    END FOR
  END FOR
END PROCEDURE
```

---

#### **7️⃣ grid_to_particles() - 网格→粒子（FLIP/PIC混合）**

```pseudocode
PROCEDURE grid_to_particles()
  flipRatio ← 0.9   // FLIP 和 PIC 的混合比例
  
  FOR component ← 0 TO 1 DO
    f ← (component == 0) ? uVel : vVel
    prevF ← (component == 0) ? uPrev : vPrev
    
    FOR i ← 0 TO NumberOfParticles-1 DO
      // 计算粒子周围的 4 个采样点
      x0, y0, x1, y1 ← GRID_CELLS(particlePos[i], component)
      tx, ty ← INTERPOLATION_WEIGHTS(...)
      sx ← 1-tx, sy ← 1-ty
      
      d0 ← sx × sy
      d1 ← tx × sy
      d2 ← tx × ty
      d3 ← sx × ty
      
      // 检查 4 个网格点的有效性（不是空气，或邻近不是空气）
      valid ← CALCULATE_VALIDITY(cellType, nr0, nr1, nr2, nr3)
      
      d ← valid₀×d₀ + valid₁×d₁ + valid₂×d₂ + valid₃×d₃
      
      IF d ≤ 0 THEN CONTINUE END IF   // 所有采样点无效，跳过
      
      // PIC 方法：线性插值当前网格速度
      picV ← (valid₀×d₀×f[nr0] + valid₁×d₁×f[nr1] + valid₂×d₂×f[nr2] + valid₃×d₃×f[nr3]) / d
      
      // FLIP 方法：累积速度变化
      corr ← (valid₀×d₀×(f[nr0]-prevF[nr0]) + ... + valid₃×d₃×(f[nr3]-prevF[nr3])) / d
      
      oldV ← particleVel[i][component]
      flipV ← oldV + corr     // 上一步速度 + 速度变化
      
      // 混合 PIC（平滑）和 FLIP（质量守恒）
      particleVel[i][component] ← (1-flipRatio)×picV + flipRatio×flipV
    END FOR
  END FOR
END PROCEDURE
```

---

#### **8️⃣ screen_update() - 可视化**

```pseudocode
PROCEDURE screen_update()
  FOR y ← 1 TO CellNumY-2 DO
    FOR x ← 1 TO CellNumX-2 DO
      cell ← cellType[x,y]
      
      IF cell == FLUID_CELL THEN
        LED_SetPixel(x*16 + y, ON)    // 流体显示为亮
      ELSE IF cell == SOLID_CELL THEN
        LED_SetPixel(x*16 + y, ON)    // 固体也显示为亮
      ELSE
        LED_SetPixel(x*16 + y, OFF)   // 空气显示为暗
      END IF
    END FOR
  END FOR
END PROCEDURE
```

---

### **关键算法特性总结**

| 特性 | 说明 |
|------|------|
| **方法** | MPS（移动粒子半隐式）+ FLIP/PIC混合 |
| **空间划分** | 规则网格哈希表 + 粒子前缀和排序 |
| **力求解** | Gauss-Seidel 迭代泊松方程求解 |
| **边界处理** | 罚函数法 + 固体掩码 |
| **速度更新** | FLIP（质量守恒）90% + PIC（稳定性）10% |
| **时间积分** | 显式欧拉法 |
| **复杂度** | O(N×M) 其中N为粒子数，M为迭代次数 |