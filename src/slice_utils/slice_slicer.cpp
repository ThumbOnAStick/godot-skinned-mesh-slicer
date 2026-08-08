#include "slice_slicer.h"

#include "slice_emitter.h"
#include "slice_geometry.h"

#include <godot_cpp/variant/packed_float32_array.hpp>
#include <godot_cpp/variant/packed_int32_array.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace godot::SliceUtils {

void add_lid(SliceCache &p_cache, const Vector3 &p_v0, const Vector3 &p_v1, const Vector3 &p_v2) {
	const KDTree &kd_tree = *p_cache.kd_tree;
	const Vector3 lid_normal = p_cache.lid_normal;

	const KDTree::Point &bp0 = kd_tree.nearest(p_v0);
	const KDTree::Point &bp1 = kd_tree.nearest(p_v1);
	const KDTree::Point &bp2 = kd_tree.nearest(p_v2);

	draw_triangle(p_cache.st_lid,
			SliceVertex(p_v0, lid_normal, Vector2(0, 0), bp0.bones, bp0.weights),
			SliceVertex(p_v1, lid_normal, Vector2(0, 0), bp1.bones, bp1.weights),
			SliceVertex(p_v2, lid_normal, Vector2(0, 0), bp2.bones, bp2.weights));
}

void process_triangle(SliceCache &p_cache, int32_t p_face_idx) {
	const Ref<MeshDataTool> &mdt = p_cache.mdt;
	const Plane &plane_os = p_cache.plane_os;

	// ---- Populate the cached per-face scratch data ----
	int32_t n_of_verts_above = 0;
	for (int32_t i = 0; i < 3; ++i) {
		p_cache.verts_indices[i] = mdt->get_face_vertex(p_face_idx, i);
		p_cache.verts[i] = mdt->get_vertex(p_cache.verts_indices[i]);
		p_cache.verts_are_above[i] = plane_os.is_point_over(p_cache.verts[i]);
		p_cache.verts_normals[i] = mdt->get_vertex_normal(p_cache.verts_indices[i]);
		p_cache.verts_uvs[i] = mdt->get_vertex_uv(p_cache.verts_indices[i]);

		if (p_cache.verts_are_above[i]) ++n_of_verts_above;
	}

	// Convenient aliases into the cached scratch.
	int32_t *verts_indices = p_cache.verts_indices;
	Vector3 *verts = p_cache.verts;
	bool *verts_are_above = p_cache.verts_are_above;
	Vector3 *verts_normals = p_cache.verts_normals;
	Vector2 *verts_uvs = p_cache.verts_uvs;

	switch (n_of_verts_above) {
		case 3: { // all vertices are above -> face is completely removed
			draw_mesh_triangle(p_cache, p_cache.st_outmesh, 0, 1, 2);
			break;
		}
		case 0: { // all vertices are below -> face is kept
			draw_mesh_triangle(p_cache, p_cache.st_sliced, 0, 1, 2);
			break;
		}
		case 2: { // two vertices are above and one below -> remove face, create one new face, create lid

			int32_t a0, a1, b; // above (remove), below (keep)
			// ensure the winding order stays the same!
			if (!verts_are_above[0]) { b = 0; a0 = 1; a1 = 2; }
			else if (!verts_are_above[1]) { b = 1; a0 = 2; a1 = 0; }
			else if (!verts_are_above[2]) { b = 2; a0 = 0; a1 = 1; }

			// copy bone weights and indices from original vertex b
			// (n0 and n1 use the same bone data as b since they're on the same side)
			PackedInt32Array b_bones = mdt->get_vertex_bones(verts_indices[b]);
			PackedFloat32Array b_weights = mdt->get_vertex_weights(verts_indices[b]);

			// Find the crossing points on the plane between b (below) and a0/a1 (above).
			ClipPoint np0 = interpolate_crossing(plane_os,
					verts[b], verts[a0], verts_normals[b], verts_normals[a0], verts_uvs[b], verts_uvs[a0]);
			ClipPoint np1 = interpolate_crossing(plane_os,
					verts[b], verts[a1], verts_normals[b], verts_normals[a1], verts_uvs[b], verts_uvs[a1]);

			SliceVertex vb(verts[b], verts_normals[b], verts_uvs[b], b_bones, b_weights);
			SliceVertex vn0 = clip_vertex(np0, b_bones, b_weights);
			SliceVertex vn1 = clip_vertex(np1, b_bones, b_weights);

			// previous order was b -> a0 -> a1, so new order is b -> n0 -> n1
			draw_triangle(p_cache.st_sliced, vb, vn0, vn1);

			PackedInt32Array a0_bones = mdt->get_vertex_bones(verts_indices[a0]);
			PackedFloat32Array a0_weights = mdt->get_vertex_weights(verts_indices[a0]);
			PackedInt32Array a1_bones = mdt->get_vertex_bones(verts_indices[a1]);
			PackedFloat32Array a1_weights = mdt->get_vertex_weights(verts_indices[a1]);

			SliceVertex va0(verts[a0], verts_normals[a0], verts_uvs[a0], a0_bones, a0_weights);
			SliceVertex va1(verts[a1], verts_normals[a1], verts_uvs[a1], a1_bones, a1_weights);
			SliceVertex vn0_a0 = clip_vertex(np0, a0_bones, a0_weights);
			SliceVertex vn1_a1 = clip_vertex(np1, a1_bones, a1_weights);

			// first triangle: a0 -> n1 -> n0
			draw_triangle(p_cache.st_outmesh, va0, vn1_a1, vn0_a0);
			// second triangle: a0 -> a1 -> n1
			draw_triangle(p_cache.st_outmesh, va0, va1, vn1_a1);

			if (p_cache.pos_on_lid_defined) {
				add_lid(p_cache, np1.position, p_cache.pos_on_lid, np0.position);
			}
			else { p_cache.pos_on_lid = np0.position; p_cache.pos_on_lid_defined = true; } // no need to add a lid

			break;
		}
		case 1: { // one vertex is above and two below -> remove face, create two new faces, create lid

			int32_t a, b0, b1; // above (remove), below (keep)
			// ensure the winding order stays the same!
			if (verts_are_above[0]) { a = 0; b0 = 1; b1 = 2; }
			else if (verts_are_above[1]) { a = 1; b0 = 2; b1 = 0; }
			else if (verts_are_above[2]) { a = 2; b0 = 0; b1 = 1; }

			// copy bone weights and indices from original vertices
			PackedInt32Array b0_bones = mdt->get_vertex_bones(verts_indices[b0]);
			PackedFloat32Array b0_weights = mdt->get_vertex_weights(verts_indices[b0]);
			PackedInt32Array b1_bones = mdt->get_vertex_bones(verts_indices[b1]);
			PackedFloat32Array b1_weights = mdt->get_vertex_weights(verts_indices[b1]);
			PackedInt32Array a_bones = mdt->get_vertex_bones(verts_indices[a]);
			PackedFloat32Array a_weights = mdt->get_vertex_weights(verts_indices[a]);

			// Find the crossing points on the plane between a (above) and b0/b1 (below).
			ClipPoint np0 = interpolate_crossing(plane_os,
					verts[b0], verts[a], verts_normals[b0], verts_normals[a], verts_uvs[b0], verts_uvs[a]);
			ClipPoint np1 = interpolate_crossing(plane_os,
					verts[b1], verts[a], verts_normals[b1], verts_normals[a], verts_uvs[b1], verts_uvs[a]);

			SliceVertex vb0(verts[b0], verts_normals[b0], verts_uvs[b0], b0_bones, b0_weights);
			SliceVertex vb1(verts[b1], verts_normals[b1], verts_uvs[b1], b1_bones, b1_weights);
			SliceVertex vn0_b0 = clip_vertex(np0, b0_bones, b0_weights);
			SliceVertex vn1_b1 = clip_vertex(np1, b1_bones, b1_weights);
			SliceVertex va(verts[a], verts_normals[a], verts_uvs[a], a_bones, a_weights);
			SliceVertex vn0_a = clip_vertex(np0, a_bones, a_weights);
			SliceVertex vn1_a = clip_vertex(np1, a_bones, a_weights);

			// previous order was b1 -> a -> b0, so the first triangle is b1 -> n1 -> n0
			draw_triangle(p_cache.st_sliced, vb1, vn1_b1, vn0_b0);
			// the second triangle is b1 -> n0 -> b0
			draw_triangle(p_cache.st_sliced, vb1, vn0_b0, vb0);

			// do the same for upper st
			draw_triangle(p_cache.st_outmesh, va, vn0_a, vn1_a);

			if (p_cache.pos_on_lid_defined) {
				add_lid(p_cache, np1.position, p_cache.pos_on_lid, np0.position);
			}
			else { p_cache.pos_on_lid = np0.position; p_cache.pos_on_lid_defined = true; } // no need to add a lid

			break;
		}
	}
}

} // namespace godot::SliceUtils
