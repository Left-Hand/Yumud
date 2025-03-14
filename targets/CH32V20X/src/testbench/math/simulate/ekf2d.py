import math
import matplotlib
import matplotlib.pyplot as plt
import numpy as np
from scipy.spatial.transform import Rotation as Rot
from typing import Tuple

# Covariance for EKF simulation
R: np.ndarray = np.diag([
    0.1,  # variance of location on x-axis
    0.1,  # variance of location on y-axis
    np.deg2rad(1.0),  # variance of yaw angle
    1.0  # variance of velocity
]) ** 2  # predict state covariance

Q: np.ndarray = np.diag([1.0, 1.0]) ** 2  # Observation x,y position covariance

# Simulation parameters
INPUT_NOISE: np.ndarray = np.diag([1.0, np.deg2rad(30.0)]) ** 2
GPS_NOISE: np.ndarray = np.diag([0.5, 0.5]) ** 2

dt: float = 0.1  # time tick [s]
SIM_TIME: float = 50.0  # simulation time [s]

show_animation: bool = True


def calc_input() -> np.ndarray:
    v: float = 1.0  # [m/s]
    yawrate: float = 0.1  # [rad/s]
    u: np.ndarray = np.float32([[v], [yawrate]])
    return u


def observation(
    xTrue: np.ndarray,
    xd: np.ndarray,
    u: np.ndarray
) -> Tuple[np.ndarray, np.ndarray, np.ndarray, np.ndarray]:
    xTrue = motion_model(xTrue, u)
    z: np.ndarray = observation_model(xTrue) + GPS_NOISE @ np.random.randn(2, 1)

    ud: np.ndarray = u + INPUT_NOISE @ np.random.randn(2, 1)
    xd = motion_model(xd, ud)

    return xTrue, z, xd, ud


def motion_model(x: np.ndarray, u: np.ndarray) -> np.ndarray:
    A: np.ndarray = np.float32([
        [1, 0, 0, 0],
        [0, 1, 0, 0],
        [0, 0, 1, 0],
        [0, 0, 0, 0]
    ])

    B: np.ndarray = np.float32([
        [dt * math.cos(x[2, 0]), 0],
        [dt * math.sin(x[2, 0]), 0],
        [0.0, dt],
        [1.0, 0.0]
    ])

    return A @ x + B @ u


def observation_model(x: np.ndarray) -> np.ndarray:
    C: np.ndarray = np.float32([
        [1, 0, 0, 0],
        [0, 1, 0, 0]
    ])
    return C @ x


def jacob_f(x: np.ndarray, u: np.ndarray) -> np.ndarray:
    yaw: float = x[2, 0]
    v: float = u[0, 0]
    G: np.ndarray = np.float32([
        [1.0, 0.0, -dt * v * math.sin(yaw), dt * math.cos(yaw)],
        [0.0, 1.0, dt * v * math.cos(yaw), dt * math.sin(yaw)],
        [0.0, 0.0, 1.0, 0.0],
        [0.0, 0.0, 0.0, 1.0]
    ])
    return G


def jacob_h() -> np.ndarray:
    C: np.ndarray = np.float32([
        [1, 0, 0, 0],
        [0, 1, 0, 0]
    ])
    return C


def ekf_estimation(
    xEst: np.ndarray,
    PEst: np.ndarray,
    z: np.ndarray,
    u: np.ndarray
) -> Tuple[np.ndarray, np.ndarray]:
    xPred: np.ndarray = motion_model(xEst, u)
    G: np.ndarray = jacob_f(xEst, u)
    PPred: np.ndarray = G @ PEst @ G.T + R

    C: np.ndarray = jacob_h()
    zPred: np.ndarray = observation_model(xPred)
    S: np.ndarray = C @ PPred @ C.T + Q
    K: np.ndarray = PPred @ C.T @ np.linalg.inv(S)
    xEst = xPred + K @ (z - zPred)
    PEst = (np.eye(len(xEst)) - K @ C) @ PPred
    return xEst, PEst


def plot_covariance_ellipse(xEst: np.ndarray, PEst: np.ndarray) -> None:
    Pxy: np.ndarray = PEst[0:2, 0:2]
    eigval: np.ndarray
    eigvec: np.ndarray
    eigval, eigvec = np.linalg.eig(Pxy)

    bigind: int
    smallind: int
    if eigval[0] >= eigval[1]:
        bigind = 0
        smallind = 1
    else:
        bigind = 1
        smallind = 0

    t: np.ndarray = np.arange(0, 2 * math.pi + 0.1, 0.1)
    a: float = math.sqrt(eigval[bigind])
    b: float = math.sqrt(eigval[smallind])
    x_list: list[float] = [a * math.cos(it) for it in t]
    y_list: list[float] = [b * math.sin(it) for it in t]
    angle: float = math.atan2(eigvec[1, bigind], eigvec[0, bigind])
    rot: np.ndarray = Rot.from_euler('z', angle).as_matrix()[0:2, 0:2]
    fx: np.ndarray = rot @ (np.float32([x_list, y_list]))
    px: np.ndarray = np.float32(fx[0, :] + xEst[0, 0]).flatten()
    py: np.ndarray = np.float32(fx[1, :] + xEst[1, 0]).flatten()
    plt.plot(px, py, "--r")


if __name__ == '__main__':
    time: float = 0.0

    # State Vector [x y yaw v]'
    xEst: np.ndarray = np.zeros((4, 1))
    xTrue: np.ndarray = np.zeros((4, 1))
    PEst: np.ndarray = np.eye(4)

    xDR: np.ndarray = np.zeros((4, 1))  # Dead reckoning

    # History
    hxEst: np.ndarray = xEst.copy()
    hxTrue: np.ndarray = xTrue.copy()
    hxDR: np.ndarray = xDR.copy()
    hz: np.ndarray = np.zeros((2, 1))  # Initialize z

    while SIM_TIME >= time:
        time += dt
        u: np.ndarray = calc_input()

        xTrue, z, xDR, ud = observation(xTrue, xDR, u)

        xEst, PEst = ekf_estimation(xEst, PEst, z, ud)

        # Store data history
        hxEst = np.c_[hxEst, xEst]
        hxDR = np.c_[hxDR, xDR]
        hxTrue = np.c_[hxTrue, xTrue]
        hz = np.c_[hz, z]

        if show_animation:
            plt.cla()
            plt.gcf().canvas.mpl_connect(
                'key_release_event',
                lambda event: [exit(0) if event.key == 'escape' else None]
            )
            plt.plot(hz[0, :], hz[1, :], ".g")
            plt.plot(hxTrue[0, :], hxTrue[1, :], "-b")
            plt.plot(hxDR[0, :], hxDR[1, :], "-k")
            plt.plot(hxEst[0, :], hxEst[1, :], "-r")
            plot_covariance_ellipse(xEst, PEst)
            plt.axis("equal")
            plt.grid(True)
            plt.pause(0.001)  # 确保图像更新

    plt.ioff()  # 关闭交互模式后显示最终结果
    plt.show()