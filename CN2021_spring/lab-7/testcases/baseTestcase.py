#!/usr/bin/env python3

import unittest
import time
import pathlib
from http import HTTPStatus
from utils.rpcServer import Calls
from utils.network import curl, createUrl

__all__ = ['BaseTestcase']


class BaseTestcase(unittest.TestCase):
    def __init__(self, rpcserver, checkDir, targetFiles,
                 *args, **kwargs):
        self.rpcserver = rpcserver
        self.checkDir = checkDir
        self.targetFiles = targetFiles
        super().__init__(*args, **kwargs)

    def compareProcedures(self, procedures, expectProcedures):
        eproIter = iter(expectProcedures)
        for i, pro in enumerate(procedures):
            try:
                epro = next(eproIter)
            except StopIteration:
                break
            if pro not in Calls.all():
                continue
            while epro.optional and pro != epro.method:
                # skip the optional epro
                epro = next(eproIter)
            self.assertEqual(pro, epro.method,
                             "After calling methods:\n  - {}"\
                             .format('\n  - '.join(procedures[:i]))
                             + f"\nExpecting `{epro.method}` but you called `{pro}`")
        else:
            try:
                epro = next(eproIter)
                while epro.optional:
                    epro = next(eproIter)
            except StopIteration:
               return
            # expectProcedures is longer that procedures
            self.assertTrue(False, f"Expecting `{epro.method}` but you call nothing")

    def checkResponse(self, body, target):
        if self.checkDir is not None:
            filepath = pathlib.Path(self.checkDir) / self.targetFiles[target]
        else:
            filepath = self.targetFiles[target]
        with open(filepath, "rb") as fp:
            self.assertEqual(body, fp.read(), "File damaged")

    def request_template(self, expectProcedures, visitIP, visitPort, target,
                              dnsIP=None, dnsPort=None, method="GET"):
        url = createUrl(netloc=visitIP, port=visitPort, path=target)
        status, body = curl(url, dnsIP, dnsPort, method=method)
        self.assertEqual(status, HTTPStatus.OK,
                         f"request to caching server failed with status '{status}'")
        if method == "GET":
            self.checkResponse(body, target)

        time.sleep(0.3)  # wait for asynchronous RPC calls arrive at RPC server
        success, procedures = self.rpcserver.root.getProcedures()
        if success:
            self.compareProcedures(procedures, expectProcedures)
        else:
            print(f"RPC calling raises exceptions: {procedures}", file=sys.stderr)

    def cache_missed_template(self, visitIP, visitPort, target,
                              dnsIP=None, dnsPort=None):
        expectProcedures = [
            Calls.do_GET(),
            Calls.fetch(),
            Calls.storeInCache(),
            Calls.send(),
            Calls.loadCache(optional=True)
        ]
        self.request_template(expectProcedures, visitIP, visitPort, target, dnsIP, dnsPort)

    def cache_hit_template(self, visitIP, visitPort, target,
                           dnsIP=None, dnsPort=None):
        expectProcedures = [
            Calls.do_GET(),
            Calls.loadCache(optional=True),
            Calls.send(),
            Calls.loadCache(optional=True)
        ]
        self.request_template(expectProcedures, visitIP, visitPort, target, dnsIP, dnsPort)

    def cache_missed_HEAD_template(self, visitIP, visitPort, target,
                           dnsIP=None, dnsPort=None):
        expectProcedures = [
            Calls.do_HEAD(),
            Calls.fetch(),
            Calls.storeInCache(),
            Calls.send(),
            Calls.loadCache(optional=True)
        ]
        self.request_template(expectProcedures, visitIP, visitPort,
                              target, dnsIP, dnsPort, method="HEAD")

    def cache_hit_HEAD_template(self, visitIP, visitPort, target,
                           dnsIP=None, dnsPort=None):
        expectProcedures = [
            Calls.do_HEAD(),
            Calls.loadCache(optional=True),
            Calls.send(),
            Calls.loadCache(optional=True)
        ]
        self.request_template(expectProcedures, visitIP, visitPort,
                              target, dnsIP, dnsPort, method="HEAD")

    def not_found_template(self, visitIP, visitPort, target,
                           dnsIP=None, dnsPort=None):
        url = createUrl(netloc=visitIP, port=visitPort, path=target)
        status, body = curl(url, dnsIP, dnsPort)
        self.assertEqual(status, HTTPStatus.NOT_FOUND,
                         "should get a HTTP NOT FOUND response")

        success, procedures = self.rpcserver.root.getProcedures()
        expectProcedures = [
            Calls.do_GET(),
            Calls.fetch()
        ]
        if success:
            self.compareProcedures(procedures, expectProcedures)
        else:
            print(f"RPC calling raises exceptions: {procedures}", file=sys.stderr)

    def tearDown(self):
        self.rpcserver.root.clearProcedures()
        time.sleep(0.3)
