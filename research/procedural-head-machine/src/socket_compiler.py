import math


REGIONS = ("skull", "face", "jaw", "chin", "neck")


def _rounded(values):
    return [round(value, 12) for value in values]


def compose_head_neck(mesh, region_artifact, socket):
    interface = socket.get("interface_vertices", [])
    previous = socket.get("previous_vertices", [])
    valid = (
        socket.get("schema") == "module-socket/0"
        and len(interface) == len(previous)
        and len(interface) >= 3
        and interface == list(range(interface[0], interface[0] + len(interface)))
        and previous == list(range(previous[0], previous[0] + len(previous)))
        and max(previous) < min(interface)
        and socket.get("neck_rings", 0) >= 2
        and all(
            isinstance(socket.get(name), (int, float))
            and math.isfinite(socket[name])
            and socket[name] > 0
            for name in ("neck_length_m", "first_tangent_scale", "end_scale_x", "end_scale_y")
        )
    )
    if not valid:
        raise ValueError("SOCKET-INPUT-1")

    interface_end = max(interface)
    if interface_end >= len(mesh["vertices"]) or max(previous) >= len(mesh["vertices"]):
        raise ValueError("SOCKET-INPUT-1")
    vertices = [list(vertex) for vertex in mesh["vertices"][: interface_end + 1]]
    faces = [
        list(face) for face in mesh["faces"]
        if all(vertex <= interface_end for vertex in face)
    ]
    regions = [
        {**{name: weights.get(name, 0.0) for name in REGIONS}, "neck": 0.0}
        for weights in region_artifact["weights"][: interface_end + 1]
    ]
    vertex_modules = ["head"] * len(vertices)
    for index in interface:
        vertex_modules[index] = "head-neck-socket"
    face_modules = ["head"] * len(faces)

    tangent_scale = socket["first_tangent_scale"]
    first_ring = []
    for previous_id, interface_id in zip(previous, interface):
        before = mesh["vertices"][previous_id]
        seam = mesh["vertices"][interface_id]
        first_ring.append(len(vertices))
        vertices.append(_rounded([
            seam[axis] + tangent_scale * (seam[axis] - before[axis])
            for axis in range(3)
        ]))
        regions.append({name: 1.0 if name == "neck" else 0.0 for name in REGIONS})
        vertex_modules.append("neck")

    center = [sum(mesh["vertices"][index][axis] for index in interface) / len(interface) for axis in range(3)]
    end_z = center[2] - socket["neck_length_m"]
    neck_rings = [first_ring]
    for ring_index in range(1, socket["neck_rings"]):
        t = ring_index / (socket["neck_rings"] - 1)
        blend = t * t * (3.0 - 2.0 * t)
        ring = []
        for segment, interface_id in enumerate(interface):
            seam = mesh["vertices"][interface_id]
            target = [
                center[0] + (seam[0] - center[0]) * socket["end_scale_x"],
                center[1] + (seam[1] - center[1]) * socket["end_scale_y"],
                end_z,
            ]
            start = vertices[first_ring[segment]]
            ring.append(len(vertices))
            vertices.append(_rounded([start[axis] + blend * (target[axis] - start[axis]) for axis in range(3)]))
            regions.append({name: 1.0 if name == "neck" else 0.0 for name in REGIONS})
            vertex_modules.append("neck")
        neck_rings.append(ring)

    ring_pairs = [(interface, neck_rings[0])] + list(zip(neck_rings, neck_rings[1:]))
    for upper, lower in ring_pairs:
        for segment in range(len(interface)):
            next_segment = (segment + 1) % len(interface)
            faces.append([upper[segment], upper[next_segment], lower[next_segment], lower[segment]])
            face_modules.append("neck")

    last_ring = neck_rings[-1]
    bottom = len(vertices)
    vertices.append(_rounded([
        sum(vertices[index][0] for index in last_ring) / len(last_ring),
        sum(vertices[index][1] for index in last_ring) / len(last_ring),
        sum(vertices[index][2] for index in last_ring) / len(last_ring),
    ]))
    regions.append({name: 1.0 if name == "neck" else 0.0 for name in REGIONS})
    vertex_modules.append("neck")
    for segment in range(len(interface)):
        next_segment = (segment + 1) % len(interface)
        faces.append([bottom, last_ring[next_segment], last_ring[segment]])
        face_modules.append("neck")

    return {
        "schema": "composed-character-mesh/0",
        "source_recipe_id": mesh["recipe_id"],
        "socket_id": socket["id"],
        "vertices": vertices,
        "faces": faces,
        "regions": regions,
        "vertex_modules": vertex_modules,
        "face_modules": face_modules,
        "socket": {
            "interface_vertices": interface,
            "previous_vertices": previous,
            "first_neck_ring": first_ring,
            "last_neck_ring": last_ring,
            "bottom_vertex": bottom,
        },
    }
