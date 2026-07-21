import math


def _positive_number(recipe, name):
    value = recipe.get(name)
    return isinstance(value, (int, float)) and math.isfinite(value) and value > 0


def _rounded(values):
    return [round(value, 12) for value in values]


def compile_parent(recipe):
    if (
        recipe.get("schema") != "branch-parent-box/0"
        or not all(_positive_number(recipe, name) for name in ("width_m", "depth_m", "height_m"))
    ):
        raise ValueError("BRANCH-PARENT-INPUT-1")
    x, y, z = (recipe["width_m"] / 2, recipe["depth_m"] / 2, recipe["height_m"] / 2)
    vertices = [
        [-x, -y, -z], [-x, y, -z], [-x, y, z], [-x, -y, z],
        [x, -y, -z], [x, y, -z], [x, y, z], [x, -y, z],
    ]
    return {
        "schema": "branch-parent-mesh/0",
        "recipe_id": recipe["id"],
        "vertices": vertices,
        "faces": [[0, 4, 5, 1], [0, 3, 7, 4], [3, 2, 6, 7], [1, 5, 6, 2]],
        "sockets": [
            {
                "name": "left",
                "interface": [0, 1, 2, 3],
                "origin": [-x, 0.0, 0.0],
                "tangent": [-1.0, 0.0, 0.0],
                "u": [0.0, 1.0, 0.0],
                "v": [0.0, 0.0, 1.0],
            },
            {
                "name": "right",
                "interface": [4, 5, 6, 7],
                "origin": [x, 0.0, 0.0],
                "tangent": [1.0, 0.0, 0.0],
                "u": [0.0, 1.0, 0.0],
                "v": [0.0, 0.0, 1.0],
            },
        ],
    }


def compose_mirrored_branches(parent, recipe):
    valid = (
        parent.get("schema") == "branch-parent-mesh/0"
        and len(parent.get("sockets", [])) == 2
        and recipe.get("schema") == "mirrored-branch-socket/0"
        and _positive_number(recipe, "branch_length_m")
        and isinstance(recipe.get("branch_rings"), int)
        and recipe["branch_rings"] >= 2
        and _positive_number(recipe, "end_scale")
    )
    if not valid:
        raise ValueError("BRANCH-SOCKET-INPUT-1")

    vertices = [list(vertex) for vertex in parent["vertices"]]
    faces = [list(face) for face in parent["faces"]]
    regions = [{"core": 1.0, "branch": 0.0} for _ in vertices]
    vertex_modules = ["core"] * len(vertices)
    face_modules = ["core"] * len(faces)
    interfaces = {}
    branch_rings = {}
    caps = {}

    for socket in parent["sockets"]:
        name = socket["name"]
        interface = socket["interface"]
        interfaces[name] = interface
        for index in interface:
            vertex_modules[index] = "core-branch-socket"
        rings = []
        for ring_index in range(1, recipe["branch_rings"] + 1):
            t = ring_index / recipe["branch_rings"]
            smooth = t * t * (3.0 - 2.0 * t)
            scale = 1.0 + smooth * (recipe["end_scale"] - 1.0)
            ring = []
            for interface_id in interface:
                seam = parent["vertices"][interface_id]
                relative = [seam[axis] - socket["origin"][axis] for axis in range(3)]
                local_u = sum(relative[axis] * socket["u"][axis] for axis in range(3))
                local_v = sum(relative[axis] * socket["v"][axis] for axis in range(3))
                point = [
                    socket["origin"][axis]
                    + socket["tangent"][axis] * recipe["branch_length_m"] * t
                    + socket["u"][axis] * local_u * scale
                    + socket["v"][axis] * local_v * scale
                    for axis in range(3)
                ]
                ring.append(len(vertices))
                vertices.append(_rounded(point))
                regions.append({"core": 0.0, "branch": 1.0})
                vertex_modules.append(f"{name}-branch")
            rings.append(ring)
        branch_rings[name] = rings

        for inner, outer in [(interface, rings[0])] + list(zip(rings, rings[1:])):
            for segment in range(4):
                next_segment = (segment + 1) % 4
                faces.append([inner[segment], inner[next_segment], outer[next_segment], outer[segment]])
                face_modules.append(f"{name}-branch")

        last = rings[-1]
        cap = len(vertices)
        vertices.append(_rounded([
            sum(vertices[index][axis] for index in last) / 4
            for axis in range(3)
        ]))
        regions.append({"core": 0.0, "branch": 1.0})
        vertex_modules.append(f"{name}-branch")
        caps[name] = cap
        for segment in range(4):
            next_segment = (segment + 1) % 4
            faces.append([cap, last[next_segment], last[segment]])
            face_modules.append(f"{name}-branch")

    left_ids = interfaces["left"] + [index for ring in branch_rings["left"] for index in ring] + [caps["left"]]
    right_ids = interfaces["right"] + [index for ring in branch_rings["right"] for index in ring] + [caps["right"]]
    return {
        "schema": "composed-character-mesh/0",
        "source_recipe_id": parent["recipe_id"],
        "socket_id": recipe["id"],
        "vertices": vertices,
        "faces": faces,
        "regions": regions,
        "vertex_modules": vertex_modules,
        "face_modules": face_modules,
        "socket": {
            "interfaces": interfaces,
            "frames": {socket["name"]: {key: socket[key] for key in ("origin", "tangent", "u", "v")} for socket in parent["sockets"]},
            "branch_rings": branch_rings,
            "caps": caps,
            "mirror_pairs": list(zip(left_ids, right_ids)),
        },
    }
