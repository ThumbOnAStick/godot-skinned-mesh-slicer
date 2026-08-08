
#include <fragment.h>

#include <algorithm>
#include <functional>
#include <unordered_map>


namespace godot
{
    /// @brief The per-vertex attributes needed to emit a lid vertex, indexed
    ///        by the original mesh vertex index.
    struct FragmentVertexData {
        Vector3 position;
        Vector3 normal;
        Vector2 uv;
        PackedInt32Array bones;
        PackedFloat32Array weights;
    };

    /// @brief Builds a single intersection record from an edge (v_1_idx, v_2_idx).
    /// @param v_1_idx Index of the intersection vertex itself.
    /// @param v_2_idx Index of a neighboring intersection vertex.
    /// @return A FragmentIntersection whose index is v_1_idx and whose only
    ///         neighbor link points at v_2_idx.
    FragmentIntersection FragmentIntersection::from_vertices(int& v_1_idx, int& v_2_idx)
    {
        FragmentIntersection i;
        i.index = v_1_idx;
        i.neighbors.push_back(v_2_idx);
        return i; 
    }

    /// @brief Appends a triangle to this fragment by extracting its index from the Triangle.
    /// @param triangle The triangle whose index should be added.
    void Fragment::append_triangle(Triangle& triangle)
    {
        this->triangles.push_back(triangle.index);
    }
    
    /// @brief Appends a triangle to this fragment by its index.
    /// @param t_idx The mesh data tool face index to add.
    void Fragment::append_triangle(int& t_idx)
    {
        this->triangles.push_back(t_idx);
    }
    
    /// @brief Records an intersection edge between two vertices.
    /// @param v_1_idx The intersection vertex index.
    /// @param v_2_idx The index of the vertex it connects to.
    void Fragment::append_intersection(int& v_1_idx, int& v_2_idx)
    {
        FragmentIntersection f_i = FragmentIntersection::from_vertices(v_1_idx, v_2_idx);
        this->intersection_verts.push_back(f_i);
    }
    
    /// @brief Groups the stored intersection vertices into connected components.
    ///
    /// Nearby intersection vertices linked through their neighbors list form a
    /// single group (e.g. one closed lid loop). Every entry of verts_groups is
    /// filled with the group id its corresponding intersection belongs to.
    void Fragment::bake_intersection_groups()
    {
        // Assign a group id to every connected component of intersection vertex.

        verts_groups.clear();
        verts_groups.resize(intersection_verts.size(), 0);

        std::vector<bool> visited(intersection_verts.size(), false);

        // DFS flood fill that marks every intersection reachable from idx with
        // the same group id.
        std::function<void(int, int)> flood_fill = [&](int idx, int group) {
            if (visited[idx]) {
                return;
            }
            visited[idx] = true;
            verts_groups[idx] = group;
            for (int neighbor_idx : intersection_verts[idx].neighbors) {
                flood_fill(neighbor_idx, group);
            }
        };

        int group_count = 0;
        for (size_t i = 0; i < intersection_verts.size(); i++) {
            if (!visited[i]) {
                flood_fill(i, group_count);
                group_count++;
            }
        }
    }
    
    /// @brief Removes a part of triangles and intersection vertices, returns the removed part. Re-bake intersection vertices afterwards.
    /// @param mdt The original mesh data tool, used to access the vertices of the removed triangles.
    /// @param t_rm Triangle indices to remove
    Fragment Fragment::cut_out(Ref<MeshDataTool> mdt, std::vector<int>& t_rm)
    {
        // The removed part takes ownership of the flagged triangles.
        Fragment removed;
        removed.triangles = t_rm;

        // Strip the flagged triangles from this fragment.
        for (int t_idx : t_rm) {
            auto it = std::find(triangles.begin(), triangles.end(), t_idx);
            if (it != triangles.end()) {
                triangles.erase(it);
            }
        }

        // Gather the mesh vertex indices used by the removed triangles so we
        // can decide which intersection vertices belong to the cut-out part.
        std::unordered_set<int> removed_vertex_indices;
        for (int t_idx : t_rm) {
            for (int i = 0; i < 3; i++) {
                removed_vertex_indices.insert(mdt->get_face_vertex(t_idx, i));
            }
        }

        // Intersection vertices that reference a removed triangle's vertex are
        // moved to the cut-out fragment and dropped from this one.
        std::vector<FragmentIntersection> kept;
        kept.reserve(intersection_verts.size());
        for (const FragmentIntersection &fi : intersection_verts) {
            if (removed_vertex_indices.count(fi.index) != 0) {
                removed.intersection_verts.push_back(fi);
            }
            else {
                kept.push_back(fi);
            }
        }
        intersection_verts = std::move(kept);

        // Re-bake the intersection groups now that the triangle/vertex sets
        // have changed.
        removed.bake_intersection_groups();
        bake_intersection_groups();

        return removed;
    }
    
    /// @brief Writes all of this fragment's triangles into a surface tool.
    /// @param triangles The neighbor-linked triangle list, indexed by face index.
    /// @param st The surface tool receiving the triangles.
    ///
    /// For each stored triangle the three face vertices are copied along with
    /// their attributes (bones, weights, normal, uv, position).
    void Fragment::build_surface(std::vector<Triangle>& triangles, Ref<SurfaceTool> st)
    {
        for (int t_idx : this->triangles) {
            Triangle &tri = triangles[t_idx];
            for (int i = 0; i < 3; i++) {
                st->set_bones(tri.bones[i]);
                st->set_weights(tri.weights[i]);
                st->set_normal(tri.normals[i]);
                st->set_uv(tri.uvs[i]);
                st->add_vertex(tri.vertices[i]);
            }
        }
    }

    /// @brief Builds a lid mesh for every intersection group.
    /// @param triangles The neighbor-linked triangle list, indexed by face index.
    /// @param lid_st The surface tool receiving the lid triangles.
    ///
    /// Each connected group of intersection vertices is ordered into a closed
    /// loop and fan-filled into triangles around its first vertex.
    void Fragment::build_lids(std::vector<Triangle>& triangles, Ref<SurfaceTool> lid_st)
    {
        if (intersection_verts.empty()) {
            return;
        }

        // Collects the attributes for every mesh vertex referenced by the
        // intersection vertices, keyed by the original mesh vertex index.
        std::unordered_map<int, FragmentVertexData> vertex_data;
        vertex_data.reserve(intersection_verts.size());
        for (const Triangle &tri : triangles) {
            for (int i = 0; i < 3; i++) {
                vertex_data[tri.vertex_indices[i]] = FragmentVertexData{
                    tri.vertices[i], tri.normals[i], tri.uvs[i], tri.bones[i], tri.weights[i]
                };
            }
        }

        // Count the number of intersection groups.
        int group_count = 0;
        for (uint8_t g : verts_groups) {
            group_count = std::max(group_count, (int)g + 1);
        }

        for (int group = 0; group < group_count; group++) {
            // Collect every intersection that belongs to this group.
            std::vector<int> members;
            for (size_t i = 0; i < intersection_verts.size(); i++) {
                if (verts_groups[i] == group) {
                    members.push_back((int)i);
                }
            }
            if ((int)members.size() < 3) {
                continue;
            }

            // Walk the neighbor links to order the members into a closed loop.
            std::vector<int> loop;
            std::unordered_set<int> visited;
            loop.push_back(members[0]);
            visited.insert(members[0]);
            int cur = members[0];
            int prev = -1;

            while ((int)loop.size() < (int)members.size()) {
                const FragmentIntersection &fi = intersection_verts[cur];
                int next = -1;
                for (int nb : fi.neighbors) {
                    if (nb == prev) continue;                 // don't go straight back
                    if (verts_groups[nb] != group) continue;  // stay within the group
                    if (visited.count(nb)) continue;          // already on the loop
                    next = nb;
                    break;
                }
                if (next == -1) {
                    break; // couldn't advance (shouldn't happen for closed loops)
                }
                loop.push_back(next);
                visited.insert(next);
                prev = cur;
                cur = next;
            }

            if ((int)loop.size() < 3) {
                continue;
            }

            // Fan fill: triangulate the loop around its first vertex.
            const FragmentVertexData &base = vertex_data[intersection_verts[loop[0]].index];
            for (size_t k = 1; k + 1 < loop.size(); k++) {
                const FragmentVertexData &a = vertex_data[intersection_verts[loop[k]].index];
                const FragmentVertexData &b = vertex_data[intersection_verts[loop[k + 1]].index];

                lid_st->set_bones(base.bones);
                lid_st->set_weights(base.weights);
                lid_st->set_normal(base.normal);
                lid_st->set_uv(base.uv);
                lid_st->add_vertex(base.position);

                lid_st->set_bones(a.bones);
                lid_st->set_weights(a.weights);
                lid_st->set_normal(a.normal);
                lid_st->set_uv(a.uv);
                lid_st->add_vertex(a.position);

                lid_st->set_bones(b.bones);
                lid_st->set_weights(b.weights);
                lid_st->set_normal(b.normal);
                lid_st->set_uv(b.uv);
                lid_st->add_vertex(b.position);
            }
        }
    }


    
    /// @brief Constructs an empty fragment with no triangles or intersections.
    Fragment::Fragment()
    {
        
    }
    
}