#include <triangulator.h>
#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/vector2.hpp>
#include <limits>  
#include <algorithm>  
#include <cmath> 

namespace godot{
    Triangulator::Triangulator(const std::vector<MeshVertex> &intput_points, const Vector3 &normal){
        // Need at least three input vertices to triangulate
        if (intput_points.empty()  || intput_points.size() < 3)
        {
            return;
        }

        this->N = intput_points.size();
        this->triangle_count = 2 * N + 1;
        this->triangulation = std::vector<int>(triangle_count * 6);
        this->skip_triangle = std::vector<bool>(triangle_count);
        this->points = std::vector<TriangulationPoint>(N + 3); // Extra 3 points used to store super triangle
        this->normal = normal;
        
        // Choose two points in the plane as one basis vector
        Vector3 e1 = (intput_points[0].position - intput_points[1].position).normalized();
        Vector3 e2 = normal.normalized();
        Vector3 e3 = e1.cross(e2).normalized();

        // To find the 2nd basis vector, find the largest component and swap with the smallest, negating the largest
        
        // Project 3D vertex onto the 2D plane
        for (int i = 0; i < N; i++)
        {
            Vector3 position = intput_points[i].position;
            Vector2 coords = Vector2(position.dot(e1), position.dot(e3));
            this->points[i] = TriangulationPoint(i, coords);
        }
    }

    std::vector<int> Triangulator::triangulate(){
         // Need at least 3 vertices to triangulate
        if (N < 3) 
        {
            return std::vector<int>();
        }

        this->add_supertriangle();
        this->normalize_coordinates();
        this->compute_triangulation();
        this->discard_triangles_with_supertrianglevertices();

        std::vector<int> triangles = std::vector<int>(3 * triangle_count);
        for (int i = 0; i < triangle_count; i++)
        {
            // Add all triangles that don't contain a super-triangle vertex
            if (!skip_triangle[i])
            {
                triangles.push_back(triangulation[i * 6 + V1]);
                triangles.push_back(triangulation[i * 6 + V2]);
                triangles.push_back(triangulation[i * 6 + V3]);
            }
        }

        return triangles;
    }

    void Triangulator::normalize_coordinates(){
         // 1) Normalize coordinates. Coordinates are scaled so they lie between 0 and 1
        // The scaling should be uniform so relative positions of points are unchanged

        float xMin = -std::numeric_limits<float>::infinity();
        float xMax = -xMin;
        float yMin = xMin;
        float yMax = xMax;

        // Find min/max points in the set
        for (int i = 0; i < N; i++)
        {
            TriangulationPoint point = points[i];
            if (point.coords.x < xMin) xMin = point.coords.x;
            if (point.coords.y < yMin) yMin = point.coords.y;
            if (point.coords.x > xMax) xMax = point.coords.x;
            if (point.coords.y > yMax) yMax = point.coords.y;
        }

        // Normalization coefficient. Using same coefficient for both x & y
        // ensures uniform scaling
        normalizationScaleFactor = std::max(xMax - xMin, yMax - yMin);

        // Normalize each point
        for (int i = 0; i < N; i++)
        {
            TriangulationPoint point = points[i];
            Vector2 normalizedPos = Vector2(
                (point.coords.x - xMin) / normalizationScaleFactor,
                (point.coords.y - yMin) / normalizationScaleFactor);

            points[i].coords = normalizedPos;            
        }
    }

    std::vector<TriangulationPoint> Triangulator::sort_points_into_bins(){
        // Compute the number of bins along each axis
        int n = std::round(std::pow((float) N, 0.25f));
        
        // Total bin count
        int binCount = n * n;

        // Assign bin numbers to each point by taking the normalized coordinates
        // and dividing them into a n x n grid.
        for (int k = 0; k < N; k++)
        {
            TriangulationPoint &point = this->points[k];  
            int i = (int) (0.99f * n * point.coords.y);
            int j = (int) (0.99f * n * point.coords.x);
            point.bin = BinSort::get_bin_number(i, j, n);
        }

        return BinSort::sort<TriangulationPoint>(this->points, N, binCount);
    }
}