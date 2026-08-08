#include "slice_geometry.h"

#include <godot_cpp/variant/packed_float32_array.hpp>
#include <godot_cpp/variant/packed_int32_array.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace godot::SliceUtils {

ClipPoint interpolate_crossing(const Plane &p_plane,
		const Vector3 &p_below, const Vector3 &p_above,
		const Vector3 &p_below_normal, const Vector3 &p_above_normal,
		const Vector2 &p_below_uv, const Vector2 &p_above_uv) {

	ClipPoint result;
	p_plane.intersects_ray(p_below, p_above - p_below, &result.position); // point on plane between below and above

	float interpolator = Math::sqrt(p_below.distance_squared_to(result.position) / p_below.distance_squared_to(p_above));

	result.uv = p_below_uv.lerp(p_above_uv, interpolator);
	result.normal = p_below_normal.lerp(p_above_normal, interpolator); // a slerp would be better, but lerp is fine here

	return result;
}

SliceVertex clip_vertex(const ClipPoint &p_clip,
		const PackedInt32Array &p_bones, const PackedFloat32Array &p_weights) {
	return SliceVertex(p_clip.position, p_clip.normal, p_clip.uv, p_bones, p_weights);
}

} // namespace godot::SliceUtils
