#include <triangulation_point.h>

namespace godot{
    TriangulationPoint::TriangulationPoint(const int &index, const Vector2 &coords){
        this->index = index;
        this->coords = coords;
    }

}