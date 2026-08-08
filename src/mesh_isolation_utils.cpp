#include <mesh_isolation_utils.h>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/immediate_mesh.hpp>
#include <godot_cpp/classes/mesh_data_tool.hpp>
#include <godot_cpp/classes/placeholder_mesh.hpp>
#include <godot_cpp/classes/primitive_mesh.hpp>
#include <godot_cpp/classes/surface_tool.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/memory.hpp>
#include <godot_cpp/variant/Vector3.hpp>
#include <godot_cpp/variant/basis.hpp>
#include <godot_cpp/variant/plane.hpp>

#include <vector>
#include <array>

#include "mesh_isolation/fragment.h"
#include "mesh_isolation/triangle.h"
using namespace godot;

Triangle MeshIsolationUtils::from_face(Ref<MeshDataTool> mdt, int f_idx) {
	Triangle result = Triangle();
	result.index = f_idx;
	for (size_t i = 0; i < 3; i++) {
		Vector3 v = mdt->get_vertex(mdt->get_face_vertex(f_idx, i));
		result.vertices[i] = v;
	}
	return result;
}

/// @brief Find all triangles connected to the first triangle
/// @param triangles The list that contians all triangles
/// @param pointer_idx The index of the first pointer
/// @param fragment The fragment that collects the connected faces.
/// @param visited Visited list
/// @return fragment list
void MeshIsolationUtils::find_connected_faces_recursive(const std::vector<Triangle> &triangles, int pointer_idx, Fragment &fragment, std::vector<bool> &visited) {
	// Record the current triangle as seen.
	fragment.append_triangle(pointer_idx);
	visited[pointer_idx] = true;

	// DFS traversal through all connected neighbors.
	const Triangle &current = triangles[pointer_idx];
	for (int neighbor_idx : current.neighbors) {
		// Only recurse into neighbors that haven't been visited yet.
		if (!visited[neighbor_idx]) {
			find_connected_faces_recursive(triangles, neighbor_idx, fragment, visited);
		}
	}
}

/// @brief Builds the neighbor-linked triangle list from the mesh.
/// @param mdt Mesh data tool of the original mesh
/// @return Triangles indexed by face index, with neighbors assigned.
std::vector<Triangle> MeshIsolationUtils::build_triangles(Ref<MeshDataTool> mdt) {
	int face_count = mdt->get_face_count();
	std::vector<Triangle> triangles(face_count);

	// Build every triangle once up front.
	for (size_t f_idx = 0; f_idx < face_count; f_idx++) {
		triangles[f_idx] = from_face(mdt, f_idx);
	}

	// Assign neighbors
	for (size_t f_idx = 0; f_idx < face_count; f_idx++) {
		Triangle &t_1 = triangles[f_idx];
		for (size_t o_f_idx = f_idx + 1; o_f_idx < face_count; o_f_idx++) {
			Triangle &t_2 = triangles[o_f_idx];
			if (t_1.try_add_neighbor(t_2)) {
				t_2.add_neighbor(f_idx); // Directly add t_1 as a neighbor to t_2
			}
		}
	}

	return triangles;
}

/// @brief Collects every disconnected set of faces as a Fragment.
/// @param triangles The neighbor-linked triangle list, indexed by face index
/// @return A vector of fragments, one for each disconnected "island".
std::vector<Fragment> MeshIsolationUtils::find_disconnected_fragments(const std::vector<Triangle> &triangles) {
	std::vector<Fragment> fragments;
	int face_count = (int)triangles.size();
	if (face_count == 0) {
		return fragments;
	}

	// Find every island by starting DFS from each unvisited face.
	std::vector<bool> visited(face_count, false);
	for (int f_idx = 0; f_idx < face_count; f_idx++) {
		if (!visited[f_idx]) {
			Fragment island;
			find_connected_faces_recursive(triangles, f_idx, island, visited);
			fragments.push_back(island);
		}
	}

	return fragments;
}

/// @brief A helper that identifies all disconnected faces.
/// @param mdt Mesh data tool of the original mesh
/// @return A string that contians the result
String MeshIsolationUtils::find_disconnected_faces_mdt(Ref<MeshDataTool> mdt) {
	// Each disconnected set of geometry is referred to as an "island"
	String result;
	int face_count = mdt->get_face_count();
	if (face_count == 0) {
		return "No faces to process";
	}

	std::vector<Triangle> triangles = build_triangles(mdt);
	std::vector<Fragment> fragments = find_disconnected_fragments(triangles);

	int largest_island = 0;
	for (const Fragment &fragment : fragments) {
		if ((int)fragment.triangles.size() > largest_island) {
			largest_island = (int)fragment.triangles.size();
		}
	}

	int connected_count = 0;
	(void)connected_count; // Reserved for future use
	result = "Islands: " + String::num((int)fragments.size()) + ", Largest: " + String::num(largest_island) + ", Total faces: " + String::num(face_count);
	return result;
}

/// @brief Builds one ArrayMesh per disconnected fragment of the given surface.
/// @param mesh The target mesh
/// @param s_idx Surface index
/// @return A typed array of ArrayMesh, one mesh per fragment.
TypedArray<ArrayMesh> MeshIsolationUtils::build_fragment_meshes(Ref<ArrayMesh> mesh, int s_idx) {
	TypedArray<ArrayMesh> result;

	Ref<MeshDataTool> mdt = memnew(MeshDataTool);
	mdt->create_from_surface(mesh, s_idx);

	std::vector<Triangle> triangles = build_triangles(mdt);
	std::vector<Fragment> fragments = find_disconnected_fragments(triangles);

	for (Fragment &fragment : fragments) {
		// Build a fresh surface tool for this fragment's mesh.
		Ref<SurfaceTool> st = memnew(SurfaceTool);
		st->begin(Mesh::PRIMITIVE_TRIANGLES);

		fragment.build_surface(triangles, st);

		// Guarantee valid face normals so the reconstructed surfaces are
		// properly lit and don't render black.
		st->generate_normals();

		Ref<ArrayMesh> fragment_mesh = memnew(ArrayMesh);
		st->commit(fragment_mesh);
		result.append(fragment_mesh);
	}

	return result;
}

/// @brief Prints out disconnected vertices of given surface
/// @param mesh The target mesh
/// @param s_idx Surface index
/// @return
String MeshIsolationUtils::find_disconnected_faces(Ref<ArrayMesh> mesh, int s_idx) {
	Ref<MeshDataTool> mdt = memnew(MeshDataTool);
	mdt->create_from_surface(mesh, s_idx);
	return find_disconnected_faces_mdt(mdt);
}

void MeshIsolationUtils::_bind_methods() {
	ClassDB::bind_static_method("MeshIsolationUtils", D_METHOD("find_disconnected_faces", "mesh", "s_idx"), &MeshIsolationUtils::find_disconnected_faces);
	ClassDB::bind_static_method("MeshIsolationUtils", D_METHOD("build_fragment_meshes", "mesh", "s_idx"), &MeshIsolationUtils::build_fragment_meshes);
}

MeshIsolationUtils::MeshIsolationUtils() {
}