from abc import ABC, abstractmethod
import gzip
import struct
from typing import Optional, Iterable
import numpy as np
from .autograd import Tensor


class Dataset(ABC):
    def __init__(self, transforms: Optional[list] = None):
        self.transforms = transforms
        self.features: Iterable
        self.labels: Iterable

    @abstractmethod
    def __getitem__(self, index) -> object:
        raise NotImplementedError

    @abstractmethod
    def __len__(self) -> int:
        raise NotImplementedError

    def apply_transforms(self, x):
        if self.transforms is not None:
            for tform in self.transforms:
                x = tform(x)
        return x


class DataLoader:
    def __init__(self, dataset, batch_size=1, shuffle=False):
        self.dataset = dataset
        self.batch_size = batch_size
        self.shuffle = shuffle
        self.indices = np.arange(len(dataset))
        if self.shuffle:
            np.random.shuffle(self.indices)

    def __iter__(self):
        self.idx = 0
        return self

    def __next__(self):
        if self.idx >= len(self.dataset):
            raise StopIteration

        end_idx = min(self.idx + self.batch_size, len(self.dataset))
        batch_indices = self.indices[self.idx : end_idx]
        batch = [self.dataset[i] for i in batch_indices]
        batch_features = np.stack([x[0] for x in batch])
        batch_labels = np.array([x[1] for x in batch])
        self.idx += self.batch_size

        return Tensor(batch_features), Tensor(batch_labels)


class MNISTDataset(Dataset):
    def __init__(self, image_filename: str, label_filename: str, transforms: Optional[list] = None):
        super().__init__(transforms)
        with gzip.open(image_filename, "rb") as f:
            magic, size, rows, cols = struct.unpack(">IIII", f.read(16))
            self.features = np.frombuffer(f.read(), dtype=np.uint8).reshape(size, rows, cols, 1).astype(np.float32)
            self.features /= 255.0

        with gzip.open(label_filename, "rb") as f:
            magic, size = struct.unpack(">II", f.read(8))
            self.labels = np.frombuffer(f.read(), dtype=np.uint8).reshape(-1)

    def __len__(self):
        return len(self.features)

    def __getitem__(self, index):
        img = self.features[index]
        if self.transforms:
            img = self.apply_transforms(img)
        return img, self.labels[index]


FashionMNISTDataset = MNISTDataset


class NDArrayDataset(Dataset):
    def __init__(self, data):
        self.data = data

    def __len__(self):
        return len(self.data)

    def __getitem__(self, index):
        return self.data[index], None


class IrisDataset(Dataset):
    def __init__(self, file_path: str, transforms: Optional[list] = None):
        super().__init__(transforms)
        data = np.loadtxt(file_path, delimiter=",", dtype=np.float32)
        self.features = data[:, :-1]
        self.labels = data[:, -1].astype(np.int32)

    def __len__(self):
        return len(self.features)

    def __getitem__(self, index):
        feature = self.features[index]
        if self.transforms:
            feature = self.apply_transforms(feature)
        return feature, self.labels[index]
