#!/usr/bin/env python3
'''Run a Caching Server

!NOTICE! You should NOT change this file!

Command line arguments:
    mainserver: main server adress.
    [port]: port number, default: 1222.
            Notice that if you run the program not in root, the port number
            should be greater than 1024.
Usage:
    $ ./runCachingServer.py <mainserver> [port]
Example:
    $ ./runCachingServer.py localhost:8000 1222
'''

import sys
import argparse
import pathlib
import shutil
from cachingServer.cachingServer import CachingServer, CachingServerHttpHandler


def parse_args(argv):
    ''' Parse arguments of the program '''
    parser = argparse.ArgumentParser(description="Run HTTP caching server")
    parser.add_argument("mainserver", type=str, help="remote main server to fetch file")
    parser.add_argument("port", action='store', default=1222, type=int, nargs='?',
                        help="port to start the http service (default: 1222)")
    parser.add_argument("--rpcserver", "-r", type=str, help="rpc server for tracing (used in test mode)")
    args = parser.parse_args(argv)
    return args.mainserver, args.port, args.rpcserver


def connectRPC(rpcAddr):
    import utils.tracer
    if rpcAddr is not None:
        try:
            addr, port = rpcAddr.split(":")
        except ValueError:
            print(f"wrong format of rpc server address '{rpcAddr}'", file=sys.stderr)
            sys.exit()
        utils.tracer.initateRPCServerProxy(addr, port)


def main(argv):
    ''' Entry of the program '''
    mainserver, port, rpcAddr = parse_args(argv)
    connectRPC(rpcAddr)

    # start a server
    # if you run locally, this will start a http service at
    # http://localhost:<port>
    with CachingServer(("", port), CachingServerHttpHandler, mainserver) as httpd:
        print(f"Caching server serving on http://{httpd.server_address[0]}:"
              f"{httpd.server_address[1]}")
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            httpd.shutdown()
            httpd.server_close()


if __name__ == '__main__':
    # main("localhost:8000", 1222)  # for development
    main(sys.argv[1:])
