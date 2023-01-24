import pandas as pd
from typing import Union, Tuple
import numpy as np


def get_intersection_values(
        series_a: pd.Series,
        series_b: pd.Series,
) -> Tuple[np.ndarray, np.ndarray]:
    """
    获取两个序列相交部分的值
    """
    index = series_a.index.intersection(series_b.index)
    series_a_common = series_a[index]
    series_b_common = series_b[index]

    return series_a_common.values, series_b_common.values


def build_forecast_series(
        points_preds: np.ndarray,
        input_series: pd.Series,
) -> pd.Series:
    """
    将给定预测包装为时间序列
    """
    time_index_length = len(points_preds)
    time_index = generate_new_dates(time_index_length, input_series=input_series)

    return pd.Series(data=points_preds, index=time_index)


def generate_new_dates(
        n: int, input_series: pd.Series
) -> Union[pd.DatetimeIndex, pd.RangeIndex]:
    """
    在input_series之后生成长度为n的时间索引
    """
    last = input_series.index[-1]
    freq = input_series.index.freq
    name = input_series.index.name
    start = pd.Timestamp(last) + freq if isinstance(input_series.index,
                                                    pd.DatetimeIndex) else last + 1

    if isinstance(start, pd.Timestamp):
        index = pd.date_range(
            start=start, periods=n, freq=freq, name=name
        )
    else:  # int
        index = pd.RangeIndex(
            start=start,
            stop=start + n,
            step=1,
            name=name,
        )
    return index
