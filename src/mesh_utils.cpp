#include <mesh_utils.h>
#include <triangle.h>
#include <godot_cpp/classes/immediate_mesh.hpp>
#include <godot_cpp/classes/mesh_data_tool.hpp>
#include <godot_cpp/classes/placeholder_mesh.hpp>
#include <godot_cpp/classes/primitive_mesh.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/memory.hpp>
#include <godot_cpp/variant/Vector3.hpp>
#include <godot_cpp/variant/basis.hpp>
#include <godot_cpp/variant/plane.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <vector>
#include <array>

using namespace godot;

Triangle MeshUtils::from_face(Ref<MeshDataTool> mdt, int f_idx) {
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
/// @param found The list of connected faces.
/// @param visited Visited list
/// @return found list
void MeshUtils::find_connected_faces_recursive(std::vector<Triangle> &triangles, int pointer_idx, std::vector<int> &found, std::vector<bool> &visited) {
	// Record the current triangle as seen.
	found.push_back(pointer_idx);
	visited[pointer_idx] = true;

	// DFS traversal through all connected neighbors.
	Triangle &current = triangles[pointer_idx];
	for (int neighbor_idx : current.neighbors) {
		// Only recurse into neighbors that haven't been visited yet.
		if (!visited[neighbor_idx]) {
			find_connected_faces_recursive(triangles, neighbor_idx, found, visited);
		}
	}
}

/// @brief A helper that identifies all disconnected faces.
/// @param mdt Mesh data tool of the original mesh
/// @return A string that contians the result
String MeshUtils::find_disconnected_faces_mdt(Ref<MeshDataTool> mdt) {
	// Each disconnected set of geometry is referred to as an "island"
	String result;
	int face_count = mdt->get_face_count();
	if (face_count == 0) {
		return "No faces to process";
	}
	int island_count = 0;
	// Fixed-size storage indexed by face index; pre-sized to the triangle count
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

	// Find every island by starting DFS from each unvisited face.
	int largest_island = 0;
	std::vector<bool> visited(face_count, false);
	for (int f_idx = 0; f_idx < face_count; f_idx++) {
		if (!visited[f_idx]) {
			std::vector<int> island;
			find_connected_faces_recursive(triangles, f_idx, island, visited);
			island_count++;
			if ((int)island.size() > largest_island) {
				largest_island = (int)island.size();
			}
		}
	}

	int connected_count = 0;
	(void)connected_count; // Reserved for future use
	result = "Islands: " + String::num(island_count) + ", Largest: " + String::num(largest_island) + ", Total faces: " + String::num(face_count);
	return result;
}

/// @brief Prints out disconnected vertices of given surface
/// @param mesh The target mesh
/// @param s_idx Surface index
/// @return
String MeshUtils::find_disconnected_faces(Ref<ArrayMesh> mesh, int s_idx) {
	Ref<MeshDataTool> mdt = memnew(MeshDataTool);
	mdt->create_from_surface(mesh, s_idx);
	return find_disconnected_faces_mdt(mdt);
}

void MeshUtils::_bind_methods() {
	ClassDB::bind_static_method("MeshUtils", D_METHOD("find_disconnected_faces", "mesh", "s_idx"), &MeshUtils::find_disconnected_faces);
}

MeshUtils::MeshUtils() {
}