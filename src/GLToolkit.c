#include "GLToolkit.h"

void genPerspective(Mat4 *ret, float fovy, float aspect, float near, float far)
{
  identity(ret);
  float xx = (-1/(tan(fovy/2)))/aspect;
  float yy = (-1/(tan(fovy/2)));
  float zz = (near+far)/(far-near);
  float zw = -1;
  float wz = -2*(near*far)/(far-near);

  ret->x.x = xx;
  ret->y.y = yy;
  ret->z.z = zz;
  ret->z.w = zw;
  ret->w.z = wz;


}

void genFrustum(Mat4* ret, float left, float right, float bottom,float top, float near, float far)
{
  ret->x.x = 2.0/(right-left);
  ret->y.y = 2/(top-bottom);
  ret->z.z = -2/(far-near);
  ret->w.x = -1*(right+left)/(right-left);
  ret->w.y = -1*(top+bottom)/(top-bottom);
  ret->w.z = -1*(far+near)/(far-near);
  ret->w.w = 1;
}

void genLookAt(Mat4* ret, const Vec4* const eye, const Vec4* const at, const Vec4* const up)
{
  /* printVector(eye); */
  /* printVector(at); */
  /* printVector(up); */
  
  identity(ret);
  Vec4 n; //forward
  subVec4(&n,eye,at);
  normalize(&n,&n);

  Vec4 u; //side
  cross(&u,&n,up);
  normalize(&u,&u);

  /* printf("u\n"); */
  /* printVector(&n); */
  /* printVector(up); */
  /* printf("\n"); */

  Vec4 v; //up
  cross(&v,&n,&u);
  normalize(&v,&v);

  /* printVector(&n); */
  /* printVector(&u); */
  /* printVector(&v); */

  float x = -1*(eye->x*u.x) - (eye->y*u.y)- (eye->z*u.z);
  float y = -1*(eye->x*v.x) - (eye->y*v.y)- (eye->z*v.z);
  float z = -1*(eye->x*n.x) - (eye->y*n.y)- (eye->z*n.z);

  /* printf("x y z \n"); */
  /* printVector(&x); */
  /* printVector(&y); */
  /* printVector(&z); */

  ret->x.x = u.x;
  ret->x.y = v.x;
  ret->x.z = n.x;
  ret->w.x = x;

  ret->y.x = u.y;
  ret->y.y = v.y;
  ret->y.z = n.y;
  ret->w.y = y;

  ret->z.x = u.z;
  ret->z.y = v.z;
  ret->z.z = n.z;
  ret->w.z = z;
  ret->w.w = 1;

  /* printf("\n"); */
  /* printMatrix(ret); */
}

//updates at vector
void moveForward(Vec4* eye, Vec4* at, float amount)
{
  Vec4 sub;
  subVec4(&sub,at,eye);
  //printf("sub vec\n");
  //printVector(&sub);
  normalize(&sub,&sub);

  //float factor = 2;
  
  at->x = at->x+sub.x*amount;
  at->y = at->y+sub.y*amount;
  at->z = at->z+sub.z*amount;

  eye->x = eye->x+sub.x*amount;
  eye->y = eye->y+sub.y*amount;
  eye->z = eye->z+sub.z*amount;


  /* printf("----------------\n"); */
  /* printf("Rotation\n"); */
  /* printf("Eye is: "); */
  /* printVector(eye); */
  /* printf("At is: "); */
  /* printVector(at); */


}
void moveBackward(Vec4* eye, Vec4* at)
{
  //printVector(eye);
  Vec4 sub;
  subVec4(&sub,at,eye);
  normalize(&sub,&sub);

  at->x = at->x-sub.x*.1;
  at->y = at->y-sub.y*.1;
  at->z = at->z-sub.z*.1;

  eye->x = eye->x-sub.x*.1;
  eye->y = eye->y-sub.y*.1;
  eye->z = eye->z-sub.z*.1;

}
void turnCamera(Vec4* eye, Vec4* at, float theta)
{
  /* printf("----------------\n"); */
  /* printf("Rotation\ninitial\n"); */
  /* printf("Eye is: "); */
  /* printVector(eye); */
  /* printf("At is: "); */
  /* printVector(at); */
  /* printf("\n"); */
  //theta = theta+1*M_PI/4;
  Vec4 sub;

  subVec4(&sub,at,eye);
  normalize(&sub,&sub);

  Mat4 rotationY;
  identity(&rotationY);
  rotateY(&rotationY,theta);
  Mat4MultVec4(&sub,&sub,&rotationY);

  at->x = eye->x+sub.x;
  at->y = eye->y+sub.y;
  at->z = eye->z+sub.z;

  /* printf("Eye is: "); */
  /* printVector(eye); */
  /* printf("At is: "); */
  /* printVector(at); */


}
