import re
import pathlib
from unittest import TestCase, main


labNum = 4  # 1-7
reportPattern = f"\\d{{9}}[\\u4e00-\\u9fa5]+_lab_{labNum}"


class TestDir(TestCase):
    def test_check_report(self):
        workingDir = pathlib.Path(".")
        reportDir = workingDir / "report"
        self.assertTrue(
            reportDir.exists(),
            "Report directory `report/` doesn't exist"
        )
        reports = list(reportDir.glob("*.pdf"))
        self.assertNotEqual(len(reports), 0, "No report PDF found")
        self.assertEqual(len(reports), 1, "More than one report PDF found")
        for report in reports:
            self.assertTrue(
                re.match(reportPattern, report.stem) is not None,
                "Wrong name of report PDF"
            )
        for capfile in reportDir.glob("*.pcap*"):
            self.assertTrue(
                capfile.stem.startswith(f"lab_{labNum}"),
                f"Wrong name of capture file '{capfile}'. "
                f"Should start with 'lab_{labNum}'"
            )

    def test_check_src(self):
        workingDir = pathlib.Path(".")
        srcNames = {
            "myrouter.py",
        }
        pyfiles = set(map(lambda f: f.name, workingDir.glob("*.py")))
        for srcfile in srcNames:
            self.assertTrue(
                srcfile in pyfiles,
                f"'{srcfile}' cannot be found"
            )


if __name__ == '__main__':
    main()
