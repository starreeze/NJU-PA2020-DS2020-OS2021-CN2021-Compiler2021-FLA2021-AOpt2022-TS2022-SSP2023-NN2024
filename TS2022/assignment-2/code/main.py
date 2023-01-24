import pandas as pd
import numpy as np
from numpy.lib.stride_tricks import sliding_window_view
from models import *
from transforms import *
from metrics import *


def train(
    train_X: np.ndarray,
    train_Y: np.ndarray,
    transform: Transform,
    model: MLForecastModel,
):
    t_X = transform.transform(train_X)
    t_Y = transform.transform(train_Y)
    model.fit(t_X, t_Y)
    return model


def test(
    test_X: np.ndarray, test_Y: np.ndarray, transform: Transform, model: MLForecastModel
):
    test_X = transform.transform(test_X)
    fore = model.forecast(test_X)
    fore = transform.inverse_transform(fore)
    target = np.concatenate((test_X, test_Y), axis=-1)
    print("mse\t=", mse(fore, target))
    print("mae\t=", mae(fore, target))
    print("mape\t=", mape(fore, target))
    print("smape\t=", smape(fore, target))
    print("mase\t=", mase(fore, target))
    print("------------------")
    return fore


def create_sub_series(series: np.ndarray, window_len: int, horizon: int):
    subseries = sliding_window_view(series, window_len + horizon)
    return subseries[:, :window_len], subseries[:, window_len:]


def main():
    # 读取ETTh1数据
    ETTh1_data = pd.read_csv("data/ETTh1.csv")
    ETTh1_data.set_index("date", inplace=True)
    ETTh1_data.index = pd.DatetimeIndex(ETTh1_data.index, freq="infer")
    OT_data = ETTh1_data["OT"]
    # 划分训练集和测试集
    split = 16 * 30 * 24
    train_OT_data, test_OT_data = OT_data[:split], OT_data[split:]
    # 从长序列中构造子序列集合, L历史序列长度,H预测序列长度
    L, H = 96, 32
    train_X, train_Y = create_sub_series(train_OT_data.values, L, H)
    test_X, test_Y = create_sub_series(test_OT_data.values, L, H)

    models = [LinearForecast(), PowerForecast()]
    transforms = [
        IdentityTransform(),
        Normalization(),
        Standardization(),
        MeanNormalization(),
        BoxCox(0.5, 5),
    ]

    for model in models:
        print("model:", type(model))
        for trans in transforms:
            print("transform:", type(trans))
            fitted_model = train(train_X, train_Y, trans, model)
            test(test_X, test_Y, trans, fitted_model)


if __name__ == "__main__":
    main()
