import json
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parent


class RawSculptContractTests(unittest.TestCase):
    def test_contract_names_one_input_and_one_output_stage(self):
        contract = json.loads((ROOT / "raw-sculpt-v3.json").read_text(encoding="utf-8"))
        self.assertEqual(contract["machineId"], "research.raw-head-design-sculpt")
        self.assertTrue((ROOT / contract["targetImage"]).resolve().is_file())
        self.assertEqual(contract["anatomySource"]["model"], "GNM Head v3")
        self.assertEqual(contract["identity"]["genderClass"], "female")
        self.assertEqual(contract["identity"]["ethnicityClass"], "black")

    def test_build_emits_required_artifacts(self):
        contract = json.loads((ROOT / "raw-sculpt-v3.json").read_text(encoding="utf-8"))
        output = ROOT / contract["outputDirectory"]
        expected = {"raw-head.blend", "front.png", "three-quarter.png", "profile.png", "manifest.json"}
        self.assertTrue(expected.issubset({path.name for path in output.glob("*")}))


if __name__ == "__main__":
    unittest.main()
