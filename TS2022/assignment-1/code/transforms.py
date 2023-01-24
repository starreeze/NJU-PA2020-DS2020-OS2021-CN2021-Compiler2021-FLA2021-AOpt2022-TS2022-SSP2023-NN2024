import numpy as np
import pandas as pd


class Transform:
    """
    Transform类用于对时间序列进行预处理，其需要实现transform(变换)和inverse_transform(逆变换)
    """

    def transform(self, data):
        """
        :param data: 时间序列
        :return: 变换后的时间序列
        """
        raise NotImplementedError

    def inverse_transform(self, data):
        """
        :param data: 时间序列
        :return: 逆变换后的时间序列
        """
        raise NotImplementedError


class IdentityTransform(Transform):
    def transform(self, data):
        return data

    def inverse_transform(self, data):
        return data


class Normalization(Transform):
    def transform(self, data: pd.Series):
        self.min = data.min()
        self.max = data.max()
        return (data - data.min()) / (data.max() - data.min())

    def inverse_transform(self, data: pd.Series):
        return data * (self.max - self.min) + self.min


class Standardization(Transform):
    def transform(self, data: pd.Series):
        self.mean = data.mean()
        self.std = data.std()
        return (data - data.mean()) / data.std()

    def inverse_transform(self, data: pd.Series):
        return data * self.std + self.mean


class MeanNormalization(Transform):
    def transform(self, data: pd.Series):
        self.min = data.min()
        self.max = data.max()
        self.mean = data.mean()
        return (data - data.mean()) / (data.max() - data.min())

    def inverse_transform(self, data: pd.Series):
        return data * (self.max - self.min) + self.mean


class BoxCox(Transform):
    def __init__(self, l1: float, l2: float):
        self.lam1 = l1
        self.lam2 = l2

    def transform(self, data: pd.Series):
        if self.lam1 != 0:
            return ((data + self.lam2).pow(self.lam1) - 1) / self.lam1
        return (data + self.lam2).transform(np.math.log)

    def inverse_transform(self, data: pd.Series):
        if self.lam1 != 0:
            return (data * self.lam1 + 1).pow(1 / self.lam1) - self.lam2
        return data.transform(np.math.exp) - self.lam2
