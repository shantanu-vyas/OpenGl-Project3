#include "Model.h"

void applyModelTranformation(Model* model, const Mat4* const mat, const int* const num_vertices)
{
  for (int i = 0; i < *num_vertices; i++)
    {
      Mat4MultVec4(&(model->vertices[i]),&(model->vertices[i]),mat);
    }
}
void translateModelVec4(Model* model, const int* const num_vertices, const Vec4* const vec)
{
  for (int i = 0; i < *num_vertices; i++)
    {
      translatePointVec4(&(model->vertices[i]),&(model->vertices[i]),vec);
    }
}

void rotateYOriginModel(Model* model, const int* const num_vertices, int degree)
{
  Vec4 centroid;
  findCentroid(&centroid, model->vertices,*num_vertices);
  scalarMultVec4(&centroid,&centroid,-1);
  translateModelVec4(model, num_vertices, &centroid);

  Mat4 rotation;
  identity(&rotation);
  rotateY(&rotation, M_PI/2);
  applyModelTranformation(model, &rotation, num_vertices);

  scalarMultVec4(&centroid,&centroid,-1);
  translateModelVec4(model, num_vertices, &centroid);
}

void scaleXModel(Model* model, const int* const num_vertices, float factor)
{
  /* printf("here"); */
  /* printf("%d",*num_vertices); */
  Mat4 scale;
  identity(&scale);
  Scale(&scale, factor,1,1);
  applyModelTranformation(model, &scale, num_vertices);
}
void scaleYModel(Model* model, const int* const num_vertices, float factor)
{
  /* printf("here"); */
  /* printf("%d",*num_vertices); */
  Mat4 scale;
  identity(&scale);
  Scale(&scale, 1,1,factor);
  applyModelTranformation(model, &scale, num_vertices);
}


void printVertices(Model* model, const int* const num_vertices)
{
  for (int i = 0; i < *num_vertices; i++)
    {
      printf("%f %f %f %d\n",model->vertices[i].x,model->vertices[i].y,model->vertices[i].z,1);
    }
}
void deepCopyModel(Model* ret, const Model* const model, const int* const num_vertices)
{
  ret->vertices = malloc(*num_vertices*sizeof(Vec4));
  ret->colors = malloc(*num_vertices*sizeof(Vec4));
  ret->num_vertices = *num_vertices;
  for (int i = 0; i < *num_vertices; i++)
    {
      ret->vertices[i] = model->vertices[i];
      ret->colors[i] = model->colors[i];
    }
    
}
