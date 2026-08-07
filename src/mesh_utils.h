#ifndef MESH_UTILS_H
#define MESH_UTILS_H

#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/mesh_data_tool.hpp>
#include <godot_cpp/classes/object.hpp>

#include <vector>

#include "triangle.h"

using namespace godot;

class MeshUtils : public Object {
	GDCLASS(MeshUtils, Object)

protected:
	static void _bind_methods();

public:
	static String find_disconnected_faces(Ref<ArrayMesh> mesh, int s_idx);
	MeshUtils();
	
	private:
	static String find_disconnected_faces_mdt(Ref<MeshDataTool> mdt);
	static void find_connected_faces_recursive(std::vector<Triangle> &triangles, int pointer_idx, std::vector<int> &found, std::vector<bool> &visited);
	static Triangle from_face(Ref<MeshDataTool> mdt, int f_idx);
};

#endif