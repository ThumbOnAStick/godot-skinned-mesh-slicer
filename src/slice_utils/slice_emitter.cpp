#include "slice_emitter.h"

using namespace godot;

namespace godot::SliceUtils {

SliceVertex vertex_from_mesh(SliceCache &p_cache, int32_t p_vertex_idx) {
	const Ref<MeshDataTool> &mdt = p_cache.mdt;
	return SliceVertex(
			mdt->get_vertex(p_vertex_idx),
			mdt->get_vertex_normal(p_vertex_idx),
			mdt->get_vertex_uv(p_vertex_idx),
			mdt->get_vertex_bones(p_vertex_idx),
			mdt->get_vertex_weights(p_vertex_idx));
}

void add_vertex(const Ref<SurfaceTool> &p_st, const SliceVertex &p_v) {
	p_st->set_bones(p_v.bones);
	p_st->set_weights(p_v.weights);
	p_st->set_normal(p_v.normal);
	p_st->set_uv(p_v.uv);
	p_st->add_vertex(p_v.position);
}

void draw_triangle(const Ref<SurfaceTool> &p_st,
		const SliceVertex &p_v0, const SliceVertex &p_v1, const SliceVertex &p_v2) {
	add_vertex(p_st, p_v0);
	add_vertex(p_st, p_v1);
	add_vertex(p_st, p_v2);
}

void add_mesh_vertex(SliceCache &p_cache, const Ref<SurfaceTool> &p_st, int32_t p_i) {
	p_st->set_bones(p_cache.mdt->get_vertex_bones(p_cache.verts_indices[p_i]));
	p_st->set_weights(p_cache.mdt->get_vertex_weights(p_cache.verts_indices[p_i]));
	p_st->set_normal(p_cache.verts_normals[p_i]);
	p_st->set_uv(p_cache.verts_uvs[p_i]);
	p_st->add_vertex(p_cache.verts[p_i]);
}

void draw_mesh_triangle(SliceCache &p_cache, const Ref<SurfaceTool> &p_st,
		int32_t p_i0, int32_t p_i1, int32_t p_i2) {
	add_mesh_vertex(p_cache, p_st, p_i0);
	add_mesh_vertex(p_cache, p_st, p_i1);
	add_mesh_vertex(p_cache, p_st, p_i2);
}

} // namespace godot::SliceUtils
