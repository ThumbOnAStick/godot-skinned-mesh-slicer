#ifndef TRIANGLE_H
#define TRIANGLE_H

#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/mesh_data_tool.hpp>
#include <godot_cpp/classes/surface_tool.hpp>

#include <array>
#include <cstddef>
#include <unordered_set>
#include <vector>

namespace godot {

class Triangle {
    public:
    int index;
    std::array<int, 3> vertex_indices; // original mesh vertex indices of the three corners
    std::array<Vector3, 3> vertices;
    std::array<Vector2, 3> uvs;
    std::array<Vector3, 3> normals;
    std::array<PackedInt32Array, 3> bones;
    std::array<PackedFloat32Array, 3> weights;
    std::vector<int> neighbors;


    Vector3& operator[](size_t idx) { return vertices[idx]; }
    const Vector3& operator[](size_t idx) const { return vertices[idx]; }
    void add_neighbor(int t_idx);
    bool try_add_neighbor(Triangle& triangle);
    static Triangle from_face(Ref<MeshDataTool> mdt, int index);
    Triangle();

};

} //namespace godot

#endif