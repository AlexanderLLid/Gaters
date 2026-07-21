"""Combine front silhouette and local depth on one closed fixed topology."""

import math

from depth_fit import sample_depth
from silhouette_fit import fit_fixed_topology


def relax_relief(values, edges, locked, iterations, factor, source_weight=0.0):
    neighbors = [set() for _ in values]
    for first, second in edges:
        neighbors[first].add(second)
        neighbors[second].add(first)
    source = list(values)
    result = list(values)
    for _ in range(iterations):
        previous = result
        result = list(previous)
        for index, adjacent in enumerate(neighbors):
            if index in locked or not adjacent:
                continue
            average = sum(previous[neighbor] for neighbor in adjacent) / len(adjacent)
            smoothed = previous[index] + factor * (average - previous[index])
            result[index] = smoothed + source_weight * (source[index] - smoothed)
    return result


def fit_closed_mask(vertices, front_spans, depth_image, depth_scale, neutral_depth,
                    edges=None, smooth_iterations=0, smooth_factor=0.0, source_weight=0.0):
    silhouette = fit_fixed_topology(vertices, front_spans)
    displacements = []
    locked = set()
    for original, shaped in zip(vertices, silhouette):
        x, y, z = shaped
        radial = math.hypot(original[0], original[1])
        front_weight = max(0.0, -original[1] / radial) ** 4 if radial else 0.0
        depth = sample_depth(depth_image, (x + 1.0) * 0.5, (z + 1.0) * 0.5)
        displacements.append((depth - neutral_depth) * depth_scale * front_weight)
        if front_weight == 0.0:
            locked.add(len(displacements) - 1)
    if edges and smooth_iterations:
        displacements = relax_relief(
            displacements, edges, locked, smooth_iterations, smooth_factor, source_weight
        )
    return [
        tuple(0.0 if abs(value) < 1e-12 else value for value in (x, y - displacement, z))
        for (x, y, z), displacement in zip(silhouette, displacements)
    ]
