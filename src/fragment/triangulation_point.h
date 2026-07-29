#ifndef TRIANGULATION_POINT_H
#define TRIANGULATION_POINT_H

#include <binsort.h>
#include <godot_cpp/variant/vector2.hpp>

namespace godot{
    class TriangulationPoint : public IBinSortable {
        public:
            Vector2 coords;
            int bin;
            int index;
            TriangulationPoint(const int &index, const Vector2 &coords);
    };
}

#endif