import copy
import math


def compile_joint_limits(skeleton, recipe, skeleton_sha256):
    joint_names = {joint["name"] for joint in skeleton.get("joints", [])}
    controls = {}
    for source in recipe.get("controls", []):
        names = {source.get("joint"), *source.get("descendants", [])}
        if None in names or not names.issubset(joint_names):
            raise ValueError("JOINT-LIMIT-JOINT-1")
        low, high = source.get("minimum_degrees"), source.get("maximum_degrees")
        if not all(isinstance(value, (int, float)) and math.isfinite(value) for value in (low, high)) or not low <= 0 <= high:
            raise ValueError("JOINT-LIMIT-RANGE-1")
        control = copy.deepcopy(source)
        control.pop("id")
        controls[source["id"]] = control
    return {
        "schema": "joint-limits/0",
        "id": recipe["id"],
        "skeleton_id": skeleton["id"],
        "source_skeleton_sha256": skeleton_sha256,
        "controls": controls,
        "mirror_pairs": copy.deepcopy(recipe.get("mirror_pairs", [])),
    }
