import numpy as np


class Transform:
    """
    Transform类用于对时间序列进行预处理，其需要实现transform(变换)和inverse_transform(逆变换)
    """

    def __init__(self):
        self.init = True

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
    def transform(self, data):
        if self.init:
            self.min = data.min()
            self.max = data.max()
            self.init = False
        return (data - self.min) / (self.max - self.min)

    def inverse_transform(self, data):
        return data * (self.max - self.min) + self.min


class Standardization(Transform):
    def transform(self, data):
        if self.init:
            self.mean = data.mean()
            self.std = data.std()
            self.init = False
        return (data - self.mean) / self.std

    def inverse_transform(self, data):
        return data * self.std + self.mean


class MeanNormalization(Transform):
    def transform(self, data):
        if self.init:
            self.min = data.min()
            self.max = data.max()
            self.mean = data.mean()
            self.init = False
        return (data - self.mean) / (self.max - self.min)

    def inverse_transform(self, data):
        return data * (self.max - self.min) + self.mean


class BoxCox(Transform):
    def __init__(self, l1: float, l2: float):
        self.lam1 = l1
        self.lam2 = l2

    def transform(self, data):
        if self.lam1 != 0:
            return (np.power((data + self.lam2), self.lam1) - 1) / self.lam1
        return np.log(data + self.lam2)

    def inverse_transform(self, data):
        if self.lam1 != 0:
            return np.power(data * self.lam1 + 1, 1 / self.lam1) - self.lam2
        return np.exp(data) - self.lam2
