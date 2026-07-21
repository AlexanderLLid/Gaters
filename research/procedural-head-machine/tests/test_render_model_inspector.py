import json
import math
import re
import sys
import tempfile
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "src"))

from render_model_inspector import render


READBACK = ROOT / "MotionSequenceRuns" / "20260721-213301-311" / "run-1" / "readback.json"


class RenderModelInspectorTests(unittest.TestCase):
    def test_writes_compact_actual_mesh_fragment(self):
        readback = json.loads(READBACK.read_text(encoding="utf-8"))
        with tempfile.TemporaryDirectory() as directory:
            output = Path(directory) / "model-inspector.html"
            render(readback, output)
            fragment = output.read_text(encoding="utf-8")
        self.assertLess(len(fragment.encode("utf-8")), 2_000_000)
        self.assertIn('id="gaters-model-inspector"', fragment)
        self.assertIn('"vertexCount":13149', fragment)
        self.assertIn('"faceCount":13822', fragment)
        self.assertNotIn("fetch(", fragment)
        self.assertNotIn("<html", fragment.lower())
        data = json.loads(re.search(r"const data = (\{.*\});", fragment).group(1))
        self.assertGreater(max(math.dist(rest, peak) for rest, peak in zip(data["rest"], data["peak"])), 0.6)


if __name__ == "__main__":
    unittest.main()
