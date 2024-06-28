from framework.data import MNISTDataset, DataLoader, FashionMNISTDataset, IrisDataset
from framework.optim import SGD, Adam
from framework import nn
import numpy as np
from tqdm import tqdm
import argparse


def ResidualBlock(dim, hidden_dim, norm=nn.BatchNorm1d, drop_prob=0.1):
    main_path = nn.Sequential(
        nn.Linear(dim, hidden_dim),
        norm(hidden_dim),
        nn.ReLU(),
        nn.Dropout(drop_prob),
        nn.Linear(hidden_dim, dim),
        norm(dim),
    )
    res = nn.Residual(main_path)
    return nn.Sequential(res, nn.ReLU())


def ResNet(dim, hidden_dim=100, num_blocks=3, num_classes=10, norm=nn.BatchNorm1d, drop_prob=0.1):
    resnet = nn.Sequential(
        nn.Linear(dim, hidden_dim),
        nn.ReLU(),
        *[
            ResidualBlock(dim=hidden_dim, hidden_dim=hidden_dim // 2, norm=norm, drop_prob=drop_prob)
            for _ in range(num_blocks)
        ],
        nn.Linear(hidden_dim, num_classes),
    )
    return resnet


def MLP(dim, hidden_dim, num_classes, layers, drop_prob=0.1):
    hidden_layer_factory = lambda: [nn.Linear(hidden_dim, hidden_dim), nn.ReLU(), nn.Dropout(drop_prob)]
    return nn.Sequential(
        nn.Linear(dim, hidden_dim),
        nn.ReLU(),
        nn.Dropout(drop_prob),
        *sum([hidden_layer_factory() for _ in range(layers - 2)], start=[]),
        nn.Linear(hidden_dim, num_classes),
    )


def train_eval_epoch(dataloader, model, opt=None):
    tot_loss, tot_error = [], 0.0
    loss_fn = nn.SoftmaxLoss()
    if opt is None:
        model.eval()
        for X, y in dataloader:
            X = X.reshape((X.shape[0], -1))
            logits = model(X)
            loss = loss_fn(logits, y)
            tot_error += np.sum(logits.numpy().argmax(axis=1) != y.numpy())
            tot_loss.append(loss.numpy())
    else:
        model.train()
        for X, y in dataloader:
            X = X.reshape((X.shape[0], -1))
            logits = model(X)
            loss = loss_fn(logits, y)
            tot_error += np.sum(logits.numpy().argmax(axis=1) != y.numpy())
            tot_loss.append(loss.numpy())
            opt.zero_grad()
            loss.backward()
            opt.step()
    sample_nums = len(dataloader.dataset)
    return tot_error / sample_nums, np.mean(tot_loss)


def run_mnist(
    batch_size=64,
    epochs=10,
    optimizer=SGD,
    lr=0.001,
    weight_decay=0.001,
    hidden_dim=64,
):
    data_dir = "data/MNIST"
    resnet = ResNet(28 * 28, hidden_dim=hidden_dim, num_classes=10)
    opt = optimizer(resnet.parameters(), lr=lr, weight_decay=weight_decay)
    train_set = MNISTDataset(f"{data_dir}/train-images-idx3-ubyte.gz", f"{data_dir}/train-labels-idx1-ubyte.gz")
    test_set = MNISTDataset(f"{data_dir}/t10k-images-idx3-ubyte.gz", f"{data_dir}/t10k-labels-idx1-ubyte.gz")
    train_loader = DataLoader(train_set, batch_size=batch_size, shuffle=True)
    test_loader = DataLoader(test_set, batch_size=batch_size)

    for _ in tqdm(range(epochs)):
        train_err, train_loss = train_eval_epoch(train_loader, resnet, opt)
        tqdm.write(f"Training Error: {train_err}, Training Loss: {train_loss}")

    test_err, test_loss = train_eval_epoch(test_loader, resnet, None)
    tqdm.write(f"Test Error: {test_err}, Test Loss: {test_loss}")
    return train_err, train_loss, test_err, test_loss


def run_fashion_mnist(
    batch_size=64,
    epochs=10,
    optimizer=SGD,
    lr=0.001,
    weight_decay=0.001,
    hidden_dim=64,
):
    data_dir = "data/FashionMNIST"
    resnet = ResNet(28 * 28, hidden_dim=hidden_dim, num_classes=10)
    opt = optimizer(resnet.parameters(), lr=lr, weight_decay=weight_decay)
    train_set = FashionMNISTDataset(f"{data_dir}/train-images-idx3-ubyte.gz", f"{data_dir}/train-labels-idx1-ubyte.gz")
    test_set = FashionMNISTDataset(f"{data_dir}/t10k-images-idx3-ubyte.gz", f"{data_dir}/t10k-labels-idx1-ubyte.gz")
    train_loader = DataLoader(train_set, batch_size=batch_size, shuffle=True)
    test_loader = DataLoader(test_set, batch_size=batch_size)

    for _ in tqdm(range(epochs)):
        train_err, train_loss = train_eval_epoch(train_loader, resnet, opt)
        tqdm.write(f"Training Error: {train_err}, Training Loss: {train_loss}")

    test_err, test_loss = train_eval_epoch(test_loader, resnet, None)
    tqdm.write(f"Test Error: {test_err}, Test Loss: {test_loss}")
    return train_err, train_loss, test_err, test_loss


def run_iris(
    batch_size=16,
    epochs=10,
    optimizer=Adam,
    lr=0.001,
    weight_decay=0.001,
    hidden_dim=256,
):
    data_dir = "data/IRIS"
    mlp = MLP(4, hidden_dim=hidden_dim, num_classes=3, layers=4)
    opt = optimizer(mlp.parameters(), lr=lr, weight_decay=weight_decay)
    train_set = IrisDataset(f"{data_dir}/train.csv")
    test_set = IrisDataset(f"{data_dir}/test.csv")
    train_loader = DataLoader(train_set, batch_size=batch_size, shuffle=True)
    test_loader = DataLoader(test_set, batch_size=batch_size)

    for _ in tqdm(range(epochs)):
        train_err, train_loss = train_eval_epoch(train_loader, mlp, opt)
        tqdm.write(f"Training Error: {train_err}, Training Loss: {train_loss}")

    test_err, test_loss = train_eval_epoch(test_loader, mlp, None)
    tqdm.write(f"Test Error: {test_err}, Test Loss: {test_loss}")
    return train_err, train_loss, test_err, test_loss


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--dataset", default="mnist")
    args = parser.parse_args()
    if args.dataset == "mnist":
        run_mnist()
    elif args.dataset == "fashion_mnist":
        run_fashion_mnist()
    elif args.dataset == "iris":
        run_iris()
    else:
        raise NotImplementedError()


if __name__ == "__main__":
    main()
