#ifndef SLICEABLE_MESH_UTILS_H
#define SLICEABLE_MESH_UTILS_H

// Public facade for the mesh slicing utilities.
// Pulls in the split modules so existing code can keep including
// "sliceable_mesh_utils.h" without referencing the internal layout.

#include "slice_utils/slice_vertex.h"
#include "slice_utils/slice_cache.h"
#include "slice_utils/slice_geometry.h"
#include "slice_utils/slice_emitter.h"
#include "slice_utils/slice_slicer.h"

#endif // SLICEABLE_MESH_UTILS_H
