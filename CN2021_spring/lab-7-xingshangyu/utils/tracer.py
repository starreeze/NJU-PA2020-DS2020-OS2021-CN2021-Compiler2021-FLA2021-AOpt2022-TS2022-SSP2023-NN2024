import functools
import rpyc


rpcserver = None


def initateRPCServerProxy(addr=None, port=None):
    global rpcserver
    if addr is not None:
        if port is None:
            port = 3322
        rpcserver = rpyc.connect(addr, port, config={"allow_all_attrs" : True})


def trace(method):
    ''' Trace methods calling '''
    @functools.wraps(method)
    def wrapper(*args, **kwargs):
        if rpcserver is not None:
            try:
                rpyc.async_(rpcserver.root.called)(method.__name__)
            except Exception as e:
                rpcserver.root.echoError(method.__name__, e)
        ret = method(*args, **kwargs)
        return ret
    return wrapper
