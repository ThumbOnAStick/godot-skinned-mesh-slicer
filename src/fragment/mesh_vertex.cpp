#include <mesh_vertex.h>

namespace godot{
    MeshVertex::MeshVertex(Vector3 position, Vector3 normal, Vector2 uv){
        this->normal = normal;
        this->position = position;
        this->uv = uv;
    }
}