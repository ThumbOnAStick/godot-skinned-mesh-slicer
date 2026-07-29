#ifndef SLICEABLE_MESH_INSTANCE_3D_H
#define SLICEABLE_MESH_INSTANCE_3D_H
namespace godot{
    struct Quad
    {
        public:
            int q1, q2, q3, q4;
            int t1, t2;
            int t1L, t1R, t2L, t2R;
            Quad(int q1, int q2, int q3, int q4, int t1, int t2, int t1L, int t1R, int t2L, int t2R);
    };
    
}
#endif