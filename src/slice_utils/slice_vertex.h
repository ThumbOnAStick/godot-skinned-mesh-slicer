#ifndef SLICE_VERTEX_H
#define SLICE_VERTEX_H

#include <godot_cpp/variant/packed_float32_array.hpp>
#include <godot_cpp/variant/packed_int32_array.hpp>
#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/vector3.hpp>

using namespace godot;

namespace godot::SliceUtils {

/// A single vertex with all the attributes SurfaceTool needs to emit it.
struct SliceVertex {
	Vector3 position;
	Vector3 normal;
	Vector2 uv;
	PackedInt32Array bones;
	PackedFloat32Array weights;

	SliceVertex() = default;
	SliceVertex(const Vector3 &p_pos, const Vector3 &p_norm, const Vector2 &p_uv,
			const PackedInt32Array &p_bones, const PackedFloat32Array &p_weights);
};

} // namespace godot::SliceUtils

#endif // SLICE_VERTEX_H
