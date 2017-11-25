#ifndef _Model_
#define _Model_

#include "VecLib.h"
#include <stdlib.h>

typedef struct Model
{
  Vec4* vertices;
  Vec4* colors;
  int num_vertices;
  Mat4 transform;
} Model;

void applyModelTranformation(Model* model, const Mat4* const mat, const int* const num_vertices);
void translateModelVec4(Model* model, const int* const num_vertices, const Vec4* const vec);
void rotateYOriginModel(Model* model, const int* const num_vertices, int degree);
void printVertices(Model* model, const int* const num_vertices);
void scaleXModel(Model* model, const int* const num_vertices, float factor);
void scaleYModel(Model* model, const int* const num_vertices, float factor);
void scaleZModel(Model* model, const int* const num_vertices, float factor);
void deepCopyModel(Model* ret, const Model* const model, const int* const num_vertices);
void flattenModelList(Model** list, Vec4** v, Vec4** c, int* nv, int* nm);
void makeCube(Model* cube);
void makeSphere(Model* sphere);
void setColor(Model* model, const Vec4* const color);







#endif

