#!/usr/bin/env python3

from time import time
from .baseTestcase import BaseTestcase


# should be set by test_entry
dnsIP = None
dnsPort = None
rpcserver = None
mainWorkDir = None
cachePort = None

targetDomain = "stfw.localhost.computer"

targetFiles = {
    "doc/": "doc/index.html",
    "doc/success.jpg": "doc/success.jpg"
}


class TestAll(BaseTestcase):
    def __init__(self, *args, **kwargs):
        super().__init__(rpcserver, mainWorkDir, targetFiles,
                         *args, **kwargs)

    def test_01_cache_missed_1(self):
        target = "doc/success.jpg"
        self.cache_missed_template(targetDomain, cachePort,
                                   target, dnsIP, dnsPort)

    def test_02_cache_hit_1(self):
        target = "doc/success.jpg"
        self.cache_hit_template(targetDomain, cachePort,
                                   target, dnsIP, dnsPort)

    def test_03_not_found(self):
        target = "noneexist"
        self.not_found_template(targetDomain, cachePort,
                                target, dnsIP, dnsPort)
