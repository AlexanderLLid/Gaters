import json
import sys
from pathlib import Path

import bpy


source_run, command_path, output_dir = map(Path, sys.argv[sys.argv.index("--") + 1 :])
mesh_data = json.loads((source_run / "mesh.json").read_text(encoding="utf-8"))
region_data = json.loads((source_run / "regions.json").read_text(encoding="utf-8"))
command = json.loads(command_path.read_text(encoding="utf-8"))
output_dir.mkdir(parents=True, exist_ok=True)

bpy.ops.wm.read_factory_settings(use_empty=True)
mesh = bpy.data.meshes.new("ProceduralHeadMesh")
mesh.from_pydata(mesh_data["vertices"], [], mesh_data["faces"])
mesh.update()
head = bpy.data.objects.new("ProceduralHead", mesh)
bpy.context.collection.objects.link(head)
for name in ("skull", "face", "jaw", "chin"):
    group = head.vertex_groups.new(name=name)
    for index, weights in enumerate(region_data["weights"]):
        if weights[name] > 0:
            group.add([index], weights[name], "REPLACE")

tree = bpy.data.node_groups.new("SemanticHeadDeformation", "GeometryNodeTree")
tree.interface.new_socket(name="Geometry", in_out="INPUT", socket_type="NodeSocketGeometry")
tree.interface.new_socket(name="Geometry", in_out="OUTPUT", socket_type="NodeSocketGeometry")
nodes, links = tree.nodes, tree.links
group_input = nodes.new("NodeGroupInput")
group_output = nodes.new("NodeGroupOutput")
position = nodes.new("GeometryNodeInputPosition")
separate = nodes.new("ShaderNodeSeparateXYZ")
region = nodes.new("GeometryNodeInputNamedAttribute")
region.data_type = "FLOAT"
region.inputs["Name"].default_value = command["region"]
preserve = nodes.new("GeometryNodeInputNamedAttribute")
preserve.data_type = "FLOAT"
preserve.inputs["Name"].default_value = command["preserve"]
subtract = nodes.new("ShaderNodeMath")
subtract.operation = "SUBTRACT"
maximum = nodes.new("ShaderNodeMath")
maximum.operation = "MAXIMUM"
maximum.inputs[1].default_value = 0.0
amount = nodes.new("ShaderNodeValue")
amount.name = "Amount"
amount.label = "Amount"
amount.outputs[0].default_value = command["amount"]
scale_mask = nodes.new("ShaderNodeMath")
scale_mask.operation = "MULTIPLY"
offset_x = nodes.new("ShaderNodeMath")
offset_x.operation = "MULTIPLY"
combine = nodes.new("ShaderNodeCombineXYZ")
set_position = nodes.new("GeometryNodeSetPosition")

links.new(group_input.outputs["Geometry"], set_position.inputs["Geometry"])
links.new(position.outputs["Position"], separate.inputs["Vector"])
links.new(region.outputs["Attribute"], subtract.inputs[0])
links.new(preserve.outputs["Attribute"], subtract.inputs[1])
links.new(subtract.outputs[0], maximum.inputs[0])
links.new(maximum.outputs[0], scale_mask.inputs[0])
links.new(amount.outputs[0], scale_mask.inputs[1])
links.new(separate.outputs["X"], offset_x.inputs[0])
links.new(scale_mask.outputs[0], offset_x.inputs[1])
links.new(offset_x.outputs[0], combine.inputs["X"])
links.new(combine.outputs["Vector"], set_position.inputs["Offset"])
links.new(set_position.outputs["Geometry"], group_output.inputs["Geometry"])

modifier = head.modifiers.new(name="SemanticHeadDeformation", type="NODES")
modifier.node_group = tree
head["deformation_id"] = command["id"]
bpy.context.view_layer.objects.active = head
head.select_set(True)
bpy.ops.wm.save_as_mainfile(filepath=str(output_dir / "deformed-head.blend"))
print(f"BLENDER_DEFORMATION_BUILD_PASS scene={output_dir / 'deformed-head.blend'}")
