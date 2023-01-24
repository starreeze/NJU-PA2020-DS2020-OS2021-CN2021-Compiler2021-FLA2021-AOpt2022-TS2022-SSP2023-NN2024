import pandas as pd
import numpy as np
import sys
from models import *
from transforms import *
from metrics import *
from matplotlib import pyplot as plt


def train(train_data: pd.Series, transform: Transform, model: ForecastModel):
    t_data = transform.transform(train_data)
    model.fit(t_data)
    return model


def test(
    full_data: pd.Series,
    test_data: pd.Series,
    transform: Transform,
    model: ForecastModel,
):
    fore = model.forecast(len(test_data))
    # 将预测做逆变换
    fore = transform.inverse_transform(fore)
    # 计算各个指标上的性能
    print(mse(fore, full_data))
    print(mae(fore, full_data))
    print(mape(fore, full_data))
    print(smape(fore, full_data))
    print(mase(fore, full_data))
    print("------------------")
    return fore


def main():
    # 读取ETTh1数据
    ETTh1_data = pd.read_csv("data/ETTh1.csv")
    ETTh1_data.set_index("date", inplace=True)
    ETTh1_data.index = pd.DatetimeIndex(ETTh1_data.index, freq="infer")
    OT_data = ETTh1_data["OT"]
    # 划分训练集和测试集
    split = 16 * 30 * 24
    train_OT_data, test_OT_data = OT_data[:split], OT_data[split:]

    models = [Naive1Forecast(0.5), NaiveSForecast(), DriftForecast()]
    transforms = [
        IdentityTransform(),
        Normalization(),
        Standardization(),
        MeanNormalization(),
        BoxCox(0.5, 0.5),
    ]

    for model in models:
        print("model:", type(model))
        for i, trans in enumerate(transforms):
            print("transform:", type(trans))
            fitted_model = train(train_OT_data, trans, model)
            forecast = test(OT_data, test_OT_data, trans, fitted_model)
            if i == 0:  # identity transform
                fig = plt.figure()
                train_OT_data.plot(label="train")
                test_OT_data.plot(label="true")
                forecast.plot(label="predict")
                plt.legend()
                plt.show()
                plt.close(fig)


def calc_mse(
    full_data: pd.Series,
    test_data: pd.Series,
    transform: Transform,
    model: ForecastModel,
):
    fore = model.forecast(len(test_data))
    fore = transform.inverse_transform(fore)
    return mse(fore, full_data)


def addtional():
    periods_interval = 12
    periods_range = range(100)
    periods = [(i + 1) * periods_interval for i in periods_range]
    ETTh1_data = pd.read_csv("data/ETTh1.csv")
    ETTh1_data.set_index("date", inplace=True)
    ETTh1_data.index = pd.DatetimeIndex(ETTh1_data.index, freq="infer")
    OT_data = ETTh1_data["OT"]
    split = 16 * 30 * 24
    train_OT_data, test_OT_data = OT_data[:split], OT_data[split:]

    mses = []
    for period in periods:
        model = NaiveSForecast(period)
        trans = IdentityTransform()
        fitted_model = train(train_OT_data, trans, model)
        forecast = calc_mse(OT_data, test_OT_data, trans, fitted_model)
        mses.append(forecast)
    plt.plot(periods, mses)
    plt.show()


if __name__ == "__main__":
    main()
    if len(sys.argv) > 1 and sys.argv[1] == "-a":
        addtional()
