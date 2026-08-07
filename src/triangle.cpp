#include <triangle.h>

namespace godot {
    bool Triangle::try_add_neighbor(Triangle& triangle){
        int connected_count = 0;
        for (size_t this_v_idx = 0; this_v_idx < 3; this_v_idx++)
        {
            for (size_t other_v_idx = 0; other_v_idx < 3; other_v_idx++)
            {
                if(vertices[this_v_idx] == triangle[other_v_idx]){
                    connected_count ++;
                }    
            }
             
        }
        if (connected_count > 1){
            add_neighbor(triangle.index);
            return true;
        }
        return false;
    }

    void Triangle::add_neighbor(int t_idx){
        this->neighbors.push_back(t_idx);
    }

    Triangle Triangle::from_face(Ref<MeshDataTool> mdt, int f_idx){
        Triangle result = Triangle();
        result.index = f_idx;
        for (size_t v_idx = 0; v_idx < 3; v_idx++)
        {
           Vector3 v = mdt->get_vertex(mdt->get_face_vertex(f_idx, v_idx));
           result[v_idx] = v;
        }
        return result;
    }

    Triangle::Triangle(){
    }

} 