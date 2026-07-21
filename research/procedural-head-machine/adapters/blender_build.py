import json
import sys
from pathlib import Path

import bpy


source_run, output_dir = map(Path, sys.argv[sys.argv.index("--") + 1 :])
mesh_data = json.loads((source_run / "mesh.json").read_text(encoding="utf-8"))
region_data = json.loads((source_run / "regions.json").read_text(encoding="utf-8"))
receipt = json.loads((source_run / "receipt.json").read_text(encoding="utf-8"))
output_dir.mkdir(parents=True, exist_ok=True)

bpy.ops.wm.read_factory_settings(use_empty=True)
mesh = bpy.data.meshes.new("ProceduralHeadMesh")
mesh.from_pydata(mesh_data["vertices"], [], mesh_data["faces"])
mesh.update()
head = bpy.data.objects.new("ProceduralHead", mesh)
bpy.context.collection.objects.link(head)
for polygon in mesh.polygons:
    polygon.use_smooth = True
for name in ("skull", "face", "jaw", "chin"):
    group = head.vertex_groups.new(name=name)
    for index, weights in enumerate(region_data["weights"]):
        if weights[name] > 0:
            group.add([index], weights[name], "REPLACE")
head["source_mesh_sha256"] = receipt["mesh_sha256"]
head["source_topology_sha256"] = receipt["topology_sha256"]
bpy.context.view_layer.objects.active = head
head.select_set(True)
bpy.ops.wm.save_as_mainfile(filepath=str(output_dir / "head.blend"))
print(f"BLENDER_HEAD_BUILD_PASS scene={output_dir / 'head.blend'}")
