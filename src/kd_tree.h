#ifndef KD_TREE_H
#define KD_TREE_H

#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/packed_int32_array.hpp>
#include <godot_cpp/variant/packed_float32_array.hpp>

#include <vector>
#include <cstdint>

namespace godot {

/// A lightweight KD-tree for 3D spatial nearest-neighbor lookup.
/// Stores vertex positions and their associated bone weights/indices.
/// Not exposed to GDScript — used internally by the mesh slicer.
class KDTree {
public:
	/// A single entry in the KD-tree: a 3D point + bone data from the original vertex.
	struct Point {
		Vector3 position;
		PackedInt32Array bones;
		PackedFloat32Array weights;

		Point() = default;
		Point(const Vector3 &p_pos, const PackedInt32Array &p_bones, const PackedFloat32Array &p_weights)
			: position(p_pos), bones(p_bones), weights(p_weights) {}
	};

	KDTree() = default;

	/// Build the tree from a set of points. Destroys any previous tree.
	void build(const std::vector<Point> &p_points);

	/// Returns the Point closest to p_target. Behaviour is undefined if build() was never called.
	const Point &nearest(const Vector3 &p_target) const;

	/// True after a successful build().
	bool is_built() const { return !m_points.empty(); }

private:
	struct Node {
		uint32_t point_index = 0;
		int32_t left = -1;   // index into m_nodes array, -1 = null
		int32_t right = -1;
	};

	std::vector<Point> m_points;   // all points (owned here)
	std::vector<Node> m_nodes;     // tree nodes, index into m_points

	/// Recursive build helper. Returns node index (into m_nodes), or -1.
	int32_t build_recursive(std::vector<uint32_t> &p_indices, int32_t p_begin, int32_t p_end, int32_t p_depth);

	/// Recursive nearest-neighbor search.
	void nearest_recursive(int32_t p_node_idx, const Vector3 &p_target, int32_t p_depth,
						   uint32_t &r_best_idx, float &r_best_dist_sq) const;

	/// Axis helper: picks x/y/z based on depth.
	static float axis_value(const Vector3 &p_v, int32_t p_axis) {
		return (p_axis == 0) ? p_v.x : ((p_axis == 1) ? p_v.y : p_v.z);
	}
};



} // namespace godot

#endif // KD_TREE_H
