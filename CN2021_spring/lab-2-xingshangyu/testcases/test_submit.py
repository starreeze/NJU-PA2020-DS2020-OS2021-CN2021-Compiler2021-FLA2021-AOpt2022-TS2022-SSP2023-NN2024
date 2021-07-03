import re
import pathlib
from unittest import TestCase


labNum = 2  # 1-7
reportPattern = f"\\d{{9}}[\\u4e00-\\u9fa5]+_lab_{labNum}"


class TestDir(TestCase):
    def test_check_report(self):
        workingDir = pathlib.Path(".")
        reportDir = workingDir / "report"
        self.assertTrue(
            reportDir.exists(),
            "Report directory `report/` doesn't exist"
        )
        reportCnt = 0
        for report in reportDir.glob("*.pdf"):
            reportCnt += 1
            self.assertTrue(
                re.match(reportPattern, report.stem) is not None,
                "Wrong name of report PDF"
            )
        self.assertNotEqual(reportCnt, 0, "No report PDF found")
        self.assertEqual(reportCnt, 1, "More than one report PDF found")
        for capfile in reportDir.glob("*.pcap*"):
            self.assertTrue(
                capfile.stem.startswith(f"lab_{labNum}"),
                f"Wrong name of capture file {capfile}. "
                f"Should start with 'lab_{labNum}'"
            )

    def test_check_src(self):
        workingDir = pathlib.Path(".")
        srcCnt = 0
        srcNames = {
            "myswitch.py",
            "myswitch_lru.py",
            "myswitch_to.py",
            "myswitch_traffic.py"
        }
        pyfiles = set(map(lambda f: f.name, workingDir.glob("*.py")))
        for srcfile in srcNames:
            self.assertTrue(
                srcfile in pyfiles,
                f"'{srcfile}' cannot be found"
            )
