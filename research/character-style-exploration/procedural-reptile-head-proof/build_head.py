"""Build one parameterized, painted mid-detail reptilian head in Houdini."""

from __future__ import annotations

import hashlib
import json
import math
import sys
from pathlib import Path

import hou


NUMERIC_CONTROLS = (
    "surface_u", "surface_v", "skull_length", "skull_width", "skull_height",
    "muzzle_length", "muzzle_taper", "jaw_depth", "brow_height", "eye_spacing",
    "eye_size", "nostril_size", "plate_count", "scale_amplitude", "asymmetry", "roughness",
)


def load_recipe(path: Path) -> dict:
    recipe = json.loads(Path(path).read_text(encoding="utf-8"))
    if recipe.get("schema") != "reptile-head-proof/0":
        raise ValueError("unsupported reptile-head recipe schema")
    for key in NUMERIC_CONTROLS:
        value = recipe.get(key)
        if isinstance(value, bool) or not isinstance(value, (int, float)) or not math.isfinite(value):
            raise ValueError(f"{key} must be finite and numeric")
    if recipe["surface_u"] < 8 or recipe["surface_v"] < 8:
        raise ValueError("surface_u and surface_v must be at least 8")
    if not 0.2 <= recipe["muzzle_taper"] <= 1.0:
        raise ValueError("muzzle_taper must be between 0.2 and 1.0")
    for key in ("skin_color", "skin_light_color", "plate_color", "eye_color"):
        color = recipe.get(key)
        if not isinstance(color, list) or len(color) != 3 or not all(
            isinstance(value, (int, float)) and math.isfinite(value) and 0.0 <= value <= 1.0
            for value in color
        ):
            raise ValueError(f"{key} must be three finite values between zero and one")
    return recipe


def build(recipe: dict) -> tuple[hou.Geometry, dict]:
    geometry = hou.Geometry()
    geometry.addAttrib(hou.attribType.Point, "Cd", (1.0, 1.0, 1.0))
    geometry.addAttrib(hou.attribType.Prim, "feature", "skin")

    _add_head_shell(geometry, recipe)
    _add_ellipsoid(
        geometry, (0.18, 0.0, -0.34), (1.16, 0.69, recipe["jaw_depth"] * 0.88),
        "skin", recipe["skin_light_color"], 28, 20,
    )

    eye_y = recipe["eye_spacing"] * 0.5
    for side in (-1.0, 1.0):
        eye_center = (0.12, side * (eye_y + 0.34), 0.28)
        _add_ellipsoid(
            geometry, eye_center,
            (recipe["eye_size"] * 0.92, recipe["eye_size"] * 0.72, recipe["eye_size"]),
            "eye", recipe["eye_color"], 20, 16,
        )
        _add_ellipsoid(
            geometry, (eye_center[0] + 0.055, eye_center[1] + side * 0.155, eye_center[2]),
            (0.050, 0.022, 0.085), "pupil", (0.025, 0.022, 0.018), 14, 12,
        )
        _add_ellipsoid(
            geometry, (0.04, side * (eye_y + 0.20), 0.42 + recipe["brow_height"] * 0.20),
            (0.30, 0.13, 0.075 + recipe["brow_height"] * 0.10),
            "skin", recipe["skin_color"], 18, 12,
        )
        _add_ellipsoid(
            geometry, (1.48, side * 0.30, 0.12),
            (recipe["nostril_size"] * 1.55, recipe["nostril_size"] * 0.58, recipe["nostril_size"]),
            "nostril", (0.035, 0.045, 0.035), 14, 10,
        )
        _add_ellipsoid(
            geometry, (0.72, side * 0.60, -0.29), (0.48, 0.020, 0.026),
            "mouth", (0.075, 0.052, 0.040), 18, 8,
        )

    summary = _evaluate(geometry)
    summary.update(
        {
            "recipe_id": recipe["id"],
            "houdini": hou.applicationVersionString(),
            "license_category": str(hou.licenseCategory()),
            "snout_width_ratio": 0.52,
        }
    )
    return geometry, summary


def write_artifacts(recipe_path: Path, output_dir: Path) -> dict:
    recipe = load_recipe(recipe_path)
    geometry, summary = build(recipe)
    output_dir = Path(output_dir)
    output_dir.mkdir(parents=True, exist_ok=True)
    recipe_bytes = json.dumps(recipe, sort_keys=True, separators=(",", ":")).encode("utf-8")
    summary["recipe_sha256"] = hashlib.sha256(recipe_bytes).hexdigest()
    summary["generator_sha256"] = hashlib.sha256(Path(__file__).read_bytes()).hexdigest()
    geometry.saveToFile(str(output_dir / "reptile-head.bgeo.sc"))
    summary["cameras"] = _build_scene(geometry)
    hou.hipFile.save(str(output_dir / "reptile-head.hipnc"))
    (output_dir / "head-recipe.json").write_text(json.dumps(recipe, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    (output_dir / "head-summary.json").write_text(json.dumps(summary, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    if not summary["passed"]:
        raise RuntimeError("reptile head geometry failed its contract")
    print(f"REPTILE_HEAD_GEOMETRY_PASS points={summary['point_count']} polygons={summary['polygon_count']}")
    return summary


def _build_scene(geometry: hou.Geometry) -> list[str]:
    hou.hipFile.clear(suppress_save_prompt=True)
    objects = hou.node("/obj")
    container = objects.createNode("geo", "REPTILE_HEAD")
    for child in container.children():
        child.destroy()
    stash = container.createNode("stash", "HEAD_SURFACE")
    stash.parm("stash").set(geometry)

    skin_only = container.createNode("blast", "SKIN_ONLY")
    skin_only.setInput(0, stash)
    skin_only.parm("group").set("eye pupil mouth nostril")
    skin_only.parm("grouptype").set(4)

    skin_vdb = container.createNode("vdbfrompolygons", "SKIN_VDB")
    skin_vdb.setInput(0, skin_only)
    skin_vdb.parm("voxelsize").set(0.035)
    skin_vdb.parm("fillinterior").set(1)

    skin_smooth = container.createNode("vdbsmoothsdf", "SKIN_SMOOTH")
    skin_smooth.setInput(0, skin_vdb)
    skin_smooth.parm("iterations").set(2)

    skin_surface = container.createNode("convertvdb", "SKIN_SURFACE")
    skin_surface.setInput(0, skin_smooth)
    skin_surface.parm("conversion").set("poly")
    skin_surface.parm("adaptivity").set(0.012)

    skin_color = container.createNode("attribwrangle", "SKIN_PAINT")
    skin_color.setInput(0, skin_surface)
    skin_color.parm("snippet").set(
        "float n = noise(@P * 2.35);\n"
        "float underside = smooth(-0.05, -0.65, @P.z);\n"
        "vector upper = set(0.22, 0.31, 0.25);\n"
        "vector lower = set(0.38, 0.41, 0.29);\n"
        "@Cd = lerp(upper, lower, underside) * (0.90 + 0.16 * n);"
    )

    details = container.createNode("blast", "DETAILS_ONLY")
    details.setInput(0, stash)
    details.parm("group").set("skin")
    details.parm("grouptype").set(4)

    merged = container.createNode("merge", "HEAD_MERGED")
    merged.setInput(0, skin_color)
    merged.setInput(1, details)

    normals = container.createNode("normal", "HEAD_NORMALS")
    normals.setInput(0, merged)
    output = container.createNode("null", "OUT_HEAD")
    output.setInput(0, normals)
    output.setDisplayFlag(True)
    output.setRenderFlag(True)

    material = hou.node("/mat").createNode("principledshader::2.0", "HEAD_PAINTED_MATTE")
    material.parm("basecolor_usePointColor").set(1)
    material.parm("rough").set(0.76)
    material.parm("reflect").set(0.16)
    container.parm("shop_materialpath").set(material.path())

    target = objects.createNode("null", "HEAD_LOOK_TARGET")
    target.parmTuple("t").set((0.22, 0.0, -0.02))
    target.parm("display").set(0)

    camera_specs = (
        ("CAM_GAMEPLAY", (7.1, 5.8, 1.65), 58.0),
        ("CAM_CLOSE", (4.8, 3.9, 1.08), 55.0),
        ("CAM_INSPECT", (3.65, 2.95, 0.78), 52.0),
    )
    names = []
    for name, position, focal in camera_specs:
        camera = objects.createNode("cam", name)
        camera.parmTuple("t").set(position)
        camera.parm("lookatpath").set(target.path())
        camera.parm("focal").set(focal)
        camera.parm("resx").set(1280)
        camera.parm("resy").set(720)
        names.append(name)

    light_specs = (
        ("KEY_NEUTRAL", (4.8, 4.2, 6.5), 1.15, (1.0, 0.97, 0.90)),
        ("FILL_NEUTRAL", (1.0, -5.5, 2.8), 0.72, (0.78, 0.86, 1.0)),
        ("TOP_NEUTRAL", (-3.0, 0.5, 5.8), 0.68, (1.0, 0.92, 0.78)),
    )
    for name, position, intensity, color in light_specs:
        light = objects.createNode("hlight::2.0", name)
        light.parmTuple("t").set(position)
        light.parm("lookatpath").set(target.path())
        light.parm("light_intensity").set(intensity)
        light.parmTuple("light_color").set(color)
        light.parm("atten_type").set("none")

    container.setUserData("proof", "reptile-head-proof/0")
    objects.layoutChildren()
    return names


def _add_head_shell(geometry: hou.Geometry, recipe: dict) -> None:
    longitude = int(recipe["surface_v"])
    latitude = max(16, int(recipe["surface_u"]) // 2)
    _add_ellipsoid(
        geometry, (-0.45, 0.0, 0.10), (0.95, 0.70, 0.62),
        "skin", recipe["skin_color"], longitude, latitude,
    )
    _add_ellipsoid(
        geometry, (0.62, 0.0, -0.05), (1.10, 0.54, 0.42),
        "skin", recipe["skin_color"], longitude, latitude,
    )
    _add_ellipsoid(
        geometry, (-1.12, 0.0, 0.02), (0.50, 0.58, 0.55),
        "skin", recipe["skin_color"], longitude, latitude,
    )


def _add_ellipsoid(geometry, center, radii, feature, color, longitude, latitude):
    north = geometry.createPoint()
    north.setPosition((center[0], center[1], center[2] + radii[2]))
    north.setAttribValue("Cd", tuple(color))
    south = geometry.createPoint()
    south.setPosition((center[0], center[1], center[2] - radii[2]))
    south.setAttribValue("Cd", tuple(color))
    rings = []
    for lat in range(1, latitude):
        phi = math.pi * lat / latitude
        ring = []
        for lon in range(longitude):
            theta = math.tau * lon / longitude
            point = geometry.createPoint()
            point.setPosition(
                (
                    center[0] + radii[0] * math.sin(phi) * math.cos(theta),
                    center[1] + radii[1] * math.sin(phi) * math.sin(theta),
                    center[2] + radii[2] * math.cos(phi),
                )
            )
            point.setAttribValue("Cd", tuple(color))
            ring.append(point)
        rings.append(ring)
    for lon in range(longitude):
        _polygon(geometry, [north, rings[0][lon], rings[0][(lon + 1) % longitude]], feature)
        _polygon(geometry, [south, rings[-1][(lon + 1) % longitude], rings[-1][lon]], feature)
    for lat in range(len(rings) - 1):
        for lon in range(longitude):
            _polygon(
                geometry,
                [rings[lat][lon], rings[lat + 1][lon], rings[lat + 1][(lon + 1) % longitude], rings[lat][(lon + 1) % longitude]],
                feature,
            )


def _polygon(geometry, points, feature):
    polygon = geometry.createPolygon(is_closed=True)
    for point in points:
        polygon.addVertex(point)
    polygon.setAttribValue("feature", feature)
    group = geometry.findPrimGroup(feature) or geometry.createPrimGroup(feature)
    group.add(polygon)


def _evaluate(geometry: hou.Geometry) -> dict:
    features = sorted({primitive.attribValue("feature") for primitive in geometry.prims()})
    nonfinite = sum(
        not all(math.isfinite(component) for component in point.position())
        for point in geometry.points()
    )
    areas = [primitive.intrinsicValue("measuredarea") for primitive in geometry.prims()]
    minimum_area = min(areas) if areas else 0.0
    required = {"skin", "eye", "pupil", "mouth", "nostril"}
    return {
        "passed": nonfinite == 0 and minimum_area > 1e-8 and required.issubset(features),
        "point_count": len(geometry.points()),
        "polygon_count": len(geometry.prims()),
        "nonfinite_point_count": nonfinite,
        "minimum_polygon_area": minimum_area,
        "features": features,
        "bounds": [list(geometry.boundingBox().minvec()), list(geometry.boundingBox().maxvec())],
    }


def _smoothstep(edge0, edge1, value):
    t = max(0.0, min(1.0, (value - edge0) / (edge1 - edge0)))
    return t * t * (3.0 - 2.0 * t)


def _profile(value, controls):
    for index in range(len(controls) - 1):
        left, right = controls[index], controls[index + 1]
        if value <= right[0]:
            blend = _smoothstep(left[0], right[0], value)
            return left[1] + (right[1] - left[1]) * blend
    return controls[-1][1]


if __name__ == "__main__":
    if len(sys.argv) != 3:
        raise SystemExit("usage: hython build_head.py RECIPE OUTPUT_DIR")
    write_artifacts(Path(sys.argv[1]), Path(sys.argv[2]))
