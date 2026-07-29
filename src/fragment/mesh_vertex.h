#ifndef MESH_VERTEX_H
#define MESH_VERTEX_H

#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/vector2.hpp>

namespace godot{
    class MeshVertex{
        public: 
            Vector3 position;
            Vector3 normal;
            Vector2 uv;
        MeshVertex(Vector3 position, Vector3 normal, Vector2 uv);
        bool operator==(const MeshVertex& other) const {      
            return (other).position == this->position;
        }     
        bool operator!=(const MeshVertex& other) const {      
            return (other).position != this->position;
        }
    };
}

#endif