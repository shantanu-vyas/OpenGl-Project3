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


void applyModelTranformation(ShaderModel* model, const Mat4* const mat, const int* const num_vertices);
void translateModelVec4(ShaderModel* model, const int* const num_vertices, const Vec4* const vec);
void rotateYOriginModel(ShaderModel* model, const int* const num_vertices, int degree);
void printVertices(ShaderModel* model, const int* const num_vertices);
void scaleXModel(ShaderModel* model, const int* const num_vertices, float factor);
void scaleYModel(ShaderModel* model, const int* const num_vertices, float factor);
void scaleZModel(ShaderModel* model, const int* const num_vertices, float factor);
void deepCopyModel(ShaderModel* ret, const ShaderModel* const model, const int* const num_vertices);
void flattenModelList(ShaderModel** list, Vec4** v, int* nv, int* nm);
void makeCube(ShaderModel* cube, const Vec4* const ambient, const Vec4* const specular, const Vec4* const diffuse, const GLfloat* const shine);
void makeSphere(ShaderModel* sphere, const Vec4* const ambient, const Vec4* const specular, const Vec4* const diffuse, const GLfloat* const shine);
void setColor(ShaderModel* model, const Vec4* const ambient, const Vec4* const specular, const Vec4* const diffuse, const GLfloat* const shine);







#endif

