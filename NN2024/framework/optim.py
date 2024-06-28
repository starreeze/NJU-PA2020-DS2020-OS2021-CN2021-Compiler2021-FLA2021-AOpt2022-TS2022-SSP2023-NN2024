from abc import ABC, abstractmethod
from collections import defaultdict


class Optimizer(ABC):
    def __init__(self, params):
        self.params = params

    @abstractmethod
    def step(self):
        raise NotImplementedError()

    def zero_grad(self):
        for p in self.params:
            p.grad = None


class SGD(Optimizer):
    def __init__(self, params, lr=0.01, momentum=0.0, weight_decay=0.0):
        super().__init__(params)
        self.lr = lr
        self.momentum = momentum
        self.u = defaultdict(float)
        self.weight_decay = weight_decay

    def step(self):
        for w in self.params:
            if self.weight_decay > 0:
                grad = w.grad.data + self.weight_decay * w.data
            else:
                grad = w.grad.data
            self.u[w] = self.momentum * self.u[w] + (1 - self.momentum) * grad
            w.data = w.data - self.lr * self.u[w]


class Adam(Optimizer):
    def __init__(
        self,
        params,
        lr=0.01,
        beta1=0.9,
        beta2=0.999,
        eps=1e-8,
        weight_decay=0.0,
    ):
        super().__init__(params)
        self.lr = lr
        self.beta1 = beta1
        self.beta2 = beta2
        self.eps = eps
        self.weight_decay = weight_decay
        self.t = 0

        self.m = defaultdict(float)
        self.v = defaultdict(float)

    def step(self):
        self.t += 1
        for w in self.params:
            if self.weight_decay > 0:
                grad = w.grad.data + self.weight_decay * w.data
            else:
                grad = w.grad.data
            self.m[w] = self.beta1 * self.m[w] + (1 - self.beta1) * grad
            self.v[w] = self.beta2 * self.v[w] + (1 - self.beta2) * (grad**2)
            unbiased_m = self.m[w] / (1 - self.beta1**self.t)
            unbiased_v = self.v[w] / (1 - self.beta2**self.t)
            w.data = w.data - self.lr * unbiased_m / (unbiased_v**0.5 + self.eps)
