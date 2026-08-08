#ifndef SLICE_EMITTER_H
#define SLICE_EMITTER_H

#include "slice_cache.h"
#include "slice_vertex.h"

#include <godot_cpp/classes/surface_tool.hpp>

#include <cstdint>

using namespace godot;

namespace godot::SliceUtils {

/// Packs a mesh vertex's position/normal/uv/bones/weights into a SliceVertex.
/// Reads the attributes of vertex index p_vertex_idx from p_cache.mdt.
SliceVertex vertex_from_mesh(SliceCache &p_cache, int32_t p_vertex_idx);

/// Emits one vertex onto a SurfaceTool.
void add_vertex(const Ref<SurfaceTool> &p_st, const SliceVertex &p_v);

/// Emits a triangle (three vertices) onto a SurfaceTool.
void draw_triangle(const Ref<SurfaceTool> &p_st,
		const SliceVertex &p_v0, const SliceVertex &p_v1, const SliceVertex &p_v2);

/// Emits one of the cached face's mesh vertices (index 0..2) onto a surface,
/// using the per-face scratch data already populated in p_cache.
void add_mesh_vertex(SliceCache &p_cache, const Ref<SurfaceTool> &p_st, int32_t p_i);

/// Emits a triangle of the cached face's mesh vertices (indices 0..2) using
/// the per-face scratch data already populated in p_cache.
void draw_mesh_triangle(SliceCache &p_cache, const Ref<SurfaceTool> &p_st,
		int32_t p_i0, int32_t p_i1, int32_t p_i2);

} // namespace godot::SliceUtils

#endif // SLICE_EMITTER_H
