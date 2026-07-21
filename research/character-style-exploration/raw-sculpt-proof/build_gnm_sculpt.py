"""Generate an anatomical design head from Google GNM and render it in Blender."""

import argparse
import hashlib
import json
import math
import sys
from pathlib import Path

import bpy
from mathutils import Vector


ROOT = Path(__file__).resolve().parent


def sha256(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def dense_identity(decoder_path, seed):
    import h5py
    import numpy as np

    rng = np.random.default_rng(seed)
    latent = rng.normal(size=(1, 64)).astype("float32")
    # female=[1,0], Black=[0,0,0,1]
    value = np.concatenate([latent, np.array([[1, 0, 0, 0, 0, 1]], dtype="float32")], axis=1)
    with h5py.File(decoder_path, "r") as handle:
        for index, name in enumerate(("dense_4", "dense_5", "dense_6", "dense_7", "dense_8")):
            group = handle[f"model_weights/{name}/{name}"]
            value = value @ group["kernel:0"][:] + group["bias:0"][:]
            if index < 4:
                value = np.maximum(value, 0.0)
    return value[0]


def material(name, color, roughness):
    value = bpy.data.materials.new(name)
    value.diffuse_color = (*color, 1.0)
    value.use_nodes = True
    bsdf = value.node_tree.nodes.get("Principled BSDF")
    bsdf.inputs["Base Color"].default_value = (*color, 1.0)
    bsdf.inputs["Roughness"].default_value = roughness
    return value


def look_at(obj, target):
    obj.rotation_euler = (Vector(target) - obj.location).to_track_quat("-Z", "Y").to_euler()


def camera(name, location):
    data = bpy.data.cameras.new(name)
    data.type = "ORTHO"
    data.ortho_scale = 2.85
    obj = bpy.data.objects.new(name, data)
    bpy.context.collection.objects.link(obj)
    obj.location = location
    look_at(obj, (0.0, 0.0, 0.0))
    return obj


def area(name, location, energy, size):
    data = bpy.data.lights.new(name, "AREA")
    data.energy = energy
    data.size = size
    obj = bpy.data.objects.new(name, data)
    bpy.context.collection.objects.link(obj)
    obj.location = location
    look_at(obj, (0.0, 0.0, 0.0))


def build(contract_path, gnm_root, dependency_root):
    contract_path = Path(contract_path).resolve()
    contract = json.loads(contract_path.read_text(encoding="utf-8"))
    output = (contract_path.parent / contract["outputDirectory"]).resolve()
    output.mkdir(parents=True, exist_ok=True)
    sys.path[:0] = [str(Path(dependency_root).resolve()), str(Path(gnm_root).resolve())]

    import numpy as np
    from gnm.shape import gnm_numpy

    gnm = gnm_numpy.GNM.from_local(
        version=gnm_numpy.GNMMajorVersion.V3,
        variant=gnm_numpy.GNMVariant.HEAD,
    )
    decoder = Path(gnm_root) / "gnm/shape/data/semantic_sampler/identity_decoder_model.h5"
    identity = dense_identity(decoder, int(contract["identity"]["seed"]))
    source_vertices = np.asarray(gnm(identity=identity), dtype=np.float64)
    if source_vertices.ndim == 3:
        source_vertices = source_vertices[0]
    source_faces = np.asarray(gnm.triangles, dtype=np.int32)

    # GNM: X horizontal, Y vertical, Z depth. Blender: X horizontal, Z vertical,
    # negative Y toward the front camera.
    vertices = np.column_stack((source_vertices[:, 0], -source_vertices[:, 2], source_vertices[:, 1]))
    skin_indices = gnm.vertex_group_indices("skin")
    center = (vertices[skin_indices].min(axis=0) + vertices[skin_indices].max(axis=0)) * 0.5
    vertices -= center
    vertices *= 7.6

    bpy.ops.wm.read_factory_settings(use_empty=True)
    mesh = bpy.data.meshes.new("GNMDesignSculptMesh")
    mesh.from_pydata(vertices.tolist(), [], source_faces.tolist())
    mesh.update()
    head = bpy.data.objects.new("DesignSculpt", mesh)
    bpy.context.collection.objects.link(head)
    for polygon in mesh.polygons:
        polygon.use_smooth = True

    skin = material("Skin clay", (0.29, 0.085, 0.038), 0.78)
    sclera = material("Sclera", (0.52, 0.48, 0.40), 0.55)
    iris = material("Iris", (0.055, 0.025, 0.012), 0.50)
    mouth = material("Mouth", (0.16, 0.025, 0.020), 0.68)
    teeth = material("Teeth", (0.55, 0.50, 0.41), 0.62)
    for value in (skin, sclera, iris, mouth, teeth):
        mesh.materials.append(value)

    material_groups = (("scleras", 1), ("irises", 2), ("mouth_sock", 3), ("teeth", 4))
    face_sets = {
        slot: set(gnm.triangle_indices_for_group(group).tolist())
        for group, slot in material_groups
    }
    for face_index, polygon in enumerate(mesh.polygons):
        for slot, indices in face_sets.items():
            if face_index in indices:
                polygon.material_index = slot
                break

    head["artifactRole"] = "anatomical-design-volume"
    head["anatomySource"] = contract["anatomySource"]["repository"]
    head["anatomyCommit"] = contract["anatomySource"]["commit"]
    head["identitySeed"] = contract["identity"]["seed"]

    world = bpy.data.worlds.new("Neutral world")
    bpy.context.scene.world = world
    world.use_nodes = True
    world.node_tree.nodes["Background"].inputs["Color"].default_value = (0.012, 0.014, 0.018, 1.0)
    world.node_tree.nodes["Background"].inputs["Strength"].default_value = 0.35
    area("Key", (-3.8, -4.5, 4.5), 850, 4.0)
    area("Fill", (3.5, -3.0, 1.8), 450, 3.5)
    area("Rim", (0.0, 2.8, 3.0), 600, 3.0)

    cameras = {
        "front": camera("Camera.Front", (0.0, -7.0, 0.0)),
        "three-quarter": camera("Camera.ThreeQuarter", (4.8, -6.0, 0.1)),
        "profile": camera("Camera.Profile", (7.0, 0.0, 0.0)),
    }
    scene = bpy.context.scene
    scene.render.engine = "BLENDER_EEVEE"
    scene.render.resolution_x = 768
    scene.render.resolution_y = 768
    scene.render.resolution_percentage = 100
    scene.render.image_settings.file_format = "PNG"
    scene.view_settings.look = "AgX - Medium High Contrast"

    blend_path = output / "raw-head.blend"
    bpy.ops.wm.save_as_mainfile(filepath=str(blend_path))
    for name, camera_obj in cameras.items():
        scene.camera = camera_obj
        scene.render.filepath = str(output / f"{name}.png")
        bpy.ops.render.render(write_still=True)

    manifest = {
        "schemaVersion": 1,
        "machineId": contract["machineId"],
        "status": "human-review",
        "input": {
            "targetImage": str((contract_path.parent / contract["targetImage"]).resolve()),
            "targetSha256": sha256((contract_path.parent / contract["targetImage"]).resolve()),
            "anatomySource": contract["anatomySource"],
            "identity": contract["identity"],
            "decoderSha256": sha256(decoder),
        },
        "output": {
            "blend": str(blend_path),
            "vertices": len(mesh.vertices),
            "faces": len(mesh.polygons),
        },
        "usesMpfb": False,
        "usesImageProjection": False,
        "claims": ["anatomically trained design volume", "editable Blender geometry"],
        "excludes": ["target likeness", "production topology approval", "texture", "rigging", "animation"],
        "artifacts": {},
    }
    for name in ("raw-head.blend", "front.png", "three-quarter.png", "profile.png"):
        manifest["artifacts"][name] = sha256(output / name)
    (output / "manifest.json").write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--contract", default=str(ROOT / "raw-sculpt-v3.json"))
    parser.add_argument("--gnm-root", required=True)
    parser.add_argument("--dependency-root", required=True)
    argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    args = parser.parse_args(argv)
    build(args.contract, args.gnm_root, args.dependency_root)


if __name__ == "__main__":
    main()
