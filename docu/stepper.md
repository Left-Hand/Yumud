��̻�����ѧ���ᶯ̬����

____
����ѧ

ת�ؿ���
���е�Ƕ�Ϊ$theta$
�����Ϊn
���Ƕ�$\theta_e = n \theta + \theta_{f}$ ����$\theta_{f}$Ϊ��е�Ƕȵ���ǶȵĲ�����

$$T_m = K I$$

SVPWM���Ƶõ�Ƕ�$\theta_{w}$
���ڵ�ǶȲ�ֵ$\theta_{ee} = \theta_{w} - \theta_e$�����$[-\pi, \pi]$ʱ ��

$$T = T_m sin(\theta_ee) = K I sin(\theta_{w} - \theta_e)$$
$$T = T_m sin(\theta_ee) = K I (sin(\theta_{w})cos(\theta_e) - cos(\theta_w)sin(\theta_e))$$



���в��Եõ��ÿ�����Ŀ��ת��T �۲��theta_e �ɵó����Ƶĵ����ͽǶ�


����IΪ������Ƶõ��� ��Iq��Id�õ� ����ʹ��Id = 0���Ʒ�
��Ŀ�곬ǰʱ Ϊ��ʹ


ʹ�����ſ��Ʒ�

$$
V_d = L_d \cdot \frac{d(I_d)}{dt} + \omega \cdot L_q \cdot I_q - I_{d_{\text{limit}}}
$$

_____

����ѧ

���ڻ��� �����·���

$$
\omega\ = \frac{d \theta}{dt}
$$

$$
\alpha\ = \frac{d \omega}{dt}
$$

���ڻ���

$$
x = r \theta
$$

$$
v = \frac{dx}{dt} = r \omega
$$

$$
a = \frac{dv}{dt} = r \alpha
$$


���廬�ְ뾶Ϊ$ R $,���Ť��Ϊ$ T $ ��Ħ����Ϊ$ F_d $
����������$M $, ת�ӽǶ���Ϊ$ J $

-----
����


���ڹؽ��뻬�� �з���

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

����

$$
T = F_d
$$

------

����

$$
-\frac{T}{R} + F_d = Ma + J\alpha
$$


$$
-\frac{T}{R} + F_d = MR \alpha + J\alpha
$$


$$
T = c_1\alpha - F_dR
$$


ֹͣԤ�о���$l$

��Ϊ�����׶� �ȱ���ٶȽ׶κ��ȱ��ٽ׶�

���ٶ����ֵΪ$a_m$ �ٶ����ֵΪ$v_m$
������Ϊ$j$

�ȱ���ٶȽ׶�



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

�ȱ��ٽ׶�


$$
l_1 = \frac{v_m ^ {2}}{2a_m}
$$


��λ��Ϊ

$$
l = l_0 + l_1 = \frac{v_m ^ {2}}{2a_m} - \frac{a_mv_m}{j} - \frac{a_m^{3}}{6j^{2}}
$$


��������

$$
v = \int_{0}^{t}adt = j\frac{t^{2}}{2}
$$

$$
x_0 = \int_{0}^{t}vdt = j\frac{t^{2}}{2}dt = j\frac{t^{3}}{6}
$$

��$t_0 = \frac{a_m}{j}$
so $v_0 =  j\frac{t_0^{2}}{2} = \frac{a_m^{2}}{2j}$
so $x_0 = \frac{j}{6}\frac{a_m ^3}{j ^ 3} = \frac{a_m^3}{6j^2}$
��ʱ��t������$t_0$ʱ

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

��ֹ

$$
T = F_s
$$