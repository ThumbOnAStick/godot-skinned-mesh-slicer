#ifndef SLICE_CACHE_H
#define SLICE_CACHE_H

#include "misc/kd_tree.h"
#include "slice_vertex.h"

#include <godot_cpp/classes/mesh_data_tool.hpp>
#include <godot_cpp/classes/surface_tool.hpp>
#include <godot_cpp/variant/plane.hpp>
#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/vector3.hpp>

#include <cstdint>

using namespace godot;

namespace godot::SliceUtils {

/// Bundles all the state shared across the faces of one slice operation.
///
/// This is a per-slice "cache": it is constructed once in the caller, populated
/// with the mesh data, cutting plane and output surface tools, then passed by
/// reference so the helper functions don't each need long parameter lists.
/// Because it is created and owned per slice (and per thread), it stays
/// reentrant — there is no global shared scratch state.
struct SliceCache {
	// Inputs (set once per slice, reused for every face).
	Ref<MeshDataTool> mdt;
	Plane plane_os;
	Vector3 lid_normal;
	Ref<SurfaceTool> st_sliced;       // kept (below-plane) triangles
	Ref<SurfaceTool> st_outmesh;      // removed (above-plane) triangles
	Ref<SurfaceTool> st_lid;          // lid (cap) triangles
	const KDTree *kd_tree = nullptr;  // nearest-neighbor tree of below-plane verts

	// Mutable lid edge accumulator (updated as faces are processed).
	Vector3 pos_on_lid;
	bool pos_on_lid_defined = false;

	// Per-face scratch (overwritten by each call to process_triangle). Stored here
	// so the draw helpers below can read them and avoid long parameter lists.
	int32_t verts_indices[3];
	Vector3 verts[3];
	bool verts_are_above[3];
	Vector3 verts_normals[3];
	Vector2 verts_uvs[3];
};

} // namespace godot::SliceUtils

#endif // SLICE_CACHE_H
