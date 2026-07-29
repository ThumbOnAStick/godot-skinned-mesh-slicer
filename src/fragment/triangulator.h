#ifndef TRIANGULATOR_H
#define TRIANGULATOR_H
#include <array>
#include <vector>
#include <list>
#include <godot_cpp/variant/vector3.hpp>
#include <triangulation_point.h>
#include <mesh_vertex.h>

namespace godot{
    class Triangulator{
        protected:
             const int V1 = 0; // Vertex 1
             const int V2 = 1; // Vertex 2
             const int V3 = 2; // Vertex 3
             const int E12 = 3; // Adjacency data for edge (V1 -> V2)
             const int E23 = 4; // Adjacency data for edge (V2 -> V3)
             const int E31 = 5; // Adjacency data for edge (V3 -> V1)

            const int SUPERTRIANGLE = 0;

            // Index for out of bounds triangle (boundary edge)
            const int OUT_OF_BOUNDS = -1;

            // Number of points to be triangulated (excluding super triangle vertices)
            int N;

            // Total number of triangles generated during triangulation
            int triangle_count;

             // Array which tracks which triangles should be ignored in the final triangulation
            std::vector<bool> skip_triangle;

            // Normal of the plane on which the points lie
            Vector3 normal;

            // Triangle vertex and adjacency data
            // Index 0 = Triangle index
            // Index 1 = [V1, V2, V3, E12, E23, E32]
            std::vector<int> triangulation;

            /// @brief Computes the triangulation of the point set.
            /// @return Returns true if the triangulation was successful
            bool compute_triangulation();

            /// @brief Initializes the triangulation by inserting the super triangle
            void add_supertriangle();

            /// @brief Restores the triangulation to a Delauney triangulation after new triangles have been added.
            /// @param p Index of the inserted point
            /// @param t1 Index of first triangle to check
            /// @param t2 Index of second triangle to check
            /// @param t3 Index of third triangle to check
            void restore_delauney_triangulation(TriangulationPoint p, const int &t1, const int &t2, const int &t3);

            /// @brief Swaps the diagonal of the quadrilateral formed by triangle `t` and the
            /// triangle adjacent to the edge that is opposite of the newly added point
            /// @param $p The index of the inserted point</param>
            /// @param $t1 Index of the triangle containing p
            /// @param $t2 Index of the triangle opposite t1 that shares edge E23 with t1
            /// @param $t3 Index of triangle adjacent to t1 after swap
            /// @param $t4 Index of triangle adjacent to t2 after swap
            /// @return 
            bool swap_quaddiagonal_ifneeded(const int $p, const int $t1, const int $t2, int $t3, int $t4);
        
            /// @brief Marks any triangles that contain super-triangle vertices as discarded 
            void discard_triangles_with_supertrianglevertices();

            /// @brief  Checks to see if the triangle formed by points v1->v2->v3 circumscribes point vP
            /// @param v1 Coordinates of 1st vertex of triangle
            /// @param v2 Coordinates of 2nd vertex of triangle
            /// @param v3 Coordinates of 3rd vertex of triangle
            /// @param v4 Coordinates of test point
            /// @return 
            bool swap_test(const Vector2 &v1, const Vector2 &v2, const Vector2 &v3, const Vector2 &v4);

            /// @brief Checks if the triangle `t` contains the specified vertex
            /// @param t The index of the triangle
            /// @param v The index of the vertex
            /// @return Returns true if the triangle `t` contains the vertex `v`
            bool triangle_contains_vertex(const int &t, const int &v);

            /// @brief Updates the adjacency information in triangle `t`. Any references to `tOld are
            /// @param t The index of the triangle to update
            /// @param t_old The index to be replaced
            /// @param t_new The new index to replace with
            void update_adjacency(const int &t, const int &t_old, const int &t_new);


            /// @brief Finds the edge index for triangle `tOrigin` that is adjacent to triangle `tAdjacent`
            /// @param t_origin The origin triangle to search
            /// @param t_adjacent The triangle index to search for
            /// @param edge_index Edge index returned as an out parameter
            /// @return True if `tOrigin` is adjacent to `tAdjacent` and supplies the
            /// shared edge index via the out parameter. If `tOrigin` is an invalid index or
            /// `tAdjacent` is not adjacent to `tOrigin`, returns false.
            bool find_shared_edge(int t_origin, int t_adjacent, int edge_index);
        
        public:
            std::vector<TriangulationPoint> points;
            float normalizationScaleFactor = 1.0;
            /// @brief Initializes the triangulator with the vertex data to be triangulated
            /// @param intput_points The points to triangulate 
            /// @param normal The normal of the triangulation plane
            Triangulator(const std::vector<MeshVertex> &intput_points, const Vector3 &normal);

            /// @brief Performs the triangulation
            /// @return Returns an array containing the indices of the triangles, mapped to the list of points passed in during initialization
            virtual std::vector<int> triangulate();

            /// @brief Uniformly scales the 2D coordinates of all the points between [0, 1]
            void normalize_coordinates();

            /// @brief Sorts the points into bins using an ordered grid
            /// @return Returns the array of sorted points
            std::vector<TriangulationPoint> sort_points_into_bins();

        
    };

}

#endif