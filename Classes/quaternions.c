// Justin Brady justin@domain17.net 3-21-2012
// my slerp demonstration

#include <math.h>
#include "quaternions.h"

static const float pi = 3.14159265;

void euler_to_quaternion(float h, float p, float b, float* w0, float* x0, float* y0, float* z0)
{
    float w = 
        cos(h/2)*cos(p/2)*cos(b/2) + sin(h/2)*sin(p/2)*sin(b/2);

    float x =
        cos(h/2)*sin(p/2)*cos(b/2) + sin(h/2)*cos(p/2)*sin(b/2);

    float y =
        sin(h/2)*cos(p/2)*cos(b/2) - cos(h/2)*sin(p/2)*sin(b/2);

    float z =
        cos(h/2)*cos(p/2)*sin(b/2) - sin(h/2)*sin(p/2)*cos(b/2);

    *w0 = w;
    *x0 = x;
    *y0 = y;
    *z0 = z;
}

// object->upright
void quaternion_to_euler_1(float w, float x, float y, float z, float* h, float* p, float* b)
{
    float sp = -2.0 * ((y*z) - (w*x));

    if(fabs(sp) > 0.9999f)
    {
        *p = (pi/2) * sp;
        *h = atan2(-x*z + (w*y), 0.5 - (y*y) - (z*z));
        *b = 0;
    }
    else
    {
        *p = asin(sp);
        *h = atan2((x*z) + (w*y), 0.5 - (x*x) - (y*y));
        *b = atan2((x*y) + (w*z), 0.5 - (x*x) - (z*z));
    }
}

// upright->object
void quaternion_to_euler_2(float w, float x, float y, float z, float* h, float* p, float* b)
{
    float sp = -2.0 * ((y*z) + (w*x));

    if(fabs(sp) > 0.9999f)
    {
        *p = (pi/2) * sp;
        *h = atan2(-x*z - (w*y), 0.5 - (y*y) - (z*z));
        *b = 0;
    }
    else
    {
        *p = asin(sp);
        *h = atan2((x*z) - (w*y), 0.5 - (x*x) - (y*y));
        *b = atan2((x*y) - (w*z), 0.5 - (x*x) - (z*z));
    }
}

void quaternion_to_matrix_1(float w, float x, float y, float z, float m[4][4], int column_major)
{
	if (column_major)
	{
		m[0][0] = 1 - 2*(y*y) - 2*(z*z);
		m[0][1] = 2*x*y - 2*w*z;
		m[0][2] = 2*x*z + 2*w*y;
		
		m[1][0] = 2*x*y + 2*w*z;
		m[1][1] = 1 - 2*(x*x) - 2*(z*z);
		m[1][2] = 2*y*z - 2*w*x;
		
		m[2][0] = 2*x*z - 2*w*y;
		m[2][1] = 2*y*z + 2*w*x;
		m[2][2] = 1 - 2*(x*x) - 2*(y*y);
	}
	else {
		m[0][0] = 1 - 2*(y*y) - 2*(z*z);
		m[1][0] = 2*x*y - 2*w*z;
		m[2][0] = 2*x*z + 2*w*y;
		
		m[0][1] = 2*x*y + 2*w*z;
		m[1][1] = 1 - 2*(x*x) - 2*(z*z);
		m[2][1] = 2*y*z - 2*w*x;
		
		m[0][2] = 2*x*z - 2*w*y;
		m[1][2] = 2*y*z + 2*w*x;
		m[2][2] = 1 - 2*(x*x) - 2*(y*y);
	}
}

void slerp(float w0, float x0, float y0, float z0,
           float w1, float x1, float y1, float z1,
           float t,
           float *w, float *x, float *y, float *z)
{
    float cosOmega = w0*w1 + x0*x1 + y0*y1 + z0*z1;
    
    if(cosOmega < 0.0)
    {
        w1 = -w1;
        x1 = -x1;
        y1 = -y1;
        z1 = -z1;
        cosOmega = -cosOmega;
    }

    float k0, k1;

    if (cosOmega > 0.9999)
    {
        k0 = 1.0-t;
        k1 = t;
    }
    else
    {
        float sinOmega = sqrt(1.0 - cosOmega*cosOmega);

        float omega = atan2(sinOmega, cosOmega);

        float oneOverSinOmega = 1.0 / sinOmega;

        k0 = sin((1.0-t) * omega) * oneOverSinOmega;
        k1 = sin(t * omega) * oneOverSinOmega;
    }

    *w = w0*k0 + w1*k1;
    *x = x0*k0 + x1*k1;
    *y = y0*k0 + y1*k1;
    *z = z0*k0 + z1*k1;
}

void quaternion_mult(float w1, float x1, float y1, float z1,
                     float w2, float x2, float y2, float z2,
                     float *w, float *x, float *y, float *z)
{
    *w = w1*w2 - x1*x2 - y1*y2 - z1*z2;
    *x = w1*x2 + x1*w2 + y1*z2 - z1*y2;
    *y = w1*y2 + y1*w2 + z1*x2 - x1*z2;
    *z = w1*z2 + z1*w2 + x1*y2 - y1*x2;
}

void quaternion_conjugate(float w1, float x1, float y1, float z1,
                          float *w, float *x, float *y, float* z)
{
    *w = w1;
    *x = -x1;
    *y = -y1;
    *z = -z1;
}

void quaternion_inverse(float w1, float x1, float y1, float z1,
                        float *w, float *x, float *y, float *z)
{
    // inverse is conjugate divided by magnitude
    float cw, cx, cy, cz;
    quaternion_conjugate(w1, x1, y1, z1, &cw, &cx, &cy, &cz);
    float mag = sqrt(cw*cw + cx*cx + cy*cy + cz*cz);
    cw /= mag;
    cx /= mag;
    cy /= mag;
    cz /= mag;

    *w = cw;
    *x = cx;
    *y = cy;
    *z = cz;
}

void quaternion_rotate_test()
{
    // rotate point V by a about the axis u
    float vx = 1;
    float vy = 0;
    float vz = 0;

    float qvw = 0;
    float qvx = vx;
    float qvy = vy;
    float qvz = vz;

    float ux = 0;
    float uy = 0;
    float uz = 1;

    float theta = pi/2; // 90 degrees
    float thetaDiv2 = theta/2;
    float quw = cos(thetaDiv2);
    float qux = sin(thetaDiv2)*ux;
    float quy = sin(thetaDiv2)*uy;
    float quz = sin(thetaDiv2)*uz;

    // conjugation 
    // v_1 = quw(qvw)inverse(quw)

    float iw, ix, iy, iz;
    quaternion_inverse(quw, qux, quy, quz,
                        &iw, &ix, &iy, &iz);
    float w, x, y, z;
    quaternion_mult(quw, qux, quy, quz,
                    qvw, qvx, qvy, qvz,
                    &w, &x, &y, &z);
    float w2, x2, y2, z2;
    quaternion_mult(w, x, y, z,
                    iw, ix, iy, iz,
                    &w2, &x2, &y2, &z2);
    w = w2;
    x = x2;
    y = y2;
    z = z2;

	/*
    cout << "w=" << w << endl
         << "x=" << x << endl
         << "y=" << y << endl
         << "z=" << z
         << endl;
	 */
}

void quaternion_rotate(float qvw, float qvx, float qvy, float qvz, float a, float ux, float uy, float uz, float *w1, float *x1, float *y1, float *z1)
{
    // rotate point V by a about the axis u
	
    float theta = a;
    float thetaDiv2 = theta/2;
    float quw = cos(thetaDiv2);
    float qux = sin(thetaDiv2)*ux;
    float quy = sin(thetaDiv2)*uy;
    float quz = sin(thetaDiv2)*uz;
	
    // conjugation 
    // v_1 = quw(qvw)inverse(quw)
	
    float iw, ix, iy, iz;
    quaternion_inverse(quw, qux, quy, quz,
					   &iw, &ix, &iy, &iz);
    float w, x, y, z;
    quaternion_mult(quw, qux, quy, quz,
                    qvw, qvx, qvy, qvz,
                    &w, &x, &y, &z);
    float w2, x2, y2, z2;
    quaternion_mult(w, x, y, z,
                    iw, ix, iy, iz,
                    &w2, &x2, &y2, &z2);
    *w1 = w2;
    *x1 = x2;
    *y1 = y2;
    *z1 = z2;
}

void quaternion_rotate2(float qvw, float qvx, float qvy, float qvz,
						  float quw, float qux, float quy, float quz,
						  float *w1, float *x1, float *y1, float *z1)
{
	float iw, ix, iy, iz;
    quaternion_inverse(quw, qux, quy, quz,
					   &iw, &ix, &iy, &iz);
    float w, x, y, z;
    quaternion_mult(quw, qux, quy, quz,
                    qvw, qvx, qvy, qvz,
                    &w, &x, &y, &z);
    float w2, x2, y2, z2;
    quaternion_mult(w, x, y, z,
                    iw, ix, iy, iz,
                    &w2, &x2, &y2, &z2);
    *w1 = w2;
    *x1 = x2;
    *y1 = y2;
    *z1 = z2;
}

// rotate v around axis u by r
void quaternion_rotate_inplace(quaternion_t* v, const quaternion_t* _u, float r)
{
    quaternion_t u_inv;
    quaternion_t u;
    quaternion_t uv;
    
    float theta = r;
    float thetaDiv2 = theta/2;
    u.w = cos(thetaDiv2);
    u.x = sin(thetaDiv2)*_u->x;
    u.y = sin(thetaDiv2)*_u->y;
    u.z = sin(thetaDiv2)*_u->z;
    
    quaternion_inverse(u.w, u.x, u.y, u.z,
                       &u_inv.w, &u_inv.x, &u_inv.y, &u_inv.z);
    quaternion_mult(u.w, u.x, u.y, u.z,
                    v->w, v->x, v->y, v->z,
                    &uv.w, &uv.x, &uv.y, &uv.z);
    quaternion_mult(uv.w, uv.x, uv.y, uv.z,
                    u_inv.w, u_inv.x, u_inv.y, u_inv.z,
                    &v->w, &v->x, &v->y, &v->z);
}

void slerp_body_axes_test()
{
    float roll = pi/2;
    float pitch = 0;
    float yaw = 0;

    float w1, x1, y1, z1;
    euler_to_quaternion(roll, pitch, yaw, &w1, &x1, &y1, &z1);

    float w2, x2, y2, z2;
    euler_to_quaternion(roll, pitch+(pi), yaw, &w2, &x2, &y2, &z2);

    float w3, x3, y3, z3;
    euler_to_quaternion(roll, pitch, yaw+(pi), &w3, &x3, &y3, &z3);

    //find normal vectors for current orientation
    // or... find quaternion representing 90degrees along
    // axis we want to slerp towards
    float w, x, y, z;
    slerp(w1, x1, y1, z1,
          w2, x2, y2, z2,
          0.5,
          &w, &x, &y, &z); // pitch up 

    float h, p, b;
    quaternion_to_euler_1(w, x, y, z, &h, &p, &b);

	/*
    cout << "h=" << h << endl;
    cout << "p=" << p << endl;
    cout << "b=" << b << endl;
	 */
}

void
get_body_vectors_for_euler(float alpha, float beta, float gamma,
                           quaternion_t* vec_x, quaternion_t* vec_y, quaternion_t* vec_z)
{
    quaternion_t zero = {1, 0, 0, 0};
    
    *vec_x = zero;
    *vec_y = zero;
    *vec_z = zero;
    
    vec_z->z = 1;
    vec_x->x = 1;
    vec_y->y = 1;
    
    // Z
    quaternion_rotate_inplace(vec_y, vec_z, alpha);
    quaternion_rotate_inplace(vec_x, vec_z, alpha);
    // X'
    quaternion_rotate_inplace(vec_z, vec_x, beta);
    quaternion_rotate_inplace(vec_y, vec_x, beta);
    // Z''
    quaternion_rotate_inplace(vec_x, vec_z, gamma);
    quaternion_rotate_inplace(vec_y, vec_z, gamma);
}

void
get_euler_from_body_vectors(quaternion_t* vecx, quaternion_t* vecy, quaternion_t* vecz,
                            float* alpha, float* beta, float* gamma)
{
    *alpha = atan2(vecz->x, -vecz->y);
    
    // jb: very rarely seen with gameAI.c probably due to slerp
    if(vecz->z >= 1.0)
    {
        vecz->z = 1.0 - vecz->z;
    }
    else if(vecz->z <= -1.0)
    {
        vecz->z = -1 - vecz->z;
    }
 
    *beta = acos(vecz->z);
    
    *gamma = atan2(vecx->z, vecy->z);
}

void
get_body_vectors_for_euler2(float euler[3], float vecx[3], float vecy[3], float vecz[3])
{
    quaternion_t qx, qy, qz;
    
    get_body_vectors_for_euler(euler[0], euler[1], euler[2], &qx, &qy, &qz);
    
    vecx[0] = qx.x; vecx[1] = qx.y; vecx[2] = qx.z;
    vecy[0] = qy.x; vecy[1] = qy.y; vecy[2] = qy.z;
    vecz[0] = qz.x; vecz[1] = qz.y; vecz[2] = qz.z;
}

void
eulerHeading1(float arrowV[3], float eu[3])
{
    float d = sqrt(arrowV[0]*arrowV[0] + arrowV[1]*arrowV[1] + arrowV[2]*arrowV[2]);
    
    float o = atan2(arrowV[0] / d, arrowV[2] / d);
    
    float k = sqrt(arrowV[0]*arrowV[0] + 0 + arrowV[2]*arrowV[2]);
    float p = atan2(arrowV[1] / d, k / d);
    
    quaternion_t bx = QX;
    quaternion_t by = QY;
    quaternion_t bz = QZ;
    
    quaternion_rotate_inplace(&bz, &QY, o+M_PI);
    quaternion_rotate_inplace(&bx, &QY, o+M_PI);
    quaternion_rotate_inplace(&bz, &bx, p);
    quaternion_rotate_inplace(&by, &bx, p);
    
    get_euler_from_body_vectors(&bx, &by, &bz,
                                &eu[0], &eu[1], &eu[2]);
    
}

extern float dot(float, float, float, float, float, float);

void
vector_relative_to_plane(quaternion_t *qA, quaternion_t *qU, quaternion_t *qV)
{
    quaternion_t svr;
    
    // if dot product < 1 reverse
    // TODO: < 0?
    float siU = dot(qA->x, qA->y, qA->z, qU->x, qU->y, qU->z) < 0.99 ? -1 : 1;
    qU->w *= siU; qU->x *= siU; qU->y *= siU; qU->z *= siU;
    
    // s-lerp qA towards qU
    slerp(qA->w, qA->x, qA->y, qA->z,
          qU->w, qU->x, qU->y, qU->z,
          0.99,
          &svr.w, &svr.x, &svr.y, &svr.z);
    
    *qA = svr;
    
    // calculate dot product between result and qV
    float siV = dot(qA->x, qA->y, qA->z, qV->x, qV->y, qV->z) < 0.99 ? -1 : 1;
    
    qV->w *= siV; qV->x *= siV; qV->y *= siV; qV->z *= siV;
    
    // s-lerp the result towards qV
    slerp(qA->w, qA->x, qA->y, qA->z,
          qV->w, qV->x, qV->y, qV->z,
          0.99,
          &svr.w, &svr.x, &svr.y, &svr.z);
    
    *qA = svr;
}

/*
int main(int argc, char** argv)
{
    quaternion_rotate_test();

    slerp_body_axes_test();
}
 */
