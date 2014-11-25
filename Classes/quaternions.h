#ifndef __QUATERNIONS_H__
#define __QUATERNIONS_H__

typedef struct {
    float w, x, y, z;
} quaternion_t;

void euler_to_quaternion(float h, float p, float b, float* w0, float* x0, float* y0, float* z0);

// object->upright
void quaternion_to_euler_1(float w, float x, float y, float z, float* h, float* p, float* b);

// upright->object
void quaternion_to_euler_2(float w, float x, float y, float z, float* h, float* p, float* b);

void quaternion_to_matrix_1(float w, float x, float y, float z, float m[4][4], int column_major);

void slerp(float w0, float x0, float y0, float z0,
           float w1, float x1, float y1, float z1,
           float t,
           float *w, float *x, float *y, float *z);

void quaternion_mult(float w1, float x1, float y1, float z1,
                     float w2, float x2, float y2, float z2,
                     float *w, float *x, float *y, float *z);

void quaternion_conjugate(float w1, float x1, float y1, float z1,
                          float *w, float *x, float *y, float* z);

void quaternion_inverse(float w1, float x1, float y1, float z1,
                        float *w, float *x, float *y, float *z);

void quaternion_rotate(float qvw, float qvx, float qvy, float qvz,
					   float a,
					   float ux, float uy, float uz,
					   float *w1, float *x1, float *y1, float *z1);

// rotate v around axis u by r
void quaternion_rotate_inplace(quaternion_t* v, quaternion_t* u, float r);

void quaternion_rotate2(float qvw, float qvx, float qvy, float qvz,
						  float quw, float qux, float quy, float quz,
						  float *w, float *x, float *y, float *z);

void
get_body_vectors_for_euler(float alpha, float beta, float gamma,
                           quaternion_t* vec_x, quaternion_t* vec_y, quaternion_t* vec_z);

void
get_body_vectors_for_euler2(float euler[3], float vecx[3], float vecy[3], float vecz[3]);

void
get_euler_from_body_vectors(quaternion_t* vecx, quaternion_t* vecy, quaternion_t* vecz,
                            float* alpha, float* beta, float* gamma);

#endif