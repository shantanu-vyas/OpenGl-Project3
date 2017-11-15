#include "Model.h"

Vec4 cube_vertices[] = {{1,1,1,1},{-1,1,1,1},{-1,-1,1,1},{-1,-1,1,1},{1,-1,1,1},
                        {1,1,1,1},{1,1,1,1},{1,-1,1,1},{1,-1,-1,1},{1,-1,-1,1},
                        {1,1,-1,1},{1,1,1,1},{1,1,1,1},{1,1,-1,1},{-1,1,-1,1},
                        {-1,1,-1,1},{-1,1,1,1},{1,1,1,1},{-1,1,1,1},{-1,1,-1,1},
                        {-1,-1,-1,1},{-1,-1,-1,1},{-1,-1,1,1},{-1,1,1,1},{-1,-1,-1,1},
                        {1,-1,-1,1},{1,-1,1,1},{1,-1,1,1},{-1,-1,1,1},{-1,-1,-1,1},
                        {1,-1,-1,1},{-1,-1,-1,1},{-1,1,-1,1},{-1,1,-1,1},{1,1,-1,1},
                        {1,-1,-1,1}};

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
void makeCube(Model* cube)
{
  Vec4 red = {1,0,0,1};
  cube->vertices = malloc(36*sizeof(Vec4));
  cube->colors = malloc(36*sizeof(Vec4));
  cube->num_vertices = 36;
  for (int i = 0; i < 36; i++)
    {
      cube->vertices[i] = cube_vertices[i];
      cube->colors[i] = red;
    }
  cube->num_vertices = 36;
}
void flattenModelList(Model** list, Vec4** v, Vec4** c, int* nv, int* nm)
{
  Model* deflist = *list; //idk why i cant figure out why this wont work otherwise

  int vertex_counter = 0;
  for (int i = 0; i < *nm; i++)
    {
       for (int p = 0; p < deflist[i].num_vertices; p++)
      	{
      	  vertex_counter++;
      	}
    }

  printf("vertex counter is %d \n",vertex_counter);
  *v = (Vec4*)malloc(sizeof(Vec4)*vertex_counter);
  *c = (Vec4*)malloc(sizeof(Vec4)*vertex_counter);
  
  Vec4 a = {1,1,1,1};
  /* (*v)[0] = a; */
  /* (*v)[1] = a; */
  /* (*v)[2] = a; */
  /* (*v)[3] = a; */

  int counter = 0;

  for (int i = 0; i < *nm; i++)
    {
      for (int p = 0; p < deflist[i].num_vertices; p++)
  	{
  	  (*v)[counter] = deflist[i].vertices[p];
  	  (*c)[counter] = deflist[i].colors[p];
  	  counter++;
  	}
    }
  *nv = vertex_counter;
}
