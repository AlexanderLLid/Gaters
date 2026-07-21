import json
import math
import sys
from pathlib import Path

import hou


arguments = list(map(Path, sys.argv[1:]))
if len(arguments) == 3:
    body_run, recipe_path, output_dir = arguments
    guide = None
elif len(arguments) == 4:
    body_run, guide_run, recipe_path, output_dir = arguments
    guide = json.loads((guide_run / "guide.json").read_text(encoding="utf-8"))
else:
    raise SystemExit("usage: hython houdini_smooth_body_build.py BODY_RUN [GUIDE_RUN] RECIPE OUTPUT")
body = json.loads((body_run / "composed-mesh.json").read_text(encoding="utf-8"))
recipe = json.loads(recipe_path.read_text(encoding="utf-8"))
output_dir.mkdir(parents=True, exist_ok=True)


def add_ellipsoid(geometry, center, radii, module, longitude=24, latitude=16):
    geometry.addAttrib(hou.attribType.Prim, "source_module", "") if geometry.findPrimAttrib("source_module") is None else None
    north = geometry.createPoint()
    north.setPosition((center[0], center[1], center[2] + radii[2]))
    south = geometry.createPoint()
    south.setPosition((center[0], center[1], center[2] - radii[2]))
    rings = []
    for lat in range(1, latitude):
        phi = math.pi * lat / latitude
        ring = []
        for lon in range(longitude):
            theta = math.tau * lon / longitude
            point = geometry.createPoint()
            point.setPosition((
                center[0] + radii[0] * math.sin(phi) * math.cos(theta),
                center[1] + radii[1] * math.sin(phi) * math.sin(theta),
                center[2] + radii[2] * math.cos(phi),
            ))
            ring.append(point)
        rings.append(ring)

    def polygon(points):
        primitive = geometry.createPolygon(is_closed=True)
        for point in points:
            primitive.addVertex(point)
        primitive.setAttribValue("source_module", module)

    for lon in range(longitude):
        polygon([north, rings[0][lon], rings[0][(lon + 1) % longitude]])
        polygon([south, rings[-1][(lon + 1) % longitude], rings[-1][lon]])
    for lat in range(len(rings) - 1):
        for lon in range(longitude):
            polygon([rings[lat][lon], rings[lat + 1][lon], rings[lat + 1][(lon + 1) % longitude], rings[lat][(lon + 1) % longitude]])


source_geometry = hou.Geometry()
module_specs = []
overlap = recipe["module_overlap_m"]
if guide is None:
    scale = body["cell_size_m"]
    for module, placement in body["placements"].items():
        minimum, maximum = placement["bounds_cells"]
        center = [((minimum[axis] + maximum[axis]) / 2) * scale for axis in range(3)]
        radii = [((maximum[axis] - minimum[axis]) / 2) * scale + overlap for axis in range(3)]
        module_specs.append({"id": module, "role": placement["role"], "center": center, "radii": radii})
        add_ellipsoid(source_geometry, center, radii, module)
else:
    for ellipsoid in guide["surface_ellipsoids"]:
        radii = [value + overlap for value in ellipsoid["radii"]]
        spec = {"id": ellipsoid["module"], "role": ellipsoid["role"], "center": ellipsoid["center"], "radii": radii}
        module_specs.append(spec)
        add_ellipsoid(source_geometry, spec["center"], radii, spec["id"])
    for segment in guide["surface_segments"]:
        start = guide["landmarks"][segment["start"]]
        end = guide["landmarks"][segment["end"]]
        for index in range(segment["samples"]):
            t = index / (segment["samples"] - 1)
            center = [start[axis] + t * (end[axis] - start[axis]) for axis in range(3)]
            radius = segment["start_radius_m"] + t * (segment["end_radius_m"] - segment["start_radius_m"]) + overlap
            spec = {"id": segment["module"], "role": segment["role"], "center": center, "radii": [radius, radius, radius]}
            module_specs.append(spec)
            add_ellipsoid(source_geometry, center, spec["radii"], spec["id"], longitude=16, latitude=12)

hou.hipFile.clear(suppress_save_prompt=True)
container = hou.node("/obj").createNode("geo", "SMOOTH_BODY")
for child in container.children():
    child.destroy()
source = container.createNode("stash", "MODULE_PRIMITIVES")
source.parm("stash").set(source_geometry)
surface_vdb = container.createNode("vdbfrompolygons", "SURFACE_VDB")
surface_vdb.setInput(0, source)
surface_vdb.parm("voxelsize").set(recipe["voxel_size_m"])
surface_vdb.parm("fillinterior").set(1)
smooth = container.createNode("vdbsmoothsdf", "SURFACE_SMOOTH")
smooth.setInput(0, surface_vdb)
smooth.parm("iterations").set(recipe["smooth_iterations"])
polygons = container.createNode("convertvdb", "SURFACE_POLYGONS")
polygons.setInput(0, smooth)
polygons.parm("conversion").set("poly")
polygons.parm("adaptivity").set(recipe["adaptivity"])
reduce = container.createNode("polyreduce", "SURFACE_REDUCE")
reduce.setInput(0, polygons)
reduce.parm("target").set("poly_count")
reduce.parm("finalcount").set(recipe["target_faces"])
reduce.parm("reducepassedtarget").set(1)

checks = []
for spec in module_specs:
    center = ",".join(f"{value:.12g}" for value in spec["center"])
    radii = ",".join(f"{value:.12g}" for value in spec["radii"])
    checks.append(
        f'{{ vector q = (pos-set({center}))/set({radii}); float d=length(q)-1; '
        f'if(d<best){{best=d; id="{spec["id"]}"; role="{spec["role"]}";}} }}'
    )
selector = "\n".join(checks)
point_labels = container.createNode("attribwrangle", "SURFACE_LABELS")
point_labels.setInput(0, reduce)
point_labels.parm("class").set(2)
point_labels.parm("snippet").set(
    "vector pos=@P; float best=1e9; string id=\"\"; string role=\"\";\n"
    + selector
    + "\ns@module=id; f@core=role==\"core\"; f@head=role==\"head\"; f@limb=role==\"limb\";"
    + "\nif(role==\"core\") @Cd=set(0.46,0.55,0.63); else if(role==\"head\") @Cd=set(0.68,0.51,0.36); else @Cd=set(0.39,0.58,0.49);"
)
face_labels = container.createNode("attribwrangle", "FACE_LABELS")
face_labels.setInput(0, point_labels)
face_labels.parm("class").set(1)
face_labels.parm("snippet").set(
    "vector pos=@P; float best=1e9; string id=\"\"; string role=\"\";\n"
    + selector
    + "\ns@module=id;"
)
normals = container.createNode("normal", "SURFACE_NORMALS")
normals.setInput(0, face_labels)
output = container.createNode("null", "OUT_SMOOTH_BODY")
output.setInput(0, normals)
output.setDisplayFlag(True)
output.setRenderFlag(True)

container.setUserData("body_metadata", json.dumps({key: body[key] for key in ("body_plan_id", "placements", "connections", "cells", "mirrors")}, sort_keys=True))
container.setUserData("parameters", json.dumps({key: recipe[key] for key in ("voxel_size_m", "smooth_iterations", "adaptivity", "module_overlap_m", "target_faces")}, sort_keys=True))
container.setUserData("surface_recipe", json.dumps(recipe, sort_keys=True))
if guide is not None:
    container.setUserData("guide_metadata", json.dumps(guide, sort_keys=True))
container.layoutChildren()
scene_path = output_dir / "smooth-body.hipnc"
hou.hipFile.save(str(scene_path))
geometry = output.geometry()
print(f"HOUDINI_SMOOTH_BODY_BUILD_PASS scene={scene_path} points={len(geometry.points())} faces={len(geometry.prims())}")
