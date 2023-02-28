#!/usr/bin/env python3
'''Testcases entry

!NOTICE! You should NOT change this file!

Usage:
    - Test DNS server:
        $ python3 test_entry.py dns
    - Test caching server:
        $ python3 test_entry.py cache
    - Test all:
        $ python3 test_entry.py all
'''

import sys
import argparse
import time
import rpyc
import unittest
from testcases import test_cache, test_dns, test_all

from utils.manageservice import startForCaching, startForDNS, startAll, terminateAll


VERBOSITY = 2


def parse_args(argv):
    ''' Parse arguments of the program '''
    parser = argparse.ArgumentParser(description="Run tests")
    parser.add_argument("case", choices=['all', 'dns', 'cache'],
                        help="Choose a test case to run")
    parser.add_argument("--dnsip", default="127.0.0.1", type=str,
                        help="DNS server IP address (default '127.0.0.1')")
    parser.add_argument("--mainip", default="127.0.0.1", type=str,
                        help="Main server IP address (default '127.0.0.1')")
    parser.add_argument("--rpcip", default="127.0.0.1", type=str,
                        help="RPC server IP address (default '127.0.0.1')")
    parser.add_argument("--cacheip", default="127.0.0.1", type=str,
                        help="Caching server IP address (default '127.0.0.1')")
    parser.add_argument("--dnsport", "-n", default=9999, type=int,
                        help="Port for DNS server service (default: 9999)")
    parser.add_argument("--mainport", "-m", default=8000, type=int,
                        help="Port for main server service (default: 8000)")
    parser.add_argument("--cacheport", "-c", default=1222, type=int,
                        help="Port for caching server service (default: 1222)")
    parser.add_argument("--rpcport", "-r", default=3322, type=int,
                        help="Port for RPC tracing server service (default: 3322)")
    parser.add_argument("--maindir", "-d", default="mainServer/", type=str,
                        help="Working directory for main server (default: 'mainServer/'")

    args = parser.parse_args(argv)
    return args


def testCache(args):
    test_cache.cacheIP = args.cacheip
    test_cache.cachePort = args.cacheport
    test_cache.mainWorkDir = args.maindir
    try:
        success = startForCaching(mainPort=args.mainport, mainWorkDir=args.maindir,
                           cachePort=args.cacheport, rpcPort=args.rpcport)
        if success:
            time.sleep(1)
            test_cache.rpcserver = rpyc.connect(args.rpcip, port=args.rpcport)
            suite = unittest.TestLoader().loadTestsFromModule(test_cache)
            unittest.TextTestRunner(verbosity=VERBOSITY).run(suite)
    finally:
        terminateAll()


def testDNS(args):
    test_dns.dnsIP = args.dnsip
    test_dns.dnsPort = args.dnsport
    try:
        success = startForDNS(port=args.dnsport)
        if success:
            time.sleep(1)
            suite = unittest.TestLoader().loadTestsFromModule(test_dns)
            unittest.TextTestRunner(verbosity=VERBOSITY).run(suite)
    finally:
        terminateAll()


def testAll(args):
    test_all.dnsIP = args.dnsip
    test_all.dnsPort = args.dnsport
    test_all.cachePort = args.cacheport
    test_all.mainWorkDir = args.maindir
    try:
        success = startAll(dnsPort=args.dnsport,
                           mainPort=args.mainport, mainWorkDir=args.maindir,
                           cachePort=args.cacheport, rpcPort=args.rpcport)
        if success:
            time.sleep(1)
            test_all.rpcserver = rpyc.connect(args.rpcip, port=args.rpcport)
            suite = unittest.TestLoader().loadTestsFromModule(test_all)
            unittest.TextTestRunner(verbosity=VERBOSITY).run(suite)
    finally:
        terminateAll()


def main(argv):
    args = parse_args(argv)
    if args.case == 'dns':
        testDNS(args)
    elif args.case == 'cache':
        testCache(args)
    elif args.case == 'all':
        testAll(args)


if __name__ == '__main__':
    main(sys.argv[1:])
