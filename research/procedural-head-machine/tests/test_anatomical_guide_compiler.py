import json
import sys
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "src"))

from anatomical_guide_compiler import compile_anatomical_guide
from body_plan_compiler import compile_body_plan


def load(path):
    return json.loads(path.read_text(encoding="utf-8"))


BODY_RUN = ROOT / "BodyPlanRuns" / "stick-humanoid-20260721-162041-004309" / "run-1"
TALL_RUN = ROOT / "BodyPlanRuns" / "stick-humanoid-tall-20260721-162041-209553" / "run-1"
RECIPE = load(ROOT / "recipes" / "humanoid-anatomical-guide.json")


class AnatomicalGuideCompilerTests(unittest.TestCase):
    def setUp(self):
        self.body = load(BODY_RUN / "composed-mesh.json")
        self.guide = compile_anatomical_guide(self.body, RECIPE)

    def test_emits_required_humanoid_landmarks(self):
        required = {
            "pelvis", "waist", "chest", "neck_base", "head_center",
            "left_shoulder", "left_elbow", "left_wrist",
            "right_shoulder", "right_elbow", "right_wrist",
            "left_hip", "left_knee", "left_ankle",
            "right_hip", "right_knee", "right_ankle",
        }
        self.assertTrue(required.issubset(self.guide["landmarks"]))

    def test_declared_pairs_are_exact_mirrors(self):
        for left_name, right_name in self.guide["symmetry_pairs"]:
            left = self.guide["landmarks"][left_name]
            right = self.guide["landmarks"][right_name]
            self.assertEqual(left, [-right[0], right[1], right[2]])

    def test_limb_segments_use_tapered_radius_profiles(self):
        segments = {segment["id"]: segment for segment in self.guide["surface_segments"]}
        self.assertGreater(segments["left_upper_arm"]["start_radius_m"], segments["left_upper_arm"]["end_radius_m"])
        self.assertGreater(segments["left_thigh"]["start_radius_m"], segments["left_thigh"]["end_radius_m"])

    def test_held_out_body_changes_guide_proportions(self):
        tall = compile_anatomical_guide(load(TALL_RUN / "composed-mesh.json"), RECIPE)
        self.assertLess(tall["landmarks"]["left_ankle"][2], self.guide["landmarks"]["left_ankle"][2])
        self.assertGreater(abs(tall["landmarks"]["left_wrist"][0]), abs(self.guide["landmarks"]["left_wrist"][0]))

    def test_rejects_missing_required_body_module(self):
        broken = json.loads(json.dumps(self.body))
        del broken["placements"]["head"]
        with self.assertRaisesRegex(ValueError, "ANATOMY-BODY-1"):
            compile_anatomical_guide(broken, RECIPE)

    def test_explicit_terminal_modules_survive_into_surface_contract(self):
        body = compile_body_plan(load(ROOT / "recipes" / "stick-humanoid-body-plan.json"))
        guide = compile_anatomical_guide(body, RECIPE)
        modules = {shape["module"] for shape in guide["surface_ellipsoids"]}
        modules.update(segment["module"] for segment in guide["surface_segments"])
        self.assertTrue({"neck", "left_hand", "right_hand", "left_foot", "right_foot"}.issubset(modules))

    def test_profiled_recipe_emits_recognizable_body_volumes(self):
        body = compile_body_plan(load(ROOT / "recipes" / "stick-humanoid-body-plan.json"))
        guide = compile_anatomical_guide(body, RECIPE)
        volumes = {shape["id"]: shape for shape in guide["surface_ellipsoids"]}
        self.assertTrue({"cranium", "lower_face", "left_deltoid", "right_deltoid"}.issubset(volumes))
        self.assertEqual(
            volumes["left_deltoid"]["center"],
            [-volumes["right_deltoid"]["center"][0], *volumes["right_deltoid"]["center"][1:]],
        )
        self.assertLess(volumes["left_hand"]["radii"][2], volumes["left_hand"]["radii"][0])
        self.assertGreater(volumes["left_foot"]["radii"][1], volumes["left_foot"]["radii"][0])
        self.assertGreater(volumes["left_foot"]["radii"][0], volumes["left_foot"]["radii"][2])

    def test_profile_volumes_cover_head_without_oversized_shoulders(self):
        body = compile_body_plan(load(ROOT / "recipes" / "stick-humanoid-body-plan.json"))
        guide = compile_anatomical_guide(body, RECIPE)
        head = [shape for shape in guide["surface_ellipsoids"] if shape["module"] == "head"]
        head_bounds = body["placements"]["head"]["bounds_cells"]
        expected_height = (head_bounds[1][2] - head_bounds[0][2]) * body["cell_size_m"]
        covered_height = (
            max(shape["center"][2] + shape["radii"][2] for shape in head)
            - min(shape["center"][2] - shape["radii"][2] for shape in head)
        )
        self.assertGreaterEqual(covered_height, expected_height * 0.95)
        volumes = {shape["id"]: shape for shape in guide["surface_ellipsoids"]}
        upper_arm = next(segment for segment in guide["surface_segments"] if segment["id"] == "left_upper_arm")
        self.assertLessEqual(max(volumes["left_deltoid"]["radii"]), upper_arm["start_radius_m"] * 1.05)


if __name__ == "__main__":
    unittest.main()
