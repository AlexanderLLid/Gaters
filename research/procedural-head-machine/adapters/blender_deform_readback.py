import json
import sys
from pathlib import Path

import bpy


output_path = Path(sys.argv[sys.argv.index("--") + 1])
head = bpy.data.objects["ProceduralHead"]
tree = head.modifiers["SemanticHeadDeformation"].node_group
amount = tree.nodes["Amount"].outputs[0].default_value
evaluated = head.evaluated_get(bpy.context.evaluated_depsgraph_get())
mesh = evaluated.to_mesh()
groups = {group.index: group.name for group in head.vertex_groups}
regions = []
for vertex in head.data.vertices:
    weights = {name: 0.0 for name in ("skull", "face", "jaw", "chin")}
    for membership in vertex.groups:
        weights[groups[membership.group]] = membership.weight
    regions.append(weights)
readback = {
    "schema": "head-deformation-readback/0",
    "backend": "blender-geometry-nodes",
    "version": bpy.app.version_string,
    "deformation_id": head["deformation_id"],
    "graph": {"operation": "widen_region", "amount": amount},
    "vertices": [list(vertex.co) for vertex in mesh.vertices],
    "faces": [list(polygon.vertices) for polygon in mesh.polygons],
    "regions": regions,
}
output_path.write_text(json.dumps(readback, indent=2, sort_keys=True) + "\n", encoding="utf-8")
evaluated.to_mesh_clear()
print(f"BLENDER_DEFORMATION_READBACK_PASS output={output_path}")
