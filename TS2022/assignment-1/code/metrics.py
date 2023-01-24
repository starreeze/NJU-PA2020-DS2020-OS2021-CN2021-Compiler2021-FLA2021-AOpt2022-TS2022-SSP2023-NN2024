from utils import get_intersection_values
import numpy as np


def mse(predict, target):
    target_values, predict_values = get_intersection_values(target, predict)
    return np.mean((target_values - predict_values) ** 2)


def mae(predict, target):
    target_values, predict_values = get_intersection_values(target, predict)
    return np.mean(np.abs(target_values - predict_values))


def mape(predict, target):
    target_values, predict_values = get_intersection_values(target, predict)
    f_filter = lambda idx: abs(target_values[idx]) > 1e-3
    legal_idx = list(filter(f_filter, range(len(predict_values))))
    target_values = target_values[legal_idx]
    predict_values = predict_values[legal_idx]
    return (
        np.sum(np.abs(target_values - predict_values) / abs(target_values))
        / len(target_values)
        * 100
    )


def smape(predict, target):
    target_values, predict_values = get_intersection_values(target, predict)
    return (
        np.sum(
            np.abs(target_values - predict_values)
            / (abs(target_values) + abs(predict_values))
        )
        / len(target_values)
        * 200
    )


def mase(predict, target):
    x, y = get_intersection_values(target, predict)
    target = target.array
    m = 24
    h = len(x)
    t = len(target) - h
    tmp = 0
    for j in range(m, t + h):
        tmp += abs(target[j] - target[j - m])
    return np.sum(np.abs(y - x)) / tmp * (t + h - m) / h
