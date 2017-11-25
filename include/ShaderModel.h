#ifndef _Model_
#define _Model_

#include "VecLib.h"
#include <stdlib.h>
#include <GL/glew.h>

typedef struct ShaderModel //Where color is defined in the shader not the application
{
  Vec4* vertices;
  int num_vertices;
  Vec4 ambient;
  Vec4 specular;
  Vec4 diffuse;
  GLfloat shine;
  Mat4 transform;
} ShaderModel;


void applyModelTranformationSM(ShaderModel* model, const Mat4* const mat, const int* const num_vertices);
void translateModelVec4SM(ShaderModel* model, const int* const num_vertices, const Vec4* const vec);
void rotateYOriginModelSM(ShaderModel* model, const int* const num_vertices, int degree);
void printVerticesSM(ShaderModel* model, const int* const num_vertices);
void scaleXModelSM(ShaderModel* model, const int* const num_vertices, float factor);
void scaleYModelSM(ShaderModel* model, const int* const num_vertices, float factor);
void scaleZModelSM(ShaderModel* model, const int* const num_vertices, float factor);
void deepCopyModelSM(ShaderModel* ret, const ShaderModel* const model, const int* const num_vertices);
void flattenModelListSM(ShaderModel** list, Vec4** v, int* nv, int* nm);
void makeCubeSM(ShaderModel* cube, const Vec4* const ambient, const Vec4* const specular, const Vec4* const diffuse, const GLfloat* const shine);
void makeSphereSM(ShaderModel* sphere, const Vec4* const ambient, const Vec4* const specular, const Vec4* const diffuse, const GLfloat* const shine);
void setColorSM(ShaderModel* model, const Vec4* const ambient, const Vec4* const specular, const Vec4* const diffuse, const GLfloat* const shine);


#endif

