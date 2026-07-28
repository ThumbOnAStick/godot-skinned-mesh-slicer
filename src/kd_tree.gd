## KD-Tree for 3D spatial nearest-neighbor lookup.
## Stores vertex positions and associated bone weights/indices from the original mesh.
## Used by the mesh slicer to copy bone data from the closest original vertex
## to each newly created lid vertex.

class_name KDNode
extends RefCounted

var k := 0           ## split axis: 0=x, 1=y, 2=z
var point := Vector3()
var bones: PackedInt32Array
var weights: PackedFloat32Array
var left: KDNode = null
var right: KDNode = null


func _init(_point := Vector3(), _k := 0, _bones := PackedInt32Array(), _weights := PackedFloat32Array()) -> void:
	point = _point
	k = _k
	bones = _bones
	weights = _weights


## Find the nearest neighbor to [param target] in the tree rooted at [param root].
static func nearest_neighbor(root: KDNode, target: Vector3, depth := 0, best: KDNode = null) -> KDNode:
	if root == null:
		return best

	var axis = depth % 3
	var next_best: KDNode
	var next_branch: KDNode

	if best == null or target.distance_squared_to(root.point) < target.distance_squared_to(best.point):
		next_best = root
	else:
		next_best = best

	var other_branch: KDNode
	if target[axis] < root.point[axis]:
		next_branch = root.left
		other_branch = root.right
	else:
		next_branch = root.right
		other_branch = root.left

	best = nearest_neighbor(next_branch, target, depth + 1, next_best)
	if target.distance_squared_to(best.point) > abs(target[axis] - root.point[axis]) ** 2:
		best = nearest_neighbor(other_branch, target, depth + 1, best)

	return best


## Build a KD-tree from arrays of positions, bone indices, and bone weights.
## All three arrays must have the same length.
static func build_kd_tree(
	positions: Array,
	bones_list: Array,
	weights_list: Array,
	depth := 0
) -> KDNode:
	if positions.is_empty():
		return null

	var new_axis = depth % 3

	# Sort points according to the current axis
	var indices := range(positions.size())
	indices.sort_custom(func(a, b):
		return positions[a][new_axis] < positions[b][new_axis]
	)

	var median_idx: int = indices.size() / 2
	var median_i: int = indices[median_idx]

	var node := KDNode.new(positions[median_i], new_axis, bones_list[median_i], weights_list[median_i])

	# Build left and right subtrees by partitioning the sorted indices
	var left_positions := []
	var left_bones := []
	var left_weights := []
	for i in range(0, median_idx):
		left_positions.append(positions[indices[i]])
		left_bones.append(bones_list[indices[i]])
		left_weights.append(weights_list[indices[i]])

	var right_positions := []
	var right_bones := []
	var right_weights := []
	for i in range(median_idx + 1, indices.size()):
		right_positions.append(positions[indices[i]])
		right_bones.append(bones_list[indices[i]])
		right_weights.append(weights_list[indices[i]])

	node.left = build_kd_tree(left_positions, left_bones, left_weights, depth + 1)
	node.right = build_kd_tree(right_positions, right_bones, right_weights, depth + 1)

	return node



