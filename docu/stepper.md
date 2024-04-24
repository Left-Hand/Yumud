雕刻机动力学单轴动态分析

____
电力学

转矩控制
令机械角度为$theta$
令极对数为n
则电角度$\theta_e = n \theta + \theta_{f}$ 其中$\theta_{f}$为机械角度到点角度的补偿项

$$T_m = K I$$

SVPWM调制得电角度$\theta_{w}$
对于电角度差值$\theta_{ee} = \theta_{w} - \theta_e$封闭在$[-\pi, \pi]$时 有

$$T = T_m sin(\theta_ee) = K I sin(\theta_{w} - \theta_e)$$
$$T = T_m sin(\theta_ee) = K I (sin(\theta_{w})cos(\theta_e) - cos(\theta_w)sin(\theta_e))$$



故有策略得到得控制器目标转矩T 观测得theta_e 可得出调制的电流和角度


其中I为电机控制得电流 由Iq和Id得到 这里使用Id = 0控制法
当目标超前时 为了使


使用弱磁控制法

$$
V_d = L_d \cdot \frac{d(I_d)}{dt} + \omega \cdot L_q \cdot I_q - I_{d_{\text{limit}}}
$$

_____

动力学

对于滑轮 有以下方程

$$
\omega\ = \frac{d \theta}{dt}
$$

$$
\alpha\ = \frac{d \omega}{dt}
$$

对于滑块

$$
x = r \theta
$$

$$
v = \frac{dx}{dt} = r \omega
$$

$$
a = \frac{dv}{dt} = r \alpha
$$


定义滑轮半径为$ R $,电机扭矩为$ T $ 动摩擦力为$ F_d $
动滑块质量$M $, 转子角动量为$ J $

-----
加速


对于关节与滑块 有方程

$$
\frac{T}{R} - F_d = Ma + J\alpha
$$

$$
\frac{T}{R} - F_d = MR \alpha + J\alpha
$$

$$
T = (MR^{2} + JR)\alpha + F_d R
$$

let $c_1$ equals to$ MR^{2} + JR $
let $c_2$ equals to$ F_d R $

$$
T = c_1\alpha + F_d
$$

匀速

$$
T = F_d
$$

------

减速

$$
-\frac{T}{R} + F_d = Ma + J\alpha
$$


$$
-\frac{T}{R} + F_d = MR \alpha + J\alpha
$$


$$
T = c_1\alpha - F_dR
$$


停止预判距离$l$

分为两个阶段 匀变加速度阶段和匀变速阶段

加速度最大值为$a_m$ 速度最大值为$v_m$
另急动度为$j$

匀变加速度阶段



$$
t_0 = \frac{a_m}{j}
$$

$$
a = -jt
$$

$$
v = v_m + \int_{0}^{t}adt = v_m -  j\frac{t^{2}}{2}
$$

$$
l_0 = \int_{0}^{t_0}vdt = \int_{0}^{t_0}v_m -  j\frac{t^{2}}{2}dt = v_mt_0 - j\frac{t_0^{3}}{6}
$$

so 

$$
l_0 = \frac{a_mv_m}{j} - \frac{j}{6} \frac{a_m^{3}}{j^{3}} = \frac{a_mv_m}{j} - \frac{a_m^{3}}{6j^{2}}
$$

匀变速阶段


$$
l_1 = \frac{v_m ^ {2}}{2a_m}
$$


总位移为

$$
l = l_0 + l_1 = \frac{v_m ^ {2}}{2a_m} - \frac{a_mv_m}{j} - \frac{a_m^{3}}{6j^{2}}
$$


启动距离

$$
v = \int_{0}^{t}adt = j\frac{t^{2}}{2}
$$

$$
x_0 = \int_{0}^{t}vdt = j\frac{t^{2}}{2}dt = j\frac{t^{3}}{6}
$$

另$t_0 = \frac{a_m}{j}$
so $v_0 =  j\frac{t_0^{2}}{2} = \frac{a_m^{2}}{2j}$
so $x_0 = \frac{j}{6}\frac{a_m ^3}{j ^ 3} = \frac{a_m^3}{6j^2}$
在时间t大于于$t_0$时

$$
v = v_0 + \int_{t_0}^{t}a_mdt = v_0 + a_m(t - t_0)
$$


$$
x_1 = x_0 + \int_{t_0}^{t}vdt = x_0 + v_0(t - t_0) + \frac{a_m(t - t_0)^2}{2}
$$

$$
x_1 = x_0 + \frac{2v_0 + a_m(t - t_0)}{2}(t -t_0)
$$



let $t_1 = t_0 + \frac{v_m - v_0}{a_m} = \frac{a_m}{j} + \frac{v_m}{a_m} - \frac{a_m}{2j} = \frac{v_m}{a_m} + \frac{a_m}{2j} $

$$
x_2 = x_1 +  \int_{t_1}^{t}v_mdt = x_1 + v_m(t - t_1)
$$

-----

静止

$$
T = F_s
$$