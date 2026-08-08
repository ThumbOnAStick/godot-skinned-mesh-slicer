#ifndef SLICE_GEOMETRY_H
#define SLICE_GEOMETRY_H

#include "slice_vertex.h"

#include <godot_cpp/variant/plane.hpp>
#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/vector3.hpp>

using namespace godot;

namespace godot::SliceUtils {

/// A point on the cutting plane, carrying the interpolated attributes.
struct ClipPoint {
	Vector3 position;
	Vector3 normal;
	Vector2 uv;
};

/// Computes the crossing point where a face edge (running from a below-plane
/// vertex to an above-plane vertex) intersects p_plane, and interpolates the
/// uv/normal at that point. The interpolation factor uses the sqrt-of-ratio
/// scheme of the original slicing code.
///
/// \param p_plane         The cutting plane.
/// \param p_below         Below-plane endpoint position.
/// \param p_above         Above-plane endpoint position.
/// \param p_below_normal  Normal at the below-plane endpoint.
/// \param p_above_normal  Normal at the above-plane endpoint.
/// \param p_below_uv      UV at the below-plane endpoint.
/// \param p_above_uv      UV at the above-plane endpoint.
ClipPoint interpolate_crossing(const Plane &p_plane,
		const Vector3 &p_below, const Vector3 &p_above,
		const Vector3 &p_below_normal, const Vector3 &p_above_normal,
		const Vector2 &p_below_uv, const Vector2 &p_above_uv);

/// Builds a SliceVertex on the plane from a ClipPoint, reusing the supplied
/// bone data (bones/weights are taken from the below-plane endpoint, not interpolated).
SliceVertex clip_vertex(const ClipPoint &p_clip,
		const PackedInt32Array &p_bones, const PackedFloat32Array &p_weights);

} // namespace godot::SliceUtils

#endif // SLICE_GEOMETRY_H
