import hashlib
import json
import tempfile
import unittest
from pathlib import Path

from anatomical_face_machine import decide, initialize_run


class AnatomicalFaceMachineTests(unittest.TestCase):
    def test_initialize_run_binds_inputs_and_refuses_overwrite(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            candidate = root / "candidate.json"
            candidate.write_text('{"candidateId":"abc"}\n', encoding="utf-8")
            target = root / "target.json"
            target.write_text('{"ratios":{"eyeAperture":1,"noseLength":2,"alarWidth":3}}\n', encoding="utf-8")
            image = root / "front.png"
            image.write_bytes(b"front")
            source = root / "source.blend"
            source.write_bytes(b"blend")
            contract = {
                "schemaVersion": 1,
                "machineId": "research.anatomical-front-fit",
                "sourceBlend": str(source),
                "candidateRecipe": str(candidate),
                "targetGeometry": str(target),
                "targetImage": str(image),
                "maximumRatioError": 0.2,
            }

            initialized = initialize_run(contract, root / "Runs" / "run-1")

            copied = root / "Runs" / "run-1" / "candidates" / "candidate.json"
            self.assertEqual(copied.read_bytes(), candidate.read_bytes())
            self.assertEqual(initialized["status"], "initialized")
            self.assertEqual(initialized["inputs"]["targetImage"]["sha256"], hashlib.sha256(b"front").hexdigest())
            with self.assertRaises(FileExistsError):
                initialize_run(contract, root / "Runs" / "run-1")

    def test_decision_keeps_mechanical_and_visual_acceptance_separate(self):
        target = {"ratios": {"eyeAperture": 0.14, "noseLength": 0.74, "alarWidth": 0.70}}
        measured = {"ratios": {"eyeAperture": 0.15, "noseLength": 0.73, "alarWidth": 0.71}}

        result = decide(target, measured, maximum_error=0.05)

        self.assertEqual(result["mechanicalDecision"], "pass")
        self.assertEqual(result["visualDecision"], "pending-human")
        self.assertEqual(result["decision"], "review")
        self.assertAlmostEqual(result["totalRatioError"], 0.03)

        rejected = decide(target, measured, maximum_error=0.02)
        self.assertEqual(rejected["mechanicalDecision"], "reject")
        self.assertEqual(rejected["decision"], "reject")


if __name__ == "__main__":
    unittest.main()
