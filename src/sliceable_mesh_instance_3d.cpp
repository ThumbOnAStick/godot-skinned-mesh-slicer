#include "sliceable_mesh_instance_3d.h"
#include "kd_tree.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/memory.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/Vector3.hpp>
#include <godot_cpp/variant/basis.hpp>
#include <godot_cpp/variant/plane.hpp>
#include <godot_cpp/classes/primitive_mesh.hpp>
#include <godot_cpp/classes/immediate_mesh.hpp>
#include <godot_cpp/classes/placeholder_mesh.hpp>

#include <vector>
#include "sliceable_mesh_utils.h"

using namespace godot;

void SliceableMeshInstance3D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_inner_material"), &SliceableMeshInstance3D::get_inner_material);
	ClassDB::bind_method(D_METHOD("set_inner_material", "p_inner_material"), &SliceableMeshInstance3D::set_inner_material);
	ClassDB::add_property("SliceableMeshInstance3D", PropertyInfo(Variant::OBJECT, "inner_material", PROPERTY_HINT_RESOURCE_TYPE, "BaseMaterial3D,ShaderMaterial"), "set_inner_material", "get_inner_material");

	ClassDB::bind_method(D_METHOD("slice_along_plane", "p_plane", "center", "out_mesh"), &SliceableMeshInstance3D::slice_along_plane);
}

SliceableMeshInstance3D::SliceableMeshInstance3D() : m_inner_material() { }

SliceableMeshInstance3D::~SliceableMeshInstance3D() { }

void SliceableMeshInstance3D::set_inner_material(const Ref<Material> p_inner_material) {
	m_inner_material = p_inner_material;
}

Ref<Material> SliceableMeshInstance3D::get_inner_material() const {
	return m_inner_material;
}

void SliceableMeshInstance3D::slice_along_plane(const Plane &p_plane, const Vector3 &center, Ref<ArrayMesh> out_mesh) {
	this->slice_along_plane_p(p_plane, center, out_mesh);
}


void SliceableMeshInstance3D::slice_along_plane_p(const Plane &p_plane, const Vector3 &center, Ref<ArrayMesh> out_mesh) {
	Ref<Mesh> mesh = this->get_mesh();
	Plane plane = p_plane;
	if (plane.is_point_over(center)){
		plane = -plane;
	}

	if (auto primitive_mesh = Object::cast_to<PrimitiveMesh>(mesh.ptr())) {
		// mesh is of type PrimitiveMesh -> convert to ArrayMesh
		Ref<ArrayMesh> array_mesh = memnew(ArrayMesh);
		array_mesh->add_surface_from_arrays(
			Mesh::PRIMITIVE_TRIANGLES,
			primitive_mesh->get_mesh_arrays()
		);
		for (size_t i = 0; i < array_mesh->get_surface_count(); i++) {
			array_mesh->surface_set_material(i, mesh->surface_get_material(i));
		}

		this->set_mesh(this->slice_mesh_along_plane(array_mesh, plane, out_mesh));
	}
	else if (auto array_mesh = Object::cast_to<ArrayMesh>(mesh.ptr())) {
		this->set_mesh(this->slice_mesh_along_plane(array_mesh, plane, out_mesh));
	}
	else if (Object::cast_to<ImmediateMesh>(mesh.ptr()) != nullptr) {
		WARN_PRINT("Cannot slice ImmediateMesh.");
		return;
	}
	else if (Object::cast_to<PlaceholderMesh>(mesh.ptr()) != nullptr) {
		WARN_PRINT("Cannot slice PlaceholderMesh.");
		return;
	}
	else {
		WARN_PRINT("Cannot slice unknown Mesh Type.");
		return;
	}
}

Ref<ArrayMesh> SliceableMeshInstance3D::slice_mesh_along_plane(
	const Ref<ArrayMesh> p_array_mesh, const Plane p_plane, Ref<ArrayMesh> out_mesh
) const {
	// transform the plane to object space
	Plane plane_os = this->get_global_transform().xform_inv(p_plane);

	Ref<ArrayMesh> new_mesh = memnew(ArrayMesh);

	int32_t surface_count = p_array_mesh->get_surface_count();

	if (surface_count == 0) {
		WARN_PRINT("Mesh has no surface.");
		return new_mesh;
	}

	// this point is used for adding the "lid"
	// it will be set to the first created vertex (which is on the edge of the slice).
	Vector3 pos_on_lid;
	// keep track if pos_on_lid has been set. the first created vertex will set it.
	bool pos_on_lid_defined = false;

	// surface tool for the lid. all surfaces will add to it and only then will a new surface be created from it.
	Ref<SurfaceTool> st_lid = memnew(SurfaceTool);
	st_lid->begin(Mesh::PRIMITIVE_TRIANGLES);

	int32_t created_surface_count = 0;
	for (size_t i = 0; i < surface_count; i++) {
		Ref<MeshDataTool> mdt = memnew(MeshDataTool);
		mdt->create_from_surface(p_array_mesh, i);

		// surface tool for the sliced surface
		Ref<SurfaceTool> st_sliced = memnew(SurfaceTool);
		st_sliced->begin(Mesh::PRIMITIVE_TRIANGLES);

		// surface tool for the "drop" surface
		Ref<SurfaceTool> st_upper = memnew(SurfaceTool);
		st_upper->begin(Mesh::PRIMITIVE_TRIANGLES);

		// will add new mesh data to the surface tools
		slice_surface_along_plane(mdt, st_sliced, st_upper, st_lid, pos_on_lid, pos_on_lid_defined, plane_os);

		// shrinks the vertex array by creating an index array (triangle list)
		// has a high performance penalty for big meshes
		// commit sliced surface as a new surface
		st_sliced->commit(new_mesh);
		st_upper->commit(out_mesh);



		// if a surface was added, set material
		if (new_mesh->get_surface_count() > created_surface_count) {
			new_mesh->surface_set_material(created_surface_count, mdt->get_material());
			++created_surface_count;
		}
	}

	// shrinks the vertex array by creating an index array (triangle list)
	// has a high performance penalty for big meshes
	// if (indexed) st_lid->index();
	// commit lid as a new surface (skipped for now)
	// TODO: Use more advanded face filling method
	st_lid->commit(new_mesh);
	// if a surface was added, set material
	if (new_mesh->get_surface_count() > created_surface_count) {
		new_mesh->surface_set_material(created_surface_count, m_inner_material);
	}

	return new_mesh;
}

void SliceableMeshInstance3D::slice_surface_along_plane(
	const Ref<MeshDataTool> p_mdt, const Ref<SurfaceTool> p_st_sliced, const Ref<SurfaceTool> p_st_outmesh, const Ref<SurfaceTool> p_st_lid,
	Vector3 &p_pos_on_lid, bool &p_pos_on_lid_defined, const Plane p_plane_os
) const {

	// --- Build KD-tree from all below-plane vertices for nearest-neighbor bone lookup ---
	std::vector<KDTree::Point> below_vertices;
	int32_t vertex_count = p_mdt->get_vertex_count();
	for (int32_t vi = 0; vi < vertex_count; ++vi) {
		Vector3 v = p_mdt->get_vertex(vi);
		if (!p_plane_os.is_point_over(v)) {
			// Vertex is below (or on) the plane — include in KD-tree
			below_vertices.emplace_back(v,
				p_mdt->get_vertex_bones(vi),
				p_mdt->get_vertex_weights(vi));
		}
	}
	KDTree kd_tree;
	if (!below_vertices.empty()) {
		kd_tree.build(below_vertices);
	}
	// ----------------------------------------------------------------------------------

	// Bundle all shared slice state into one cache so helper functions take fewer parameters.
	SliceUtils::SliceCache cache;
	cache.mdt = p_mdt;
	cache.plane_os = p_plane_os;
	cache.lid_normal = p_plane_os.normal;
	cache.st_sliced = p_st_sliced;
	cache.st_outmesh = p_st_outmesh;
	cache.st_lid = p_st_lid;
	cache.kd_tree = &kd_tree;
	cache.pos_on_lid = p_pos_on_lid;
	cache.pos_on_lid_defined = p_pos_on_lid_defined;

	for (int32_t face_idx = 0; face_idx < (int32_t)p_mdt->get_face_count(); ++face_idx) {
		SliceUtils::process_triangle(cache, face_idx);
	}

	// Write the lid accumulator back out to the caller's references.
	p_pos_on_lid = cache.pos_on_lid;
	p_pos_on_lid_defined = cache.pos_on_lid_defined;
}
