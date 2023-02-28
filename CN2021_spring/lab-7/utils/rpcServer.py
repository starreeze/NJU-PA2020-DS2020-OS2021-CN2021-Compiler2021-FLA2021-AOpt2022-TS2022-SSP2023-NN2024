#!/usr/bin/env python

import sys
import argparse
from collections import namedtuple
import rpyc
from rpyc.utils.server import ThreadedServer

__all__ = ['Calls']


OneCall = namedtuple("OneCall", ("method", "optional"))

class Calls:
    @staticmethod
    def all():
        return {
            "do_GET", "do_HEAD", "requestMainServer", "setHeaders",
            "sendHeaders", "getHeaders"
        }

    @staticmethod
    def do_GET(optional=False):
        return OneCall("do_GET", optional)

    @staticmethod
    def do_HEAD(optional=False):
        return OneCall("do_HEAD", optional)

    @staticmethod
    def fetch(optional=False):
        return OneCall("requestMainServer", optional)

    @staticmethod
    def storeInCache(optional=False):
        return OneCall("setHeaders", optional)

    @staticmethod
    def send(optional=False):
        return OneCall("sendHeaders", optional)

    @staticmethod
    def loadCache(optional=False):
        return OneCall("getHeaders", optional)


def parse_args(argv):
    ''' Parse arguments of the program '''
    parser = argparse.ArgumentParser(description="Run RPC tracing server")
    parser.add_argument("port", action='store', default=3322, type=int, nargs='?',
                        help="port to start the http service (default: 3322)")
    args = parser.parse_args(argv)
    return args.port

class RPCService(rpyc.Service):
    procedures = []
    errors = []

    def called(self, method: str):
        self.procedures.append(method)
        # print(f"Called {method}")

    def echoError(self, method, message):
        ''' echo error message to RPC server '''
        print(f"Error when calling `{method}`: {message}", file=sys.stderr)
        self.errors.append(message)

    def getProcedures(self):
        if len(self.errors) > 0:
            return False, self.errors
        return True, self.procedures

    def clearProcedures(self):
        self.procedures.clear()


if __name__ == "__main__":
    port = parse_args(sys.argv[1:])

    t = ThreadedServer(
        RPCService, port=port,
        protocol_config={'allow_public_attrs': True}
    )
    print(f"Starting RPC server at port: {port}")
    t.start()
