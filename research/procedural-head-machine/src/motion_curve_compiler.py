import copy
import math

from pose_skeleton import apply_pose


def _sample(keys, time_seconds):
    if time_seconds <= keys[0]["time_seconds"]:
        return float(keys[0]["degrees"])
    for left, right in zip(keys, keys[1:]):
        if time_seconds <= right["time_seconds"]:
            span = right["time_seconds"] - left["time_seconds"]
            factor = (time_seconds - left["time_seconds"]) / span
            return left["degrees"] + (right["degrees"] - left["degrees"]) * factor
    return float(keys[-1]["degrees"])


def compile_motion(skeleton, limits, recipe, skeleton_sha256, limits_sha256):
    controls = limits.get("controls", {})
    tracks = recipe.get("tracks", [])
    duration, fps = recipe.get("duration_seconds"), recipe.get("fps")
    if not isinstance(fps, int) or fps <= 0 or not isinstance(duration, (int, float)) or duration <= 0 or duration * fps % 1:
        raise ValueError("MOTION-TIME-1")
    if len({track.get("control") for track in tracks}) != len(tracks):
        raise ValueError("MOTION-TRACK-1")
    for track in tracks:
        control = controls.get(track.get("control"))
        keys = track.get("keys", [])
        times = [key.get("time_seconds") for key in keys]
        if not control or len(keys) < 2 or times[0] != 0 or times[-1] != duration or any(not isinstance(value, (int, float)) or not math.isfinite(value) for value in times) or any(right <= left for left, right in zip(times, times[1:])):
            raise ValueError("MOTION-TRACK-1")
        if any(not isinstance(key.get("degrees"), (int, float)) or not math.isfinite(key["degrees"]) or not control["minimum_degrees"] <= key["degrees"] <= control["maximum_degrees"] for key in keys):
            raise ValueError("MOTION-LIMIT-1")

    frames = []
    for index in range(round(duration * fps) + 1):
        time_seconds = index / fps
        values = {track["control"]: _sample(track["keys"], time_seconds) for track in tracks}
        working = copy.deepcopy(skeleton)
        if any(abs(value) > 1e-12 for value in values.values()):
            for track in tracks:
                control = controls[track["control"]]
                working["joints"] = apply_pose(working, {**control, "degrees": values[track["control"]]})
        frames.append({"index": index, "time_seconds": time_seconds, "controls": values, "joints": working["joints"]})
    return {
        "schema": "motion-frames/0",
        "id": recipe["id"],
        "skeleton_id": skeleton["id"],
        "source_skeleton_sha256": skeleton_sha256,
        "source_joint_limits_sha256": limits_sha256,
        "fps": fps,
        "duration_seconds": duration,
        "frames": frames,
    }
