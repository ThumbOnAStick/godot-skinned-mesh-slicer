#ifndef FRAGMENT_H
#define FRAGMENT_H
#include "triangle.h"
#include <array>
#include <cstddef>
#include <unordered_set>
#include <vector>
#include <godot_cpp/classes/mesh_data_tool.hpp>
#include <godot_cpp/classes/surface_tool.hpp>

namespace godot {

struct FragmentIntersection{
    int index;
    std::vector<int> neighbors; // Stores indexes of its neighbors

    static FragmentIntersection from_vertices(int& v_1_idx, int& v_2_idx);

    bool operator==(const FragmentIntersection &other) const {
        return index == other.index;
    }
};

class Fragment {
    public:
    std::vector<int> triangles; // Stores index of each triangle, they should be consistant with the mesh data tool face indices
    std::vector<FragmentIntersection> intersection_verts; // The list of vertex in the intersection
    std::vector<uint8_t> verts_groups; // The group that each intersection belongs

    void append_triangle(Triangle& triangle);
    void append_triangle(int& t_idx);
    void append_intersection(int& v_1_idx, int& v_2_idx);
    void bake_intersection_groups();
    Fragment cut_out(Ref<MeshDataTool> mdt, std::vector<int>& t_rm);
    void build_surface(std::vector<Triangle>& triangles, Ref<SurfaceTool> main_st);
    void build_lids(std::vector<Triangle>& triangles, Ref<SurfaceTool> lid_st);
    
    Fragment();
};

} //namespace godot

#endif