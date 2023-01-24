import numpy as np
from math import inf
from tqdm import tqdm


class MLForecastModel:
    def __init__(self) -> None:
        super().__init__()
        self.fitted = False
        self.horizon = None

    def fit(self, X: np.ndarray, Y: np.ndarray) -> None:
        """
        :param X: 用于训练的历史序列集合
        :param Y: 历史序列对应的未来序列集合
        """
        self.horizon = Y.shape[1]
        self._fit(X, Y)
        self.fitted = True

    def _fit(self, X: np.ndarray, Y: np.ndarray):
        raise NotImplementedError

    def _forecast(self, X: np.ndarray) -> np.ndarray:
        raise NotImplementedError

    def forecast(self, X: np.ndarray) -> np.ndarray:
        """
        :param X: 输入的历史序列
        :return: 预测的未来序列值
        """
        if not self.fitted:
            raise ValueError("模型未训练")
        pred = self._forecast(X)
        return pred


class ZeroForecast(MLForecastModel):
    def _fit(self, X: np.ndarray, Y: np.ndarray) -> None:
        pass

    def _forecast(self, X: np.ndarray) -> np.ndarray:
        return np.zeros((X.shape[0], self.horizon))


class LinearForecast(MLForecastModel):
    def _fit(self, X: np.ndarray, Y: np.ndarray) -> None:
        xlen, ylen = X.shape[1], Y.shape[1]
        up = np.zeros((ylen, xlen))
        down = np.zeros((xlen, xlen))
        for x, y in zip(X, Y):
            xt = np.array([x])
            yt = np.array([y])
            up += np.matmul(np.transpose(yt), xt)
            down += np.matmul(np.transpose(xt), xt)
        self.W = np.matmul(up, np.linalg.inv(down))

    def _forecast(self, X: np.ndarray) -> np.ndarray:
        res = []
        for x in X:
            res.append([np.matmul(self.W, x)])
        return np.concatenate(res)


class PowerForecast(MLForecastModel):
    def forward(self, lam, x) -> np.ndarray:
        pred = np.empty((len(x) + self.horizon,))
        pred[0] = x[0]
        for i in range(1, len(x)):
            pred[i] = (1 - lam) * x[i] + lam * pred[i - 1]
        pred[len(x) :] = np.ones((self.horizon,)) * pred[len(x) - 1]
        return pred

    def _fit(self, X: np.ndarray, Y: np.ndarray) -> None:
        min_lam, min_mse = 0, inf
        for lam in tqdm(np.arange(100) / 100.0):
            loss = 0
            for x, y in zip(X, Y):
                y_pred = self.forward(lam, x)
                y_true = np.concatenate((x, y), axis=0)
                loss += np.mean((y_pred - y_true) ** 2)
            if loss < min_mse:
                min_lam = lam
                min_mse = loss
        print(f"best lambda: {min_lam}")
        self.lam = min_lam

    def _forecast(self, X: np.ndarray) -> np.ndarray:
        res = np.empty((len(X), self.horizon))
        for i, x in enumerate(X):
            res[i] = self.forward(self.lam, x)[-self.horizon :]
        return res
