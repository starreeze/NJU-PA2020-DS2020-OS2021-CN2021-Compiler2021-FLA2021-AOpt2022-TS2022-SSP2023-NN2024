from subprocess import Popen, PIPE
from unittest import TestCase


class Lab1Test(TestCase):
    def test_myhub(self):
        p = Popen(
            ["swyard", "-t", "testcases/myhub_testscenario.py", "myhub.py"],
            stdout=PIPE, stderr=PIPE
        )
        output, error = p.communicate()
        assert p.returncode == 0
