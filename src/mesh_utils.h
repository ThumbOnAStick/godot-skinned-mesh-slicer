#ifndef MESHUTILS_H
#define MESHUTILS_H

#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/mesh_data_tool.hpp>
#include <godot_cpp/classes/object.hpp>

#include <dfs.h>

using namespace godot;

class MeshUtils : public Object {
	GDCLASS(MeshUtils, Object)

protected:
	static void _bind_methods();

public:
	static String find_disconnected_meshes(Ref<ArrayMesh> mesh, int s_idx);
	MeshUtils();

private:
	static String find_disconnected_meshes_mdt(Ref<MeshDataTool> mdt);
	static Triangle from_face(Ref<MeshDataTool> mdt, int f_idx);
};

#endif