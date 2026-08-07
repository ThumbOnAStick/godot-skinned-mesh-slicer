#ifndef DFS_H
#define DFS_H

#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/mesh_data_tool.hpp>

#include <array>
#include <cstddef>
#include <unordered_set>
#include <vector>

namespace godot {

struct Triangle {
    std::array<Vector3, 3> vertices;

    Vector3& operator[](size_t idx) { return vertices[idx]; }
    const Vector3& operator[](size_t idx) const { return vertices[idx]; }
    String get_all();
};

class Adjacency {
public:
	std::vector<std::array<Triangle, 2>> adjacencies;

	bool try_add_adjacency(Triangle &triangle1, Triangle &triangle2);
    String get_all();

private:
	void add_adjacency(Triangle &triangle1, Triangle &triangle2);
};

} //namespace godot

#endif