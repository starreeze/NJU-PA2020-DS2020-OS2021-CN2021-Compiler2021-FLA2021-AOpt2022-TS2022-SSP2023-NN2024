import os
import sys
import http.server
from http.server import SimpleHTTPRequestHandler
from functools import partial


def main(argv):
    import argparse

    parser = argparse.ArgumentParser(argv)
    parser.add_argument('--bind', '-b', default='', metavar='ADDRESS',
                        help='Specify alternate bind address '
                             '[default: all interfaces]')
    parser.add_argument('--directory', '-d', default=os.getcwd(),
                        help='Specify alternative directory '
                        '[default:current directory]')
    parser.add_argument('port', action='store',
                        default=8000, type=int,
                        nargs='?',
                        help='Specify alternate port [default: 8000]')
    args = parser.parse_args()
    os.chdir(args.directory)
    http.server.test(HandlerClass=SimpleHTTPRequestHandler, port=args.port, bind=args.bind)


if __name__ == '__main__':
    main(sys.argv[1:])
