#include "ShaderModel.h"

Vec4 cube_vertices[] = 
  {{1.f, 1.f, 1.f, 1.f},{-1.f, 1.f, 1.f, 1.f},{-1.f, -1.f, 1.f, 1.f},{-1.f, -1.f, 1.f, 1.f},{1.f, -1.f, 1.f, 1.f},
  {1.f, 1.f, 1.f, 1.f},{1.f, 1.f, 1.f, 1.f},{1.f, -1.f, 1.f, 1.f},{1.f, -1.f, -1.f, 1.f},{1.f, -1.f, -1.f, 1.f},
  {1.f, 1.f, -1.f, 1.f},{1.f, 1.f, 1.f, 1.f},{1.f, 1.f, 1.f, 1.f},{1.f, 1.f, -1.f, 1.f},{-1.f, 1.f, -1.f, 1.f},
  {-1.f, 1.f, -1.f, 1.f},{-1.f, 1.f, 1.f, 1.f},{1.f, 1.f, 1.f, 1.f},{-1.f, 1.f, 1.f, 1.f},{-1.f, 1.f, -1.f, 1.f},
  {-1.f, -1.f, -1.f, 1.f},{-1.f, -1.f, -1.f, 1.f},{-1.f, -1.f, 1.f, 1.f},{-1.f, 1.f, 1.f, 1.f},{-1.f, -1.f, -1.f, 1.f},
  {1.f, -1.f, -1.f, 1.f},{1.f, -1.f, 1.f, 1.f},{1.f, -1.f, 1.f, 1.f},{-1.f, -1.f, 1.f, 1.f},{-1.f, -1.f, -1.f, 1.f},
  {1.f, -1.f, -1.f, 1.f},{-1.f, -1.f, -1.f, 1.f},{-1.f, 1.f, -1.f, 1.f},{-1.f, 1.f, -1.f, 1.f},{1.f, 1.f, -1.f, 1.f},
  {1.f, -1.f, -1.f, 1.f}};

void applyModelTranformation(ShaderModel* model, const Mat4* const mat, const int* const num_vertices)
{
  for (int i = 0; i < *num_vertices; i++)
    {
      Mat4MultVec4(&(model->vertices[i]),&(model->vertices[i]),mat);
    }
}
void translateModelVec4(ShaderModel* model, const int* const num_vertices, const Vec4* const vec)
{
  for (int i = 0; i < *num_vertices; i++)
    {
      translatePointVec4(&(model->vertices[i]),&(model->vertices[i]),vec);
    }
}

void rotateYOriginModel(ShaderModel* model, const int* const num_vertices, int degree)
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

void scaleXModel(ShaderModel* model, const int* const num_vertices, float factor)
{
  /* printf("here"); */
  /* printf("%d",*num_vertices); */
  Mat4 scale;
  identity(&scale);
  Scale(&scale,factor,1,1);
  applyModelTranformation(model, &scale, num_vertices);
}
void scaleYModel(ShaderModel* model, const int* const num_vertices, float factor)
{
  /* printf("here"); */
  /* printf("%d",*num_vertices); */
  Mat4 scale;
  identity(&scale);
  Scale(&scale,1,factor,1);
  applyModelTranformation(model, &scale, num_vertices);
}
void scaleZModel(ShaderModel* model, const int* const num_vertices, float factor)
{
  /* printf("here"); */
  /* printf("%d",*num_vertices); */
  Mat4 scale;
  identity(&scale);
  Scale(&scale,1,1,factor);
  applyModelTranformation(model, &scale, num_vertices);
}



void printVertices(ShaderModel* model, const int* const num_vertices)
{
  for (int i = 0; i < *num_vertices; i++)
    {
      printf("%f %f %f %d\n",model->vertices[i].x,model->vertices[i].y,model->vertices[i].z,1);
    }
}
void deepCopyModel(ShaderModel* ret, const ShaderModel* const model, const int* const num_vertices)
{
  ret->vertices = malloc(*num_vertices*sizeof(Vec4));
  ret->num_vertices = *num_vertices;
  ret->ambient = model->ambient;
  ret->specular = model->specular;
  ret->diffuse = model->diffuse;
  ret->shine = model->shine;
  for (int i = 0; i < *num_vertices; i++)
    {
      ret->vertices[i] = model->vertices[i];
    }

}
void makeCube(ShaderModel* cube, const Vec4* const ambient, const Vec4* const specular, const Vec4* const diffuse, const GLfloat* const shine)
{
  cube->vertices = malloc(36*sizeof(Vec4));
  cube->num_vertices = 36;
  cube->ambient = *ambient;
  cube->specular = *specular;
  cube->diffuse = *diffuse;
  cube->shine = *shine;
  for (int i = 0; i < 36; i++)
    {
      cube->vertices[i] = cube_vertices[i];
    }
  cube->num_vertices = 36;
}

void makeSphere(ShaderModel* sphere, const Vec4* const ambient, const Vec4* const specular, const Vec4* const diffuse, const GLfloat* const shine)
{
  sphere->vertices = malloc(sizeof(Vec4)*1140*100);
  sphere->num_vertices = 1140*16;
  sphere->ambient = *ambient;
  sphere->specular = *specular;
  sphere->diffuse = *diffuse;
  sphere->shine = *shine;
  // returns number of vertices it created. I think its 1140
  float DegreesToRadians = (float)M_PI / 180.f;
  Vec4 temp;
  Vec4 offset = (Vec4) { -0.5f, 0.25f, 0.0f, 1.0f }; // sphere location is (−0.5, 0.25, 0.0)
  //vec4 offset = (vec4) { 0.f, 0.f, 0.0f, 0.0f };
  int k = 0;
  for (float phi = -90.f; phi <= 90.f; phi += 5.f) {
    float phir = phi*DegreesToRadians;
    float phir10 = (phi + 10.f)*DegreesToRadians;
    float phir20 = (phi + 20.f)*DegreesToRadians;
    for (float theta = -180.f; theta <= 180.f; theta += 5.f) {
      float thetar = theta*DegreesToRadians;
      float thetar_prev = (theta - 20.f)*DegreesToRadians;
      temp.x = sinf(thetar)*cosf(phir);
      temp.y = cosf(thetar)*cosf(phir);
      temp.z = sinf(phir);
      temp.w = 1.f;
      sphere->vertices[k++] = temp;
      temp.x = sinf(thetar)*cosf(phir20);
      temp.y = cosf(thetar)*cosf(phir20);
      temp.z = sinf(phir20);
      temp.w = 1.f;
      sphere->vertices[k++] = temp;
      temp.x = sinf(thetar_prev)*cosf(phir20);
      temp.y = cosf(thetar_prev)*cosf(phir20);
      temp.z = sinf(phir20);
      temp.w = 1.f;
      sphere->vertices[k++] = temp;
      temp.x = sinf(thetar)*cosf(phir);
      temp.y = cosf(thetar)*cosf(phir);
      temp.z = sinf(phir);
      temp.w = 1.f;
      sphere->vertices[k++] = temp;
      temp.x = sinf(thetar_prev)*cosf(phir);
      temp.y = cosf(thetar_prev)*cosf(phir);
      temp.z = sinf(phir);
      temp.w = 1.f;
      sphere->vertices[k++] = temp;
      temp.x = sinf(thetar_prev)*cosf(phir20);
      temp.y = cosf(thetar_prev)*cosf(phir20);
      temp.z = sinf(phir20);
      temp.w = 1.f;
      sphere->vertices[k++] = temp;
    }
  }
}
void flattenModelList(ShaderModel** list, Vec4** v, int* nv, int* nm)
{
  ShaderModel* deflist = *list; //idk why i cant figure out why this wont work otherwise

  int vertex_counter = 0;
  for (int i = 0; i < *nm; i++)
    {
      for (int p = 0; p < deflist[i].num_vertices; p++)
        {
          vertex_counter++;
        }
    }

  //printf("vertex counter is %d \n",vertex_counter);
  *v = (Vec4*)malloc(sizeof(Vec4)*vertex_counter);
  //printf("pointer for vertices %p \n",v);
  //printf("pointer for colors %p \n",c);
  int counter = 0;

  for (int i = 0; i < *nm; i++)
    {
      for (int p = 0; p < deflist[i].num_vertices; p++)
        {
          (*v)[counter] = deflist[i].vertices[p];
          counter++;
        }
    }
  *nv = vertex_counter;
}
void setColor(ShaderModel* model, const Vec4* const ambient, const Vec4* const specular, const Vec4* const diffuse, const GLfloat* const shine)
{
  for (int i = 0; i < model->num_vertices; i++)
    { 
      model->ambient = *ambient;
      model->specular = *specular;
      model->diffuse = *diffuse;
      model->shine = *shine;
    }
}
