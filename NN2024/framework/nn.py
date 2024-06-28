from abc import ABC, abstractmethod
from framework.autograd import Tensor
from framework import ops
import framework.init as init
from functools import reduce


class Parameter(Tensor):
    pass


def _unpack_params(value: object) -> list[Tensor]:
    if isinstance(value, Parameter):
        return [value]
    elif isinstance(value, Module):
        return value.parameters()
    elif isinstance(value, dict):
        params = []
        for k, v in value.items():
            params += _unpack_params(v)
        return params
    elif isinstance(value, (list, tuple)):
        params = []
        for v in value:
            params += _unpack_params(v)
        return params
    else:
        return []


def _child_modules(value: object) -> list["Module"]:
    if isinstance(value, Module):
        modules = [value]
        modules.extend(_child_modules(value.__dict__))
        return modules
    if isinstance(value, dict):
        modules = []
        for k, v in value.items():
            modules += _child_modules(v)
        return modules
    elif isinstance(value, (list, tuple)):
        modules = []
        for v in value:
            modules += _child_modules(v)
        return modules
    else:
        return []


class Module(ABC):
    def __init__(self):
        self.training = True

    def parameters(self) -> list[Tensor]:
        return _unpack_params(self.__dict__)

    def _children(self) -> list["Module"]:
        return _child_modules(self.__dict__)

    def eval(self):
        self.training = False
        for m in self._children():
            m.training = False

    def train(self):
        self.training = True
        for m in self._children():
            m.training = True

    @abstractmethod
    def forward(self) -> Tensor:
        pass

    def __call__(self, *args, **kwargs):
        return self.forward(*args, **kwargs)


class Identity(Module):
    def forward(self, x):
        return x


class Linear(Module):
    def __init__(self, in_features, out_features, bias=True):
        super().__init__()
        self.in_features = in_features
        self.out_features = out_features
        self.weight = Parameter(init.kaiming_uniform(in_features, out_features, requires_grad=True))
        self.bias = Parameter(init.kaiming_uniform(out_features, 1, requires_grad=True).transpose()) if bias else None

    def forward(self, X: Tensor) -> Tensor:
        out = X.matmul(self.weight)
        if self.bias:
            out += self.bias.broadcast_to(out.shape)
        return out


class Flatten(Module):
    def forward(self, X):
        size = reduce(lambda a, b: a * b, X.shape)
        return X.reshape((X.shape[0], size // X.shape[0]))


class ReLU(Module):
    def forward(self, x: Tensor) -> Tensor:
        return ops.relu(x)


class Sequential(Module):
    def __init__(self, *modules):
        super().__init__()
        self.modules = modules

    def forward(self, x: Tensor) -> Tensor:
        out = x
        for module in self.modules:
            out = module(out)
        return out


class SoftmaxLoss(Module):
    def forward(self, logits: Tensor, y: Tensor):
        one_hot_y = init.one_hot(logits.shape[1], y)
        return ops.summation(ops.logsumexp(logits, (1,)) / logits.shape[0]) - ops.summation(
            one_hot_y * logits / logits.shape[0]
        )


class BatchNorm1d(Module):
    def __init__(self, dim, eps=1e-5, momentum=0.1):
        super().__init__()
        self.dim = dim
        self.eps = eps
        self.momentum = momentum

        self.weight = Parameter(init.ones(dim, requires_grad=True))
        self.bias = Parameter(init.zeros(dim, requires_grad=True))
        self.running_mean = init.zeros(dim)
        self.running_var = init.ones(dim)

    def forward(self, x: Tensor) -> Tensor:
        if self.training:
            batch_mean = x.sum((0,)) / x.shape[0]
            batch_var = ((x - batch_mean.broadcast_to(x.shape)) ** 2).sum((0,)) / x.shape[0]
            self.running_mean = (1 - self.momentum) * self.running_mean + self.momentum * batch_mean.data
            self.running_var = (1 - self.momentum) * self.running_var + self.momentum * batch_var.data
            norm = (x - batch_mean.broadcast_to(x.shape)) / (batch_var.broadcast_to(x.shape) + self.eps) ** 0.5
            return self.weight.broadcast_to(x.shape) * norm + self.bias.broadcast_to(x.shape)
        else:
            norm = (x - self.running_mean.broadcast_to(x.shape)) / (
                self.running_var.broadcast_to(x.shape) + self.eps
            ) ** 0.5
            return self.weight.broadcast_to(x.shape) * norm + self.bias.broadcast_to(x.shape)


class LayerNorm1d(Module):
    def __init__(self, dim, eps=1e-5):
        super().__init__()
        self.dim = dim
        self.eps = eps
        self.weight = Parameter(init.ones(dim, requires_grad=True))
        self.bias = Parameter(init.zeros(dim, requires_grad=True))

    def forward(self, x: Tensor) -> Tensor:
        mean = (x.sum((1,)) / x.shape[1]).reshape((x.shape[0], 1)).broadcast_to(x.shape)
        var = (((x - mean) ** 2).sum((1,)) / x.shape[1]).reshape((x.shape[0], 1)).broadcast_to(x.shape)
        deno = (var + self.eps) ** 0.5
        return self.weight.broadcast_to(x.shape) * (x - mean) / deno + self.bias.broadcast_to(x.shape)


class Dropout(Module):
    def __init__(self, p=0.5):
        super().__init__()
        self.p = p

    def forward(self, x: Tensor) -> Tensor:
        if self.training:
            mask = init.randb(*x.shape, p=1 - self.p)
            return x * mask / (1 - self.p)
        else:
            return x


class Residual(Module):
    def __init__(self, fn: Module):
        super().__init__()
        self.fn = fn

    def forward(self, x: Tensor) -> Tensor:
        return x + self.fn(x)
