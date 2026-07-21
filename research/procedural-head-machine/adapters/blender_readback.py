import json
import sys
from pathlib import Path

import bpy


output_path = Path(sys.argv[sys.argv.index("--") + 1])
head = bpy.data.objects["ProceduralHead"]
mesh = head.data
groups = {group.index: group.name for group in head.vertex_groups}
regions = []
for vertex in mesh.vertices:
    weights = {name: 0.0 for name in ("skull", "face", "jaw", "chin")}
    for membership in vertex.groups:
        weights[groups[membership.group]] = membership.weight
    regions.append(weights)
readback = {
    "schema": "head-adapter-readback/0",
    "backend": "blender",
    "version": bpy.app.version_string,
    "vertices": [list(vertex.co) for vertex in mesh.vertices],
    "faces": [list(polygon.vertices) for polygon in mesh.polygons],
    "regions": regions,
    "source_mesh_sha256": head["source_mesh_sha256"],
    "source_topology_sha256": head["source_topology_sha256"],
}
output_path.write_text(json.dumps(readback, indent=2, sort_keys=True) + "\n", encoding="utf-8")
print(f"BLENDER_HEAD_READBACK_PASS output={output_path}")
