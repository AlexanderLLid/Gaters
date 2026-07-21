import json
import sys
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "src"))

from guide_skeleton_adapter_verifier import verify_guide_skeleton_adapter


def load(path):
    return json.loads(path.read_text(encoding="utf-8"))


SKELETON_RUN = ROOT / "SkeletonRuns" / "humanoid-guide-skeleton-stick-humanoid-20260721-175401-425940" / "run-1"


def readback_from(skeleton):
    return {
        "schema": "guide-skeleton-adapter-readback/0",
        "joint_names": [joint["name"] for joint in skeleton["joints"]],
        "parents": [joint["parent"] or "" for joint in skeleton["joints"]],
        "positions": [list(joint["position"]) for joint in skeleton["joints"]],
        "transforms": [list(joint["basis"]["aim"] + joint["basis"]["up"] + joint["basis"]["side"]) for joint in skeleton["joints"]],
        "edges": [[bone["parent"], bone["child"]] for bone in skeleton["bones"]],
        "source_guide_sha256": skeleton["source_guide_sha256"],
        "skeleton_sha256": load(SKELETON_RUN / "receipt.json")["skeleton_sha256"],
    }


class GuideSkeletonAdapterVerifierTests(unittest.TestCase):
    def setUp(self):
        self.skeleton = load(SKELETON_RUN / "skeleton.json")
        self.readback = readback_from(self.skeleton)

    def test_accepts_exact_native_readback(self):
        self.assertTrue(verify_guide_skeleton_adapter(self.skeleton, self.readback, self.readback["skeleton_sha256"])["passed"])

    def test_rejects_position_drift(self):
        self.readback["positions"][0][0] += 0.1
        rules = {failure["rule"] for failure in verify_guide_skeleton_adapter(self.skeleton, self.readback, self.readback["skeleton_sha256"])["failures"]}
        self.assertIn("GUIDE-SKELETON-ADAPTER-POSITION-1", rules)

    def test_rejects_hierarchy_drift(self):
        self.readback["edges"].pop()
        rules = {failure["rule"] for failure in verify_guide_skeleton_adapter(self.skeleton, self.readback, self.readback["skeleton_sha256"])["failures"]}
        self.assertIn("GUIDE-SKELETON-ADAPTER-HIERARCHY-1", rules)

    def test_rejects_transform_drift(self):
        self.readback["transforms"][0][0] += 0.1
        rules = {failure["rule"] for failure in verify_guide_skeleton_adapter(self.skeleton, self.readback, self.readback["skeleton_sha256"])["failures"]}
        self.assertIn("GUIDE-SKELETON-ADAPTER-TRANSFORM-1", rules)


if __name__ == "__main__":
    unittest.main()
