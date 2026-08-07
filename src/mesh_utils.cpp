#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/memory.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/Vector3.hpp>
#include <godot_cpp/variant/basis.hpp>
#include <godot_cpp/variant/plane.hpp>
#include <godot_cpp/classes/primitive_mesh.hpp>
#include <godot_cpp/classes/immediate_mesh.hpp>
#include <godot_cpp/classes/placeholder_mesh.hpp>
#include <godot_cpp/classes/mesh_data_tool.hpp>
#include <mesh_utils.h>
#include <dfs.h>

#include <vector>

using namespace godot;


Triangle MeshUtils::from_face(Ref<MeshDataTool> mdt, int f_idx){
    Triangle result = Triangle();
    for (size_t i = 0; i < 3; i++)
    {
        Vector3 v = mdt->get_vertex(mdt->get_face_vertex(f_idx, i));
        result.vertices[i] = v;
    }
    return result;

}

String MeshUtils::find_disconnected_meshes_mdt(Ref<MeshDataTool> mdt){
        // Each disconnected set of geometry is referred to as an "island"
        String result;
        int face_count = mdt->get_face_count();
        Adjacency adj = Adjacency();
        // Append adjacencies
        for (size_t f_idx = 0; f_idx < face_count; f_idx++){
                Triangle t_1 = from_face(mdt, f_idx);
            for (size_t o_f_idx = f_idx + 1; o_f_idx < face_count; o_f_idx++){
                Triangle t_2 = from_face(mdt, o_f_idx);
                adj.try_add_adjacency(t_1, t_2);
            }
        }
        result = "adjacency list: " + adj.get_all() + ".";
        return result;

}

/// @brief Prints out disconnected vertices of given surface
/// @param mesh The target mesh
/// @param s_idx Surface index
/// @return 
String MeshUtils::find_disconnected_meshes(Ref<ArrayMesh> mesh, int s_idx){
    Ref<MeshDataTool> mdt = memnew(MeshDataTool);
	mdt->create_from_surface(mesh, s_idx);
    return find_disconnected_meshes_mdt(mdt);

}

void MeshUtils::_bind_methods(){
    ClassDB::bind_static_method("MeshUtils", D_METHOD("find_disconnected_meshes", "mesh", "s_idx"), &MeshUtils::find_disconnected_meshes);
}

MeshUtils::MeshUtils(){
    
}