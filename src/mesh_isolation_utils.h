#ifndef MESH_UTILS_H
#define MESH_UTILS_H

#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/mesh_data_tool.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/variant/typed_array.hpp>

#include <vector>

#include "mesh_isolation/fragment.h"
#include "mesh_isolation/triangle.h"

using namespace godot;

class MeshIsolationUtils : public Object {
	GDCLASS(MeshIsolationUtils, Object)

protected:
	static void _bind_methods();

public:
	static String find_disconnected_faces(Ref<ArrayMesh> mesh, int s_idx);
	static TypedArray<ArrayMesh> build_fragment_meshes(Ref<ArrayMesh> mesh, int s_idx);
	MeshIsolationUtils();
	
	private:
	static String find_disconnected_faces_mdt(Ref<MeshDataTool> mdt);
	static std::vector<Triangle> build_triangles(Ref<MeshDataTool> mdt);
	static std::vector<Fragment> find_disconnected_fragments(const std::vector<Triangle> &triangles);
	static void find_connected_faces_recursive(const std::vector<Triangle> &triangles, int pointer_idx, Fragment &fragment, std::vector<bool> &visited);
	static Triangle from_face(Ref<MeshDataTool> mdt, int f_idx);
};

#endif