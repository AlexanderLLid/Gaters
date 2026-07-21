"""Apply one bounded style-macro shape key to the retained anatomical head."""

import argparse
import hashlib
import json
import math
import sys
from pathlib import Path

import bpy
from mathutils import Vector
from bl_ext.blender_org.mpfb.services import LocationService, TargetService

ROOT = Path(__file__).resolve().parent
if str(ROOT) not in sys.path:
    sys.path.insert(0, str(ROOT))

from sculpt_clay import material, mixed_coordinates, render_views
from style_macro import (
    bounded_parameter,
    deform_point,
    resolve_eye_height,
    resolve_eye_narrows,
    resolve_eye_scale_decreases,
    resolve_mouth_narrow,
    resolve_mouth_width,
)


def arguments():
    parser = argparse.ArgumentParser()
    parser.add_argument("--recipe", required=True)
    parser.add_argument("--output", required=True)
    return parser.parse_args(sys.argv[sys.argv.index("--") + 1:])


def sha256(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def topology_hash(head):
    payload = json.dumps(
        [list(polygon.vertices) for polygon in head.data.polygons],
        separators=(",", ":"),
    ).encode("utf-8")
    return hashlib.sha256(payload).hexdigest()


def load_recipe(path):
    recipe_path = Path(path).resolve()
    recipe = json.loads(recipe_path.read_text(encoding="utf-8"))
    required = {"schemaVersion", "controlSet", "sourceBlend", "baselineMacro", "targetGeometry", "parameters"}
    if set(recipe) != required or recipe["schemaVersion"] != 1:
        raise ValueError("unsupported style macro recipe")
    if recipe["controlSet"] != "gaters.style-macro-direct.v1":
        raise ValueError("unexpected style macro control set")
    expected_parameters = {"upperHeightScale", "lowerHeightScale", "cheekScale", "jawScale", "lateralPower"}
    optional_parameters = {
        "mouthWidthValue", "mouthNarrowValue", "eyeScaleDecrease", "eyeHeightValue",
        "eyeScaleDecreaseLeft", "eyeScaleDecreaseRight",
        "eyeNarrowLeft", "eyeNarrowRight",
        "skinColor", "lipColor", "browColor", "scleraColor", "irisColor",
        "pupilColor", "showBrows", "addPupils",
        "eyeBagLeft", "eyeBagRight", "upperLipVolumeIncrease",
        "lowerLipVolumeIncrease", "upperLipVolumeDecrease", "lowerLipVolumeDecrease",
        "browWidthScale", "browHeightScale", "browDepthScale",
    }
    if not expected_parameters <= set(recipe["parameters"]) or set(recipe["parameters"]) - expected_parameters - optional_parameters:
        raise ValueError("style macro parameters are incomplete")
    for name in ("sourceBlend", "baselineMacro", "targetGeometry"):
        recipe[name] = str((ROOT / recipe[name]).resolve())
        if not Path(recipe[name]).is_file():
            raise FileNotFoundError(recipe[name])
    if Path(bpy.data.filepath).resolve() != Path(recipe["sourceBlend"]):
        raise RuntimeError("style macro opened the wrong source blend")
    return recipe_path, recipe


def body_indices(head):
    group = head.vertex_groups.get("body")
    if group is None:
        raise RuntimeError("head body vertex group is missing")
    return {
        vertex.index for vertex in head.data.vertices
        if any(link.group == group.index and link.weight > 0.5 for link in vertex.groups)
    }


def apply_presentation(head, parameters):
    color_names = {
        "skinColor": "SculptClay",
        "scleraColor": "SculptSclera",
        "irisColor": "SculptIris",
    }
    for parameter, material_name in color_names.items():
        if parameter not in parameters:
            continue
        color = parameters[parameter]
        if len(color) != 4 or any(not 0.0 <= float(channel) <= 1.0 for channel in color):
            raise ValueError(f"{parameter} must be bounded RGBA")
        surface = bpy.data.materials.get(material_name)
        if surface is None:
            raise RuntimeError(f"presentation material is missing: {material_name}")
        surface.node_tree.nodes.get("Principled BSDF").inputs["Base Color"].default_value = color

    if parameters.get("showBrows", False):
        brows = bpy.data.objects.get("FaceProofHead.eyebrow003")
        if brows is None:
            raise RuntimeError("source brow geometry is missing")
        brow = material("StyleBrow", parameters["browColor"], 0.88)
        brows.data.materials.clear()
        brows.data.materials.append(brow)
        brows.hide_render = False
        brows.hide_set(False)
        width_scale = bounded_parameter(parameters, "browWidthScale", 1.0)
        height_scale = bounded_parameter(parameters, "browHeightScale", 1.0)
        depth_scale = bounded_parameter(parameters, "browDepthScale", 1.0)
        for sign in (-1, 1):
            vertices = [vertex for vertex in brows.data.vertices if (vertex.co.x < 0) == (sign < 0)]
            center = sum((vertex.co for vertex in vertices), Vector()) / len(vertices)
            for vertex in vertices:
                offset = vertex.co - center
                vertex.co = center + Vector((
                    offset.x * width_scale,
                    offset.y * depth_scale,
                    offset.z * height_scale,
                ))

    if "lipColor" in parameters:
        lips = head.vertex_groups.get("lips")
        if lips is None:
            raise RuntimeError("source lips vertex group is missing")
        lip = material("StyleLip", parameters["lipColor"], 0.72)
        head.data.materials.append(lip)
        lip_vertices = {
            vertex.index for vertex in head.data.vertices
            if any(link.group == lips.index and link.weight > 0.5 for link in vertex.groups)
        }
        for polygon in head.data.polygons:
            if sum(index in lip_vertices for index in polygon.vertices) >= len(polygon.vertices) / 2:
                polygon.material_index = len(head.data.materials) - 1

    if parameters.get("addPupils", False):
        pupil_material = material("StylePupil", parameters["pupilColor"], 0.8)
        for side in ("L", "R"):
            iris = bpy.data.objects.get(f"SculptPupil{side}")
            if iris is None:
                raise RuntimeError(f"source iris is missing: {side}")
            bpy.ops.mesh.primitive_uv_sphere_add(
                segments=20, ring_count=10, radius=1.0,
                location=(iris.location.x, iris.location.y - 0.0008, iris.location.z),
            )
            pupil = bpy.context.object
            pupil.name = f"StylePupil{side}"
            pupil.scale = (0.00105, 0.00045, 0.00105)
            pupil.data.materials.append(pupil_material)


def main():
    args = arguments()
    recipe_path, recipe = load_recipe(args.recipe)
    output = Path(args.output).resolve()
    if output.exists() and any(output.iterdir()):
        raise FileExistsError(f"refusing to overwrite style macro evidence: {output}")
    output.mkdir(parents=True, exist_ok=True)
    head = bpy.data.objects.get("FaceProofHead")
    camera = bpy.data.objects.get("FaceProofCamera")
    left_eye, right_eye = bpy.data.objects.get("SculptEyeL"), bpy.data.objects.get("SculptEyeR")
    if not all((head, camera, left_eye, right_eye)):
        raise RuntimeError("style macro source objects are missing")
    if head.data.shape_keys.key_blocks.get("StyleMacroDirect") is not None:
        raise RuntimeError("StyleMacroDirect already exists")

    baseline = json.loads(Path(recipe["baselineMacro"]).read_text(encoding="utf-8"))
    mouth = head.data.shape_keys.key_blocks.get("mouth.width") if head.data.shape_keys else None
    if mouth is None:
        raise RuntimeError("source mouth.width shape key is missing")
    mouth.value = resolve_mouth_width(recipe["parameters"], mouth.value)
    mouth_narrow = resolve_mouth_narrow(recipe["parameters"])
    if mouth_narrow:
        target_path = Path(LocationService.get_mpfb_data("targets")) / "mouth" / "mouth-scale-horiz-decr.target.gz"
        if not target_path.is_file():
            raise FileNotFoundError(target_path)
        TargetService.load_target(
            head, str(target_path), weight=mouth_narrow, name="StyleMouthNarrow"
        )
    eye_keys = [
        head.data.shape_keys.key_blocks.get("sculpt.eye.left.height1"),
        head.data.shape_keys.key_blocks.get("sculpt.eye.right.height1"),
    ]
    if any(key is None for key in eye_keys):
        raise RuntimeError("source eye height shape keys are missing")
    for eye_key in eye_keys:
        eye_key.value = resolve_eye_height(recipe["parameters"], eye_key.value)
    narrow_keys = [
        head.data.shape_keys.key_blocks.get("eye.leftNarrow"),
        head.data.shape_keys.key_blocks.get("eye.rightNarrow"),
    ]
    if any(key is None for key in narrow_keys):
        raise RuntimeError("source eye narrow shape keys are missing")
    narrow_values = resolve_eye_narrows(
        recipe["parameters"], narrow_keys[0].value, narrow_keys[1].value
    )
    for narrow_key, value in zip(narrow_keys, narrow_values):
        narrow_key.value = value
    for key_name, parameter_name in (
        ("age.eyeBagsLeft", "eyeBagLeft"),
        ("age.eyeBagsRight", "eyeBagRight"),
        ("sculpt.upperlip.volume.incr", "upperLipVolumeIncrease"),
        ("sculpt.lowerlip.volume.incr", "lowerLipVolumeIncrease"),
    ):
        shape = head.data.shape_keys.key_blocks.get(key_name)
        if shape is None:
            raise RuntimeError(f"source presentation shape key is missing: {key_name}")
        shape.value = bounded_parameter(recipe["parameters"], parameter_name, shape.value)
    for parameter_name, target_name, key_name in (
        ("upperLipVolumeDecrease", "mouth-upperlip-volume-decr.target.gz", "StyleUpperLipDecrease"),
        ("lowerLipVolumeDecrease", "mouth-lowerlip-volume-decr.target.gz", "StyleLowerLipDecrease"),
    ):
        weight = bounded_parameter(recipe["parameters"], parameter_name, 0.0)
        if weight:
            target_path = Path(LocationService.get_mpfb_data("targets")) / "mouth" / target_name
            if not target_path.is_file():
                raise FileNotFoundError(target_path)
            TargetService.load_target(head, str(target_path), weight=weight, name=key_name)
    eye_scales = resolve_eye_scale_decreases(recipe["parameters"])
    if any(eye_scales):
        target_root = Path(LocationService.get_mpfb_data("targets")) / "eyes"
        for side, name, eye_scale in (
            ("l", "StyleEyeScaleLeft", eye_scales[0]),
            ("r", "StyleEyeScaleRight", eye_scales[1]),
        ):
            if not eye_scale:
                continue
            target_path = target_root / f"{side}-eye-scale-decr.target.gz"
            if not target_path.is_file():
                raise FileNotFoundError(target_path)
            TargetService.load_target(head, str(target_path), weight=eye_scale, name=name)
    current = mixed_coordinates(head)
    eligible = body_indices(head)
    center_x = (left_eye.location.x + right_eye.location.x) * 0.5
    eye_z = (left_eye.location.z + right_eye.location.z) * 0.5
    chin_z = baseline["anchors"]["chin"]["world"][2]
    top_z = max(current[index].z for index in eligible)
    cheek_x = [baseline["anchors"][name]["world"][0] for name in ("leftCheek", "rightCheek")]
    side_radius = sum(abs(value - center_x) for value in cheek_x) / 2.0
    bands = baseline["contourBands"]
    parameters = recipe["parameters"]
    deformed = [point.copy() for point in current]
    maximum = 0.0
    maximum_y = 0.0
    for index in eligible:
        changed = deform_point(
            tuple(current[index]), center_x=center_x, eye_z=eye_z, chin_z=chin_z,
            top_z=top_z, side_radius=side_radius,
            upper_height_scale=parameters["upperHeightScale"],
            lower_height_scale=parameters["lowerHeightScale"],
            cheek_scale=parameters["cheekScale"], jaw_scale=parameters["jawScale"],
            cheek_fraction=bands["cheekFraction"], jaw_fraction=bands["jawFraction"],
            lateral_power=parameters["lateralPower"],
        )
        deformed[index][:] = changed
        delta = deformed[index] - current[index]
        maximum = max(maximum, delta.length)
        maximum_y = max(maximum_y, abs(delta.y))

    key = head.shape_key_add(name="StyleMacroDirect", from_mix=False)
    basis = head.data.shape_keys.key_blocks[0]
    for index, (before, after) in enumerate(zip(current, deformed)):
        key.data[index].co = basis.data[index].co + (after - before)
    key.value = 1.0
    head["style_macro_recipe"] = json.dumps(parameters, sort_keys=True)
    apply_presentation(head, parameters)

    blend = output / "face-style-macro.blend"
    bpy.ops.wm.save_as_mainfile(filepath=str(blend))
    target = (center_x, -0.01, camera.location.z)
    render_views(camera, target, center_x, output)
    bpy.ops.wm.save_as_mainfile(filepath=str(blend))
    artifacts = [blend, *(output / name for name in ("front.png", "three-quarter.png", "profile.png"))]
    manifest = {
        "schemaVersion": 1, "status": "built-unverified", "shapeKey": key.name,
        "sourceBlend": {"path": recipe["sourceBlend"], "sha256": sha256(recipe["sourceBlend"])},
        "recipe": {"path": str(recipe_path), "sha256": sha256(recipe_path), "parameters": parameters},
        "baselineMacro": {"path": recipe["baselineMacro"], "sha256": sha256(recipe["baselineMacro"])},
        "targetGeometry": {"path": recipe["targetGeometry"], "sha256": sha256(recipe["targetGeometry"])},
        "topology": {"vertexCount": len(head.data.vertices), "faceCount": len(head.data.polygons), "sha256": topology_hash(head)},
        "maximumDisplacement": maximum, "maximumYDisplacement": maximum_y,
        "artifacts": {path.name: sha256(path) for path in artifacts},
        "toolVersions": {"blender": bpy.app.version_string, "python": sys.version.split()[0],
                         "builderSha256": sha256(Path(__file__).resolve()), "deformerSha256": sha256(ROOT / "style_macro.py")},
    }
    (output / "manifest.json").write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(f"STYLE_MACRO_BUILD_OK output={output} maxDisplacement={maximum:.8f}")


if __name__ == "__main__":
    main()
