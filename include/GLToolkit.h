#ifndef _GLToolkit_
#define _GLToolkit_

#include <math.h>
#include "VecLib.h"

void genFrustum(Mat4* ret, float left, float right, float bottom,float top, float near, float far);
void genPerspective(Mat4 *ret, float fovy, float aspect, float near, float far);
void genLookAt(Mat4* ret, const Vec4* const eye, const Vec4* const at, const Vec4* const up);
void turnCamera(Vec4* eye, Vec4* at, float theta);
void moveForward(Vec4* eye, Vec4* at,float amount);
void moveBackward(Vec4* eye, Vec4* at);



#endif
