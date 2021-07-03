#!/usr/bin/env python3
'''Run a DNS Server

!NOTICE! You should NOT change this file!

Command line arguments:
    [port]: port number, default: 9999.
            Notice that if you run the program not in root, the port number
            should be greater than 1024.
Usage:
    $ ./runDNSServer.py [port]
'''

import sys
import argparse
import pathlib
import shutil
from dnsServer.dns_server import DNSServer, DNSHandler


def parse_args(argv):
    ''' Parse arguments of the program '''
    parser = argparse.ArgumentParser(description="Run DNS server")
    parser.add_argument("port", action='store', default=9999, type=int, nargs='?',
                        help="port to start the dns service (default: 9999)")
    parser.add_argument("file", action='store', default="./dnsServer/dns_table.txt", 
                        type=str, nargs='?', help="file used to load dns table (default: ./dnsServer/dns_table.txt)")
    args = parser.parse_args(argv)
    return (args.port, args.file)


def main(argv):
    ''' Entry of the program '''
    port,dns_file = parse_args(argv)

    # start a server
    # if you run locally, this will start a dns service at
    # localhost:<port>
    with DNSServer(("", port), dns_file, DNSHandler) as dnsd:
        # print(f"Start a dns server listening on {HOST}:{PORT}")
        print(f"DNS server serving on {dnsd.server_address[0]}:"
              f"{dnsd.server_address[1]}")
        try:
            dnsd.serve_forever()
        except KeyboardInterrupt:
            dnsd.shutdown()
            dnsd.server_close()

if __name__ == '__main__':
    main(sys.argv[1:])
