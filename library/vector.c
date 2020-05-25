#include "vector.h"
#include <stdio.h>
#include <math.h>

const vector_t VEC_ZERO = { 0.0, 0.0 };
const double THIS_IS_A_HALF = 0.5;

// Checks if v3 is in the rectangular region bounded by v1 and v2
bool vector_in_region(vector_t v1, vector_t v2, vector_t v3) {
    // v1 is bottom left v2 is top right
    if (v3.x >= v1.x && v3.x <= v2.x && v3.y >= v1.y && v3.y <= v2.y) {
        return true;
    }

    return false;
}

vector_t vec_add(vector_t v1, vector_t v2) {
    vector_t vec = { (v1.x + v2.x), (v1.y + v2.y) };
    return vec;
}

vector_t vec_multiply(double scalar, vector_t v) {
    vector_t result = {scalar * v.x, scalar *v.y };
    return result;
}

vector_t vec_negate(vector_t v) {
    return vec_multiply(-1.0, v);
}

vector_t vec_subtract(vector_t v1, vector_t v2) {
    return vec_add(v1, vec_negate(v2));
}

double vec_dot(vector_t v1, vector_t v2) {
    return (v1.x*v2.x + v1.y*v2.y);
}

double vec_cross(vector_t v1, vector_t v2) {
    return (v1.x * v2.y) - (v2.x * v1.y);
}

double vec_norm(vector_t v){
    return sqrt(vec_dot(v,v));
}

vector_t vec_rotate(vector_t v, double angle) {
    // First create the rotation matrix that will be represented by two vectors
    vector_t r_matx = { cos(angle), -1 * sin(angle) };
    vector_t r_maty = { sin(angle), cos(angle) };
    vector_t result = { vec_dot(r_matx, v), vec_dot(r_maty, v) };

    return result;
}

vector_t vec_average(vector_t v1, vector_t v2) {
    return vec_multiply(THIS_IS_A_HALF, vec_add(v1, v2));
}

bool vec_isequal(vector_t v1, vector_t v2) {
    return v1.x == v2.x && v1.y == v2.y;
}
