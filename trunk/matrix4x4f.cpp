#include <stdlib.h>
#include <stdio.h>
#include "matrix4x4f.h"

matrix4x4f::matrix4x4f() {
       
        identity();
       
}

matrix4x4f::matrix4x4f( float m0, float m4, float  m8, float m12,
                        float m1, float m5, float  m9, float m13,
                        float m2, float m6, float m10, float m14,
                        float m3, float m7, float m11, float m15 ) {
                       
    m[0]=m0; m[4]=m4; m[8] =m8;  m[12]=m12;
    m[1]=m1; m[5]=m5; m[9] =m9;  m[13]=m13;
    m[2]=m2; m[6]=m6; m[10]=m10; m[14]=m14;
    m[3]=m3; m[7]=m7; m[11]=m11; m[15]=m15;

}

void matrix4x4f::identity() {

    m[0]=1.0f; m[4]=0.0f; m[8] =0.0f; m[12]=0.0f;
    m[1]=0.0f; m[5]=1.0f; m[9] =0.0f; m[13]=0.0f;
    m[2]=0.0f; m[6]=0.0f; m[10]=1.0f; m[14]=0.0f;
    m[3]=0.0f; m[7]=0.0f; m[11]=0.0f; m[15]=1.0f;

}

void matrix4x4f::rotate(const float angle, vector3f axis) {

    float s = sin(DEGTORAD(angle));
    float c = cos(DEGTORAD(angle));

    axis.normalize();

    float ux = axis.x;
    float uy = axis.y;
    float uz = axis.z;
   
    m[0]  = c + (1-c) * ux;
    m[1]  = (1-c) * ux*uy + s*uz;
    m[2]  = (1-c) * ux*uz - s*uy;
    m[3]  = 0;
   
    m[4]  = (1-c) * uy*ux - s*uz;
    m[5]  = c + (1-c) * pow(uy,2);
    m[6]  = (1-c) * uy*uz + s*ux;
    m[7]  = 0;
   
    m[8]  = (1-c) * uz*ux + s*uy;
    m[9]  = (1-c) * uz*uz - s*ux;
    m[10] = c + (1-c) * pow(uz,2);
    m[11] = 0;
   
    m[12] = 0;
    m[13] = 0;
    m[14] = 0;
    m[15] = 1;

}

void matrix4x4f::transformVector(vector3f *vec) {

    vector3f &v = *vec;

    float x = v.x;
    float y = v.y;
    float z = v.z;

    v.x = x * m[0] +
          y * m[4] +
          z * m[8];

    v.y = x * m[1] +
          y * m[5] +
          z * m[9];

    v.z = x * m[2] +
          y * m[6] +
          z * m[10];

}

void matrix4x4f::transform(vector4f *v) {
        //we should save these values because transformVector changes the vector v
        float x = v->vec.x,
                  y = v->vec.y,
                  z = v->vec.z,
                  w = v->w;
//      printf("before:<%f,%f,%f,%f>\n", v->vec.x, v->vec.y, v->vec.z, v->w);

        //3D part
        transformVector(&(v->vec));
        //finish him with w coordinate
        v->vec.x += v->w * m[12];
        v->vec.y += v->w * m[13];
        v->vec.z += v->w * m[14];
//      printf("after:<%f,%f,%f,%f>\n", v->vec.x, v->vec.y, v->vec.z, v->w);

        v->w = m[3]*(x) + m[7]*(y) + m[11]*(z) + m[15]*(w);
//      printf("forgoten w:<%f,%f,%f,%f>\n", v->vec.x, v->vec.y, v->vec.z, v->w);
}


