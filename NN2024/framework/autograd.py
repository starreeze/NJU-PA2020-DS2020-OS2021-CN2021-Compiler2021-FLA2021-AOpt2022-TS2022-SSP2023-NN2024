from typing import Optional, Union
from abc import ABC, abstractmethod
import framework as fw
from framework import init
import numpy as np
from numpy import ndarray

LAZY_MODE = False


class Device(ABC):
    pass


class CPUDevice(Device):
    def __repr__(self):
        return "cpu()"

    def __hash__(self):
        return self.__repr__().__hash__()

    def __eq__(self, other):
        return isinstance(other, CPUDevice)

    def enabled(self):
        return True

    def zeros(self, *shape, dtype=np.float32):
        return np.zeros(shape, dtype=dtype)

    def ones(self, *shape, dtype=np.float32):
        return np.ones(shape, dtype=dtype)

    def randn(self, *shape):
        return np.random.randn(*shape)

    def rand(self, *shape):
        return np.random.rand(*shape)

    def one_hot(self, n, i, dtype=np.float32):
        return np.eye(n, dtype=dtype)[i]


def cpu():
    """Return cpu device"""
    return CPUDevice()


class Op(ABC):
    @abstractmethod
    def __call__(self, *args):
        raise NotImplementedError()

    @abstractmethod
    def compute(self, *args: list["Tensor"]):
        raise NotImplementedError()

    @abstractmethod
    def gradient(self, out_grad: "Tensor", node: "Tensor") -> Union["Tensor", tuple["Tensor"]]:
        raise NotImplementedError()

    def gradient_as_tuple(self, out_grad: "Tensor", node: "Tensor") -> tuple["Tensor"]:
        output = self.gradient(out_grad, node)
        if isinstance(output, tuple):
            return output
        elif isinstance(output, list):
            return tuple(output)
        else:
            return (output,)


class TensorOp(Op):
    """Op class specialized to output tensors, will be alterate subclasses for other structures"""

    def __call__(self, *args):
        return Tensor.make_from_op(self, list(args))


class TensorTupleOp(Op):
    """Op class specialized to output Tensortuple"""

    def __call__(self, *args):
        return TensorTuple.make_from_op(self, args)


class Value:
    op: Optional[Op]
    inputs: list["Value"]
    cached_data: Optional[ndarray]
    requires_grad: bool

    def realize_cached_data(self) -> ndarray:
        if self.cached_data is not None:
            return self.cached_data
        assert self.op is not None
        self.cached_data = self.op.compute(*[x.realize_cached_data() for x in self.inputs])  # type: ignore
        return self.cached_data  # type: ignore

    def is_leaf(self):
        return self.op is None

    def _init(
        self,
        op: Optional[Op],
        inputs: list["Value"],
        num_outputs: int = 1,
        cached_data: Optional[ndarray] = None,
        requires_grad: Optional[bool] = None,
    ):
        if requires_grad is None:
            requires_grad = any(x.requires_grad for x in inputs)
        self.op = op
        self.inputs = inputs
        self.num_outputs = num_outputs
        self.cached_data = cached_data
        self.requires_grad = requires_grad


class TensorTuple(Value):
    """Represent a tuple of tensors.

    To keep things simple, we do not support nested tuples.
    """

    def __len__(self):
        cdata = self.realize_cached_data()
        return len(cdata)

    def __getitem__(self, index: int):
        return fw.ops.tuple_get_item(self, index)

    def tuple(self):
        return tuple([x for x in self])

    def __repr__(self):
        return "TensorTuple" + str(self.tuple())

    def __str__(self):
        return self.__repr__()

    def __add__(self, other):
        assert isinstance(other, TensorTuple)
        assert len(self) == len(other)
        return fw.ops.make_tuple(*[self[i] + other[i] for i in range(len(self))])


class Tensor(Value):
    grad: "Tensor"

    def __init__(self, array, *, device: Optional[Device] = None, dtype=None, requires_grad=True, **kwargs):
        if isinstance(array, Tensor):
            if device is None:
                device = array.device
            if dtype is None:
                dtype = array.dtype
            if device == array.device and dtype == array.dtype:
                cached_data = array.realize_cached_data()
            else:
                cached_data = Tensor.from_numpy(array.numpy(), device=device, dtype=dtype)
        else:
            device = device if device else cpu()
            cached_data = Tensor.from_numpy(array, device=device, dtype=dtype)

        self._init(
            None,
            [],
            cached_data=cached_data,
            requires_grad=requires_grad,
        )

    @staticmethod
    def from_numpy(numpy_array, device, dtype):
        return np.array(numpy_array, dtype=dtype)

    @staticmethod
    def make_from_op(op: Op, inputs: list["Value"]):
        tensor = Tensor.__new__(Tensor)
        tensor._init(op, inputs)
        if not LAZY_MODE:
            if not tensor.requires_grad:
                return tensor.detach()
            tensor.realize_cached_data()
        return tensor

    @staticmethod
    def make_const(data, requires_grad=False):
        tensor = Tensor.__new__(Tensor)
        tensor._init(
            None,
            [],
            cached_data=data if not isinstance(data, Tensor) else data.realize_cached_data(),
            requires_grad=requires_grad,
        )
        return tensor

    @property
    def data(self):
        return self.detach()

    @data.setter
    def data(self, value):
        assert isinstance(value, Tensor)
        self.cached_data = value.realize_cached_data().astype(self.dtype)

    def detach(self):
        """Create a new tensor that shares the data but detaches from the graph."""
        return Tensor.make_const(self.realize_cached_data())

    @property
    def shape(self):
        return self.realize_cached_data().shape

    @property
    def dtype(self):
        return self.realize_cached_data().dtype

    @property
    def device(self):
        return cpu()

    def backward(self, out_grad=None):
        out_grad = out_grad if out_grad else init.ones(*self.shape, dtype=self.dtype, device=self.device)
        compute_gradient_of_variables(self, out_grad)

    def __repr__(self):
        return "Tensor(" + str(self.realize_cached_data()) + ")"

    def __str__(self):
        return self.realize_cached_data().__str__()

    def numpy(self):
        data = self.realize_cached_data()
        return data

    def __add__(self, other):
        if isinstance(other, Tensor):
            return fw.ops.EWiseAdd()(self, other)
        else:
            return fw.ops.AddScalar(other)(self)

    def __mul__(self, other):
        if isinstance(other, Tensor):
            return fw.ops.EWiseMul()(self, other)
        else:
            return fw.ops.MulScalar(other)(self)

    def __pow__(self, other):
        if isinstance(other, Tensor):
            raise NotImplementedError()
        else:
            return fw.ops.PowerScalar(other)(self)

    def __sub__(self, other):
        if isinstance(other, Tensor):
            return fw.ops.EWiseAdd()(self, fw.ops.Negate()(other))
        else:
            return fw.ops.AddScalar(-other)(self)

    def __truediv__(self, other):
        if isinstance(other, Tensor):
            return fw.ops.EWiseDiv()(self, other)
        else:
            return fw.ops.DivScalar(other)(self)

    def __matmul__(self, other):
        return fw.ops.MatMul()(self, other)

    def matmul(self, other):
        return fw.ops.MatMul()(self, other)

    def sum(self, axes=None):
        return fw.ops.Summation(axes)(self)

    def broadcast_to(self, shape):
        return fw.ops.BroadcastTo(shape)(self)

    def reshape(self, shape):
        return fw.ops.Reshape(shape)(self)

    def __neg__(self):
        return fw.ops.Negate()(self)

    def transpose(self, axes=None):
        return fw.ops.Transpose(axes)(self)

    __radd__ = __add__
    __rmul__ = __mul__
    __rsub__ = __sub__
    __rmatmul__ = __matmul__


def compute_gradient_of_variables(output_tensor, out_grad):
    node_to_output_grads_list: dict[Tensor, list[Tensor]] = {}
    node_to_output_grads_list[output_tensor] = [out_grad]

    reverse_topo_order = list(reversed(find_topo_sort([output_tensor])))

    for node in reverse_topo_order:
        ajoint: Tensor = sum(node_to_output_grads_list[node])  # type: ignore
        node.grad = ajoint
        if node.op is None:
            continue
        partial_ajoints = node.op.gradient_as_tuple(ajoint, node)
        for in_node, partial_ajoint in zip(node.inputs, partial_ajoints):
            if in_node not in node_to_output_grads_list:
                node_to_output_grads_list[in_node] = []  # type: ignore
            node_to_output_grads_list[in_node].append(partial_ajoint)  # type: ignore


def find_topo_sort(node_list: list[Tensor]) -> list[Tensor]:
    topo_order = []
    visited = set()
    for node in node_list:
        topo_sort_dfs(node, visited, topo_order)
    return topo_order


def topo_sort_dfs(node, visited, topo_order):
    if node in visited:
        return
    for in_node in node.inputs:
        topo_sort_dfs(in_node, visited, topo_order)
    topo_order.append(node)
    visited.add(node)
