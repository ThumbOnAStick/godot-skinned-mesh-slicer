#include "kd_tree.h"

#include <algorithm>
#include <limits>

namespace godot {

void KDTree::build(const std::vector<Point> &p_points) {
	m_points = p_points;
	m_nodes.clear();

	if (m_points.empty()) {
		return;
	}

	// Build index array referencing all points
	std::vector<uint32_t> indices(m_points.size());
	for (uint32_t i = 0; i < m_points.size(); ++i) {
		indices[i] = i;
	}

	m_nodes.reserve(m_points.size());
	build_recursive(indices, 0, static_cast<int32_t>(indices.size()), 0);
}

int32_t KDTree::build_recursive(std::vector<uint32_t> &p_indices, int32_t p_begin, int32_t p_end, int32_t p_depth) {
	int32_t count = p_end - p_begin;
	if (count <= 0) {
		return -1;
	}

	int32_t axis = p_depth % 3;

	// Sort the sub-range by the current axis
	std::sort(p_indices.begin() + p_begin, p_indices.begin() + p_end,
		[&](uint32_t a, uint32_t b) {
			return axis_value(m_points[a].position, axis) < axis_value(m_points[b].position, axis);
		});

	int32_t mid = p_begin + count / 2;

	Node node;
	node.point_index = p_indices[mid];
	int32_t node_idx = static_cast<int32_t>(m_nodes.size());
	m_nodes.push_back(node);

	// Recurse left
	int32_t left_idx = build_recursive(p_indices, p_begin, mid, p_depth + 1);
	m_nodes[node_idx].left = left_idx;

	// Recurse right
	int32_t right_idx = build_recursive(p_indices, mid + 1, p_end, p_depth + 1);
	m_nodes[node_idx].right = right_idx;

	return node_idx;
}

const KDTree::Point &KDTree::nearest(const Vector3 &p_target) const {
	if (m_nodes.empty()) {
		// Should not happen if build() was called with non-empty data.
		// Return first point as fallback.
		return m_points[0];
	}

	uint32_t best_idx = m_nodes[0].point_index;
	float best_dist_sq = m_points[best_idx].position.distance_squared_to(p_target);

	nearest_recursive(0, p_target, 0, best_idx, best_dist_sq);

	return m_points[best_idx];
}

void KDTree::nearest_recursive(int32_t p_node_idx, const Vector3 &p_target, int32_t p_depth,
							   uint32_t &r_best_idx, float &r_best_dist_sq) const {
	if (p_node_idx < 0) {
		return;
	}

	const Node &node = m_nodes[p_node_idx];
	const Point &pt = m_points[node.point_index];

	float d_sq = pt.position.distance_squared_to(p_target);
	if (d_sq < r_best_dist_sq) {
		r_best_dist_sq = d_sq;
		r_best_idx = node.point_index;
	}

	int32_t axis = p_depth % 3;
	float diff = axis_value(p_target, axis) - axis_value(pt.position, axis);

	// Choose nearer branch first
	int32_t nearer = (diff < 0.0f) ? node.left : node.right;
	int32_t farther = (diff < 0.0f) ? node.right : node.left;

	nearest_recursive(nearer, p_target, p_depth + 1, r_best_idx, r_best_dist_sq);

	// Check if we need to visit the farther branch
	if (diff * diff < r_best_dist_sq) {
		nearest_recursive(farther, p_target, p_depth + 1, r_best_idx, r_best_dist_sq);
	}
}

} // namespace godot
