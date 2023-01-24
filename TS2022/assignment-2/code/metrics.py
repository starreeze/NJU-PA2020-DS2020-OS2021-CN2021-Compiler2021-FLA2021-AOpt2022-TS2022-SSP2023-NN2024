import numpy as np


def get_intersection(target, predict):
    return target[:, -predict.shape[-1] :], predict


def mse(predict, target):
    target, predict = get_intersection(target, predict)
    return np.mean((target - predict) ** 2)


def mae(predict, target):
    target, predict = get_intersection(target, predict)
    return np.mean(np.abs(target - predict))


def mape(predict, target):
    target, predict = get_intersection(target, predict)
    res = 0
    for t, p in zip(target, predict):
        f_filter = lambda idx: abs(t[idx]) > 1e-3
        legal_idx = list(filter(f_filter, range(len(p))))
        t = t[legal_idx]
        p = p[legal_idx]
        res += np.sum(np.abs(t - p) / abs(t)) / len(t) * 100
    return res / len(target)


def smape(predict, target):
    target, predict = get_intersection(target, predict)
    return np.mean(
        np.sum(np.abs(target - predict) / (abs(target) + abs(predict)), axis=-1)
        / target.shape[-1]
        * 200
    )


def mase(predict, target):
    x, y = get_intersection(target, predict)
    m = 24
    h = x.shape[-1]
    t = target.shape[-1] - h
    tmp = np.zeros((x.shape[0],))
    for j in range(m, t + h):
        tmp += abs(target[:, j] - target[:, j - m])
    return np.mean(np.sum(np.abs(y - x), axis=-1) / tmp) * (t + h - m) / h
