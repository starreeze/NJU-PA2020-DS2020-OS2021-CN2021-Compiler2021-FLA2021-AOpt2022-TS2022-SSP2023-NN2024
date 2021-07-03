#!/usr/bin/env python3

import sys
import socket
import unittest
from utils.dns_utils import DNS_Request, DNS_Response
from utils.network import resolve_domain_name


# should be set by test_entry
dnsIP = None
dnsPort = None


class TestDNS(unittest.TestCase):

    def resolveDomain(self, domain_name):
        global dnsIP, dnsPort
        return resolve_domain_name(domain_name, dnsIP, dnsPort)
    
    def test_non_exist(self):
        res = self.resolveDomain("domain.non.exists")
        self.assertEqual(res, None)

    def test_cname1(self):
        res = self.resolveDomain("test.cncourse.org.")
        self.assertEqual(res.response_type, 5)
        self.assertEqual(str(res.response_val), "home.nasa.org.")

    def test_cname2(self):
        res = self.resolveDomain("home.cncourse.org")
        self.assertEqual(res.response_type, 5)
        self.assertEqual(str(res.response_val), "home.nasa.org.")
    
    def test_location1(self):
        res = self.resolveDomain("home.nasa.org.")
        self.assertEqual(res.response_type, 1)
        self.assertEqual(str(res.response_val), "10.0.0.1")

    def test_location2(self):
        res = self.resolveDomain("lab.nasa.org")
        self.assertEqual(res.response_type, 1)
        self.assertEqual(str(res.response_val), "10.0.0.5")


if __name__ == '__main__':
    unittest.main()
