import numpy as np
import pandas as pd
from utils import build_forecast_series


class ForecastModel:
    def __init__(self) -> None:
        super().__init__()
        self.fitted = False
        self.train_series = None

    def fit(self, X: pd.Series) -> None:
        """
        :param X: 用于训练的时间序列, 长度为t
        """
        self.train_series = X
        self._fit(X)
        self.fitted = True

    def _fit(self, X: pd.Series):
        raise NotImplementedError

    def _forecast(self, horizon: int) -> np.ndarray:
        raise NotImplementedError

    def forecast(self, horizon: int) -> pd.Series:
        """
        :param horizon: 预测序列长度
        :return: 预测的未来序列值，长度为horizon
        """
        if not self.fitted:
            raise ValueError("模型未训练")
        pred = self._forecast(horizon)
        return build_forecast_series(pred, self.train_series)


class ZeroForecast(ForecastModel):
    def _fit(self, X: pd.Series) -> None:
        pass

    def _forecast(self, horizon: int) -> np.ndarray:
        return np.zeros((horizon,))


class Naive1Forecast(ForecastModel):
    def __init__(self, std=1.0) -> None:
        super().__init__()
        self.std = std

    def _fit(self, X: pd.Series) -> None:
        self.last = X.array[-1]

    def _forecast(self, horizon: int) -> np.ndarray:
        return np.ones((horizon,)) * self.last


class NaiveSForecast(ForecastModel):
    def __init__(self, gran=24) -> None:
        super().__init__()
        self.gran = gran

    def _fit(self, X: pd.Series) -> None:
        self.X = X.array

    def _forecast(self, horizon: int) -> np.ndarray:
        res = np.zeros((horizon,))
        for h in range(horizon):
            n = len(self.X)
            res[h] = self.X[n + h - self.gran * (h // self.gran + 1)]
        return res


class DriftForecast(ForecastModel):
    def _fit(self, X: pd.Series) -> None:
        self.X = X.array

    def _forecast(self, horizon: int) -> np.ndarray:
        res = np.zeros((horizon,))
        for h in range(horizon):
            res[h] = self.X[-1] + h * (self.X[-1] - self.X[0]) / (len(self.X) - 1)
        return res