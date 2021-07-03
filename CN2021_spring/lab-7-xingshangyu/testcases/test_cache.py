#!/usr/bin/env python3
'''Testcases for caching server

!NOTICE! You should NOT change this file!

You should run the test by running "test_entry.py"
'''

from .baseTestcase import BaseTestcase


# should be set by test_entry
rpcserver = None
mainWorkDir = None
cacheIP = None
cachePort = None

targetFiles = {
    "doc/": "doc/index.html",
    "doc/success.jpg": "doc/success.jpg"
}


class TestCache(BaseTestcase):
    def __init__(self, *args, **kwargs):
        super().__init__(rpcserver, mainWorkDir, targetFiles,
                         *args, **kwargs)

    def test_01_cache_missed_1(self):
        target = "doc/"
        self.cache_missed_template(cacheIP, cachePort, target)

    def test_02_cache_hit_1(self):
        target = "doc/"
        self.cache_hit_template(cacheIP, cachePort, target)

    def test_03_cache_missed_2(self):
        target = "doc/success.jpg"
        self.cache_missed_template(cacheIP, cachePort, target)

    def test_04_cache_hit_2(self):
        target = "doc/success.jpg"
        self.cache_hit_template(cacheIP, cachePort, target)

    def test_05_HEAD(self):
        target = "doc/success.jpg"
        self.cache_hit_HEAD_template(cacheIP, cachePort, target)

    def test_06_not_found(self):
        target = "noneexist"
        self.not_found_template(cacheIP, cachePort, target)
