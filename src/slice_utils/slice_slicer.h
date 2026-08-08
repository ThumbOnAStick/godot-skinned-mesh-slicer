#ifndef SLICE_SLICER_H
#define SLICE_SLICER_H

#include "slice_cache.h"

#include <godot_cpp/variant/vector3.hpp>

#include <cstdint>

using namespace godot;

namespace godot::SliceUtils {

/// Adds a "lid" triangle (the flat cap of the cut) to the lid surface.
/// Bone weights/indices are sampled from the nearest below-plane vertex
/// via the KD-tree stored in p_cache.
void add_lid(SliceCache &p_cache, const Vector3 &p_v0, const Vector3 &p_v1, const Vector3 &p_v2);

/// Processes a single face of a mesh being sliced by the plane in p_cache.
///
/// The face is classified by how many of its 3 vertices lie above the plane:
///   - 0 below: face is kept in the "sliced" surface.
///   - 3 above: face is removed and emitted into the "outmesh" surface.
///   - 1 or 2 above: the face is split, producing kept triangles, removed triangles,
///     plus an edge contribution added to the lid surface.
///
/// Scratch data is cached in p_cache; the only fresh local is the classification count.
///
/// \param p_cache      Per-slice cache (mesh data, plane, output surfaces, tree, lid accumulator).
/// \param p_face_idx   Index of the face to process.
void process_triangle(SliceCache &p_cache, int32_t p_face_idx);

} // namespace godot::SliceUtils

#endif // SLICE_SLICER_H
