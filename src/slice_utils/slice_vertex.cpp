#include "slice_vertex.h"

using namespace godot;

namespace godot::SliceUtils {

SliceVertex::SliceVertex(const Vector3 &p_pos, const Vector3 &p_norm, const Vector2 &p_uv,
		const PackedInt32Array &p_bones, const PackedFloat32Array &p_weights)
	: position(p_pos), normal(p_norm), uv(p_uv), bones(p_bones), weights(p_weights) {}

} // namespace godot::SliceUtils
