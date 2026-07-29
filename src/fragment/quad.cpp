#include <quad.h>

namespace godot{
    Quad::Quad(int q1, int q2, int q3, int q4, int t1, int t2, int t1L, int t1R, int t2L, int t2R){
        this->q1 = q1;
        this->q2 = q2;
        this->q3 = q3;
        this->q4 = q4;
        this->t1 = t1;
        this->t2 = t2;
        this->t1L = t1L;
        this->t1R = t1R;
        this->t2L = t2L;
        this->t2R = t2R;
    }

}